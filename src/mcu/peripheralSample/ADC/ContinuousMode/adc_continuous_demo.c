/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     adc_continuous_demo.c
* @brief    adc continuous mode demo
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
#define ADC_SAMPLE_PIN_0                    P2_3
#define ADC_SAMPLE_CHANNEL_0                ADC_Channel_Index_3

#define ADC_CONTINUOUS_SAMPLE_PERIOD        (200-1)//sampling once 20ms


/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_adc_init(void)
{
    Pad_Config(ADC_SAMPLE_PIN_0, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    Pinmux_Config(ADC_SAMPLE_PIN_0, IDLE_MODE);

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
#if (ADC_MODE_SINGLE_OR_DIFFERENTIAL == ADC_SINGLE_MODE)//single mode
    ADC_InitStruct.schIndex[0]         = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL_0);
#elif (ADC_MODE_SINGLE_OR_DIFFERENTIAL == ADC_DIFFERENTIAL_MODE)//differential mode
    ADC_InitStruct.schIndex[0]         = EXT_DIFFERENTIAL(ADC_SAMPLE_CHANNEL_0);
#endif
    ADC_InitStruct.bitmap              = 0x01;
    ADC_InitStruct.adcSamplePeriod     = ADC_CONTINUOUS_SAMPLE_PERIOD;
    ADC_InitStruct.adcFifoThd          = 10;
    ADC_Init(ADC, &ADC_InitStruct);

#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
    /* High bypass resistance mode config, please notice that the input voltage of
      adc channel using high bypass mode should not be over 0.9V */
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_0, ENABLE);
#endif

    ADC_INTConfig(ADC, ADC_INT_FIFO_TH, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    ADC_Cmd(ADC, ADC_Continuous_Mode, ENABLE);
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

}

/**
  * @brief  ADC interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void ADC_Handler(void)
{
    uint16_t sample_data[10];

    if (ADC_GetIntFlagStatus(ADC, ADC_INT_FIFO_TH) == SET)
    {
        ADC_ClearINTPendingBit(ADC, ADC_INT_FIFO_TH);
        ADC_GetFifoData(ADC, sample_data, 10);
        for (uint8_t i = 0; i < 10; i++)
        {
            DBG_DIRECT("[ADC] ADC continuous mode sample data! data[%d]=%d", i, sample_data[i]);
        }
    }
}
