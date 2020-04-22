/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     adc_one_shot_demo.c
* @brief    adc one shot mode demo
* @details
* @author   yuan
* @date     2018-06-22
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_adc.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_tim.h"
#include "trace.h"

/* Defines --------------------------------------------------------------*/
#define ADC_Channel_Index_0         0
#define ADC_Channel_Index_1         1
#define ADC_Channel_Index_2         2
#define ADC_Channel_Index_3         3
#define ADC_Channel_Index_4         4
#define ADC_Channel_Index_5         5
#define ADC_Channel_Index_6         6
#define ADC_Channel_Index_7         7

#define ADC_Schedule_Index_0        0
#define ADC_Schedule_Index_1        1
#define ADC_Schedule_Index_2        2
#define ADC_Schedule_Index_3        3
#define ADC_Schedule_Index_4        4
#define ADC_Schedule_Index_5        5
#define ADC_Schedule_Index_6        6
#define ADC_Schedule_Index_7        7
#define ADC_Schedule_Index_8        8
#define ADC_Schedule_Index_9        9

/* Config ADC channel and mode --------------------------------------------------------------*/
//ADC bypass mode or divide mode
#define ADC_DIVIDE_MODE                     0
#define ADC_BYPASS_MODE                     1
#define ADC_MODE_DIVIDE_OR_BYPASS           ADC_DIVIDE_MODE

//ADC channel single or differential sample mode
#define ADC_SINGLE_MODE                     0
#define ADC_DIFFERENTIAL_MODE               1
#define ADC_MODE_SINGLE_OR_DIFFERENTIAL     ADC_SINGLE_MODE

//ADC sample channel config:P2_0~P2_7 VBAT
#define ADC_SAMPLE_PIN_0                    P2_2
#define ADC_SAMPLE_PIN_1                    P2_3
#define ADC_SAMPLE_CHANNEL_0                ADC_Channel_Index_2
#define ADC_SAMPLE_CHANNEL_1                ADC_Channel_Index_3


#define ADC_ONE_SHOT_SAMPLE_PERIOD          (1000000)


/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_adc_init(void)
{
    Pad_Config(ADC_SAMPLE_PIN_0, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(ADC_SAMPLE_PIN_1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    Pinmux_Config(ADC_SAMPLE_PIN_0, IDLE_MODE);
    Pinmux_Config(ADC_SAMPLE_PIN_1, IDLE_MODE);

}

/**
  * @brief  Initialize adc peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_adc_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    ADC_InitTypeDef ADC_InitStruct;
    ADC_StructInit(&ADC_InitStruct);
#if (ADC_MODE_SINGLE_OR_DIFFERENTIAL == ADC_SINGLE_MODE)//single mode
    ADC_InitStruct.schIndex[0]         = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL_0);
    ADC_InitStruct.schIndex[1]         = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL_1);
#elif (ADC_MODE_SINGLE_OR_DIFFERENTIAL == ADC_DIFFERENTIAL_MODE)//differential mode
    ADC_InitStruct.schIndex[0]         = EXT_DIFFERENTIAL(ADC_SAMPLE_CHANNEL_0);
    ADC_InitStruct.schIndex[1]         = EXT_DIFFERENTIAL(ADC_SAMPLE_CHANNEL_1);
#endif
    ADC_InitStruct.bitmap              = 0x03;
    ADC_InitStruct.timerTriggerEn      = ENABLE;
    ADC_Init(ADC, &ADC_InitStruct);

#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
    /* High bypass resistance mode config, please notice that the input voltage of
      adc channel using high bypass mode should not be over 0.9V */
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_0, ENABLE);
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_1, ENABLE);
#endif

    ADC_INTConfig(ADC, ADC_INT_ONE_SHOT_DONE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    ADC_Cmd(ADC, ADC_One_Shot_Mode, ENABLE);
}

/**
  * @brief  Initialize timer peripheral for adc one shot mode.
  * @param  No parameter.
  * @return void
*/
void driver_adc_timer_init(void)
{
    /* TIM7 for timing voltage acqusition(fix) */
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;

    TIM_StructInit(&TIM_InitStruct);

    TIM_InitStruct.TIM_PWM_En = PWM_DISABLE;
    TIM_InitStruct.TIM_Period = ADC_ONE_SHOT_SAMPLE_PERIOD - 1 ;    //sampling once 1 second
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_SOURCE_DIV = TIM_CLOCK_DIVIDER_40;
    TIM_TimeBaseInit(TIM7, &TIM_InitStruct);

    TIM_ClearINT(TIM7);
    TIM_INTConfig(TIM7, ENABLE);
    TIM_Cmd(TIM7, ENABLE);
}

/**
  * @brief  Demo code of operation about adc.
  * @param  No parameter.
  * @return void
*/
void adc_demo(void)
{
    /* Configure pad and pinmux firstly! */
    board_adc_init();

    /* Initialize adc peripheral */
    driver_adc_init();

    /* Initialize timer peripheral */
    driver_adc_timer_init();

}

/**
  * @brief  ADC interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void ADC_Handler(void)
{
    uint16_t sample_data[2];

    if (ADC_GetIntFlagStatus(ADC, ADC_INT_ONE_SHOT_DONE) == SET)
    {
        ADC_ClearINTPendingBit(ADC, ADC_INT_ONE_SHOT_DONE);
        sample_data[0] = ADC_ReadByScheduleIndex(ADC, ADC_Schedule_Index_0);
        sample_data[1] = ADC_ReadByScheduleIndex(ADC, ADC_Schedule_Index_1);
        DBG_DIRECT("[ADC] ADC one shot mode sample data! data0 = %d, data1 = %d ", sample_data[0],
                   sample_data[1]);
    }
}
