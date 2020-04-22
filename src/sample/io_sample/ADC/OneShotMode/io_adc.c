/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_adc.c
* @brief    This file provides demo code of adc continuous mode.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "io_adc.h"

#include "app_task.h"

/* Globals ------------------------------------------------------------------*/
ADC_Data_TypeDef ADC_Global_Data;

/**
  * @brief  Initialization adc global data.
  * @param  No parameter.
  * @return void
  */
void global_data_adc_init(void)
{
    /* Initialize adc k value! */
    APP_PRINT_INFO0("[io_adc] global_data_adc_init");
    bool adc_k_status = false;
    adc_k_status = ADC_CalibrationInit();
    if (false == adc_k_status)
    {
        APP_PRINT_ERROR0("[io_adc] global_data_adc_init: ADC_CalibrationInit fail!");
    }
    memset(&ADC_Global_Data, 0, sizeof(ADC_Global_Data));
}

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
}

/**
  * @brief  Initialize ADC peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_adc_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    ADC_InitTypeDef ADC_InitStruct;
    ADC_StructInit(&ADC_InitStruct);
    /* In one shot mode, if it is necessary to sample at regular intervals,
       the hardware timer TIM7 (which needs to be configured separately) can be used to realize ADC timing sampling.
       After enabling the timer, there is no need to manually enable ADC for each sampling.*/
    ADC_InitStruct.timerTriggerEn    = ENABLE;
    /* Configure the ADC sampling schedule, a schedule represents an ADC channel data,
       up to 16, i.e. schIndex[0] ~ schIndex[15] */
    ADC_InitStruct.schIndex[0]         = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL_0);
    ADC_InitStruct.schIndex[1]         = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL_1);
    /* Set the bitmap corresponding to schedule, 16 bits, LSB,
       schIndex[0-15] corresponding to 16 bits of bitmap bit0-bit15.
       For example, if config schIndex[0] and schIndex [1], then bitmap is 0000 0000 0011 (that is, 0x0003);
       if config schIndex [0] and schIndex [2], then bitmap is 0000 0000 0101 (that is, 0x0005).
    */
    ADC_InitStruct.bitmap              = 0x03;
    /* Fixed 255 in OneShot mode. */
    ADC_InitStruct.adcSamplePeriod     = 255;
    ADC_Init(ADC, &ADC_InitStruct);

#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
    /* High bypass resistance mode config, please notice that the input voltage of
      adc channel using high bypass mode should not be over 0.9V */
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_0, ENABLE);
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_1, ENABLE);
    APP_PRINT_INFO0("[io_adc] driver_adc_init: ADC sample mode is bypass mode !");
#else
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_0, DISABLE);
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_1, DISABLE);
    APP_PRINT_INFO0("[io_adc] driver_adc_init: ADC sample mode is divide mode !");
#endif

    ADC_INTConfig(ADC, ADC_INT_ONE_SHOT_DONE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    /* When ADC is enabled, sampling will be done quickly and interruption will occur.
       After initialization, ADC can be enabled when sampling is needed.*/
    ADC_Cmd(ADC, ADC_One_Shot_Mode, ENABLE);
}

/**
  * @brief  Initialize timer peripheral for adc one shot mode.
  * @param  No parameter.
  * @return void
*/
void driver_adc_timer_init(void)
{
    /* Only timer7 can be used as a hardware timer for timing sampling of ADC one shot mode. */
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;

    TIM_StructInit(&TIM_InitStruct);

    TIM_InitStruct.TIM_PWM_En = PWM_DISABLE;
    /* Timing settings for timer see IO TIM demo. */
    TIM_InitStruct.TIM_Period = ADC_ONE_SHOT_SAMPLE_PERIOD - 1 ;    //sampling once 1 second
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_SOURCE_DIV = TIM_CLOCK_DIVIDER_40;
    TIM_TimeBaseInit(TIM7, &TIM_InitStruct);

    TIM_ClearINT(TIM7);
    TIM_INTConfig(TIM7, ENABLE);
    TIM_Cmd(TIM7, ENABLE);
}

/**
  * @brief  Calculate adc sample voltage.
  * @param  No parameter.
  * @return void
  */
static void io_adc_voltage_calculate(T_IO_MSG *io_adc_msg)
{
    uint8_t sample_data_len = 0;
    uint16_t sample_data[ADC_SCHEDULE_NUM] = {0};
    float sample_voltage[ADC_SCHEDULE_NUM] = {0};
    ADC_ErrorStatus error_status = NO_ERROR;

    uint16_t *p_buf = io_adc_msg->u.buf;

    sample_data_len = p_buf[0];
    for (uint8_t i = 0; i < sample_data_len; i++)
    {
        sample_data[i] = p_buf[i + 1];
    }
    for (uint8_t i = 0; i < sample_data_len; i++)
    {
#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
        sample_voltage[i] = ADC_GetVoltage(BYPASS_SINGLE_MODE, (int32_t)sample_data[i], &error_status);
#else
        sample_voltage[i] = ADC_GetVoltage(DIVIDE_SINGLE_MODE, (int32_t)sample_data[i], &error_status);
#endif
        if (error_status < 0)
        {
            APP_PRINT_INFO2("[io_adc] io_adc_voltage_calculate: ADC parameter or efuse data error! i = %d, error_status = %d",
                            i, error_status);
        }
        else
        {
            APP_PRINT_INFO4("[io_adc] io_adc_voltage_calculate: ADC rawdata_%-4d = %d, voltage_%-4d = %dmV ", i,
                            sample_data[i], i, (uint32_t)sample_voltage[i]);
        }
    }
    memset(&ADC_Global_Data, 0, sizeof(ADC_Global_Data));
}

/**
  * @brief  Handle adc data function.
  * @param  No parameter.
  * @return void
  */
void io_handle_adc_msg(T_IO_MSG *io_adc_msg)
{
    io_adc_voltage_calculate(io_adc_msg);
}

/**
  * @brief  ADC interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void ADC_Handler(void)
{
    uint16_t sample_data[ADC_SCHEDULE_NUM] = {0};

    if (ADC_GetIntFlagStatus(ADC, ADC_INT_ONE_SHOT_DONE) == SET)
    {
        ADC_ClearINTPendingBit(ADC, ADC_INT_ONE_SHOT_DONE);
        /* ADC one shot sampling mode, read data from schedule table.
           The schedule index is which config in adc init function.
           The value is ADC_Schedule_Index_0 ~ ADC_Schedule_Index_15
        */

        sample_data[0] = ADC_ReadByScheduleIndex(ADC, ADC_Schedule_Index_0);
        sample_data[1] = ADC_ReadByScheduleIndex(ADC, ADC_Schedule_Index_1);

        T_IO_MSG int_adc_msg;

        int_adc_msg.type = IO_MSG_TYPE_ADC;
        int_adc_msg.subtype = 0;
        ADC_Global_Data.RawData[0] = 2;
        for (uint8_t i = 0; i < 2; i++)
        {
            ADC_Global_Data.RawData[i + 1] = sample_data[i];
        }
        int_adc_msg.u.buf = (void *)(ADC_Global_Data.RawData);
        if (false == app_send_msg_to_apptask(&int_adc_msg))
        {
            APP_PRINT_ERROR0("[io_adc] ADC_Handler: Send int_adc_msg failed!");
            //Add user code here!
            ADC_ClearFifo(ADC);
            ADC_ClearINTPendingBit(ADC, ADC_INT_FIFO_TH);
            return;
        }
    }
}

