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
    /* Configuration of ADC continuous sampling cycle,
       can be configured to (1-256) clock cycles.
       That is, N = T*10000000 - 1 ,max = 255.
    */
    ADC_InitStruct.adcSamplePeriod     = ADC_CONTINUOUS_SAMPLE_PERIOD;
    /* Configure the interrupt of ADC_INT_FIFO_TH threshold value. */
    ADC_InitStruct.adcFifoThd          = 16;
    ADC_Init(ADC, &ADC_InitStruct);

#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
    /* High bypass resistance mode config, please notice that the input voltage of
      adc channel using high bypass mode should not be over 0.9V */
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_0, ENABLE);
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_1, ENABLE);
    APP_PRINT_INFO0("[ADC] ADC sample mode is bypass mode !");
#else
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_0, DISABLE);
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_1, DISABLE);
    APP_PRINT_INFO0("[ADC] ADC sample mode is divide mode !");
#endif

    ADC_INTConfig(ADC, ADC_INT_FIFO_TH, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    /* When ADC is enabled, sampling will be done quickly and interruption will occur.
       After initialization, ADC can be enabled when sampling is needed.*/
    ADC_Cmd(ADC, ADC_Continuous_Mode, ENABLE);
}

/**
  * @brief  Calculate adc sample voltage.
  * @param  No parameter.
  * @return void
  */
static void io_adc_voltage_calculate(T_IO_MSG *io_adc_msg)
{
    uint8_t sample_data_len = 0;
    uint16_t sample_data[ADC_FIFO_MAX] = {0};
    float sample_voltage[ADC_FIFO_MAX] = {0};
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
    for (uint32_t i = 0; i < 1000000; i++);
    ADC_Cmd(ADC, ADC_Continuous_Mode, ENABLE);
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
    uint8_t data_len = 0;
    uint16_t sample_data[ADC_FIFO_MAX] = {0};

    if (ADC_GetIntFlagStatus(ADC, ADC_INT_FIFO_TH) == SET)
    {
        ADC_Cmd(ADC, ADC_Continuous_Mode, DISABLE);

        data_len = ADC_GetFifoLen(ADC);
        /* ADC continuous sampling mode, read data from FIFO.
           In multi-channel sampling,
           the data order in FIFO corresponds to the channel order set by schedule index.
        */
        ADC_GetFifoData(ADC, sample_data, data_len);

        ADC_ClearFifo(ADC);
        ADC_ClearINTPendingBit(ADC, ADC_INT_FIFO_TH);

        T_IO_MSG int_adc_msg;

        int_adc_msg.type = IO_MSG_TYPE_ADC;
        int_adc_msg.subtype = 0;
        ADC_Global_Data.RawData[0] = data_len;
        for (uint8_t i = 0; i < data_len; i++)
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

