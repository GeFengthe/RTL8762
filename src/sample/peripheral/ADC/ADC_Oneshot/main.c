/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of ADC Oneshot mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
/* Include ---------------------------------------------------------------*/
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_adc.h"
#include "rtl876x_nvic.h"
#include "rtl876x_tim.h"
#include "rtl876x_wdg.h"
#include "board.h"
#include "trace.h"
#include <os_sched.h>
#include "bee2_adc_lib.h"
/* Defines --------------------------------------------------------------*/
#define ADC_SCHEDULE_0               0
#define ADC_SCHEDULE_1               1
#define ADC_SCHEDULE_2               2
#define ADC_SCHEDULE_3               3
#define ADC_SCHEDULE_4               4
#define ADC_SCHEDULE_5               5
#define ADC_SCHEDULE_6               6
#define ADC_SCHEDULE_7               7

#define ADC_SCHEDULE_8               8
#define ADC_SCHEDULE_9               9
#define ADC_SCHEDULE_10              10
#define ADC_SCHEDULE_11              11
#define ADC_SCHEDULE_12              12
#define ADC_SCHEDULE_13              13
#define ADC_SCHEDULE_14              14
#define ADC_SCHEDULE_15              15


#define ADC_CHANNEL_2                2

#define ADC_CHANNEL_VBAT_SCHEDULE    ADC_SCHEDULE_0
#define ADC_CHANNEL_2_PIN            P2_2
#define ADC_CHANNEL_2_PIN_SCHEDULE   ADC_SCHEDULE_1

/**
  * @brief  initialization of pinmux settings and pad settings.
  * @param   No parameter.
  * @return  void
  */
void Board_ADC_init(void)
{
    Pad_Config(ADC_CHANNEL_2_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pinmux_Config(ADC_CHANNEL_2_PIN, IDLE_MODE);
}

/**
  * @brief  Initialize ADC peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_ADC_init(void)
{
    int i;
    ADC_DeInit(ADC);
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    ADC_InitTypeDef adcInitStruct;
    ADC_StructInit(&adcInitStruct);

    for (i = 0; i < 8; i++)
    {
        adcInitStruct.schIndex[ADC_SCHEDULE_0 + i]         = INTERNAL_VBAT_MODE;
        adcInitStruct.schIndex[ADC_SCHEDULE_8 + i]         = EXT_SINGLE_ENDED(ADC_CHANNEL_2);
    }

    //(1<<ADC_SCHEDULE_0) | (1<<ADC_SCHEDULE_1).........|(1<<ADC_SCHEDULE_15);
    adcInitStruct.bitmap              = 0xffff;
    adcInitStruct.adcSamplePeriod     = 127;             //10M/(127+1)=78.125K
    adcInitStruct.adcConvertTimePeriod = ADC_CONVERT_TIME_1100NS;
    adcInitStruct.timerTriggerEn      = ENABLE;
    ADC_Init(ADC, &adcInitStruct);
    ADC_INTConfig(ADC, ADC_INT_ONE_SHOT_DONE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    ADC_Cmd(ADC, ADC_One_Shot_Mode, ENABLE);
}
/**
* @brief  ADC interrupt handler function.
* @param   No parameter.
* @return  void
*/
uint16_t avg_min[2], avg_max[2];
bool calc_en[2] = {false, false};
void calc_avg(uint16_t *arr, uint8_t index)
{
    uint32_t avg = arr[0];
    uint16_t max = arr[0];
    uint16_t min = arr[0];
    for (int i = 1; i < 8; i++)
    {
        if (arr[i] > max)
        {
            max = arr[i];
        }
        if (arr[i] < min)
        {
            min = arr[i];
        }
        avg += arr[i];
    }
    avg = (avg - max - min) / 6;

    if (calc_en[index] == false)
    {
        calc_en[index] = true;
        avg_min[index] = avg_max[index] = avg;
    }
    else
    {
        if (avg > avg_max[index])
        {
            avg_max[index] = avg;
        }
        if (avg < avg_min[index])
        {
            avg_min[index] = avg;
        }
    }
    DBG_DIRECT("avg voltage is %d,avg_max is %d,avg_min is %d", avg, avg_max[index], avg_min[index]);
}
void ADC_Handler(void)
{
    uint16_t sample_data[8];
    uint16_t vol[8];
//    uint16_t res = 0;

    if (ADC_GetIntFlagStatus(ADC, ADC_INT_ONE_SHOT_DONE) == SET)
    {
        ADC_ErrorStatus ErrorStatus;
        ADC_ClearINTPendingBit(ADC, ADC_INT_ONE_SHOT_DONE);
        for (int i = 0; i < 8; i++)
        {
            sample_data[i] = ADC_ReadByScheduleIndex(ADC, ADC_SCHEDULE_0 + i);
            vol[i] =  ADC_GetVoltage(DIVIDE_SINGLE_MODE, sample_data[i], &ErrorStatus);
            DBG_DIRECT("VBAT---Schedule index%d:raw data %x,vbat vol is %d", i, sample_data[i], vol[i]);
        }
        calc_avg(vol, 0);
        for (int i = 0; i < 8; i++)
        {
            sample_data[i] = ADC_ReadByScheduleIndex(ADC, ADC_SCHEDULE_8 + i);
//            res = ADC_GetRes(sample_data[i], EXT_SINGLE_ENDED(ADC_SCHEDULE_8+i));
            vol[i] =  ADC_GetVoltage(DIVIDE_SINGLE_MODE, sample_data[i], &ErrorStatus);
            DBG_DIRECT("CHANNEL2---Schedule index%d:raw data %x,vbat vol is %d", i, sample_data[i], vol[i]);
        }
        calc_avg(vol, 1);
        DBG_DIRECT("Next Sample----------------");
    }
    ADC_Cmd(ADC, ADC_One_Shot_Mode, ENABLE);
}
int main()
{
    WDG_Disable();
    __enable_irq();
    Board_ADC_init();
    Driver_ADC_init();
    while (1)
    {
        __nop();
    }

}
