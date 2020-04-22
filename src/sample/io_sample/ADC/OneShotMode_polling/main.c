/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    Uses adc one shot sampling mode. Polling reads adc sampling data.
* @details
* @author   yuan_feng
* @date     2018-12-03
* @version  v0.1
*********************************************************************************************************
*/

/* Includes --------------------------------------------------------------*/
#include "rtl876x_adc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"

#include "bee2_adc_lib.h"

#include "trace.h"
//#define INTERNAL_VBAT_TEST          1

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
#define ADC_Schedule_Index_10       10
#define ADC_Schedule_Index_11       11
#define ADC_Schedule_Index_12       12
#define ADC_Schedule_Index_13       13
#define ADC_Schedule_Index_14       14
#define ADC_Schedule_Index_15       15

/* Config ADC channel and mode --------------------------------------------------------------*/
/* ADC bypass mode or divide mode */
#define ADC_DIVIDE_MODE                     0
#define ADC_BYPASS_MODE                     1
/* Change the ADC sampling mode here! */
#define ADC_MODE_DIVIDE_OR_BYPASS           ADC_DIVIDE_MODE

/* ADC sample channel config:P2_0~P2_7 and VBAT */
/* Change the ADC sampling pin here! */
#define ADC_SAMPLE_PIN_0                    P2_2
#define ADC_SAMPLE_CHANNEL_0                ADC_SAMPLE_PIN_0 - P2_0

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_adc_init(void)
{
    Pad_Config(ADC_SAMPLE_PIN_0, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
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
    ADC_InitStruct.adcSamplePeriod      = 255;
    for (uint8_t i = 0; i < 16; i++)
    {
#ifdef INTERNAL_VBAT_TEST
        ADC_InitStruct.schIndex[i] = INTERNAL_VBAT_MODE;
#else
        ADC_InitStruct.schIndex[i] = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL_0);
#endif
    }
    ADC_InitStruct.bitmap               = 0xFFFF;
    ADC_InitStruct.adcPowerAlwaysOnCmd  = ADC_POWER_ALWAYS_ON_ENABLE;
    ADC_Init(ADC, &ADC_InitStruct);

#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
    /* High bypass resistance mode config, please notice that the input voltage of
      adc channel using high bypass mode should not be over 0.9V */
    ADC_BypassCmd(ADC_SAMPLE_CHANNEL_0, ENABLE);
    DBG_DIRECT("[ADC]ADC sample mode is bypass mode !");
#else
    DBG_DIRECT("[ADC]ADC sample mode is divide mode !");
#endif
    ADC_INTConfig(ADC, ADC_INT_ONE_SHOT_DONE, ENABLE);
}

/**
  * @brief  Demo code of operation about adc.
  * @param  No parameter.
  * @return void
  */
void adc_demo(void)
{
    /* Initialize adc k value! */
    bool adc_k_status;
    adc_k_status = ADC_CalibrationInit();
    if (false == adc_k_status)
    {
        APP_PRINT_ERROR0("[ADC]ADC_CalibrationInit fail!");
    }
    /* Configure pad and pinmux firstly! */
    board_adc_init();

    /* Initialize adc peripheral */
    driver_adc_init();

    /* Enable adc */
    ADC_Cmd(ADC, ADC_One_Shot_Mode, ENABLE);
}

/**
  * @brief  ADC polling sample demo code.
  * @param  No parameter.
  * @return void
  */
void adc_sample_demo(void)
{
    uint16_t sample_data[16] = {0};
    float sample_voltage[16] = {0};
    ADC_ErrorStatus error_status = NO_ERROR;

    while (ADC_GetIntFlagStatus(ADC, ADC_INT_ONE_SHOT_DONE) == RESET);
    ADC_ClearINTPendingBit(ADC, ADC_INT_ONE_SHOT_DONE);
    for (uint8_t i = 0; i < 16; i++)
    {
        sample_data[i] = ADC_ReadByScheduleIndex(ADC, ADC_Schedule_Index_0 + i);
    }

    for (uint8_t i = 0; i < 16; i++)
    {
#if (ADC_MODE_DIVIDE_OR_BYPASS == ADC_BYPASS_MODE)
        sample_voltage[i] = ADC_GetVoltage(BYPASS_SINGLE_MODE, (int32_t)sample_data[i], &error_status);
#else
        sample_voltage[i] = ADC_GetVoltage(DIVIDE_SINGLE_MODE, (int32_t)sample_data[i], &error_status);
#endif
        if (error_status < 0)
        {
            /* Notes: DBG_DIRECT is only used for debug demo, do not use in app!
               In app project, the APP_PRINT_INFO can be used.
            */
            DBG_DIRECT("[ADC]adc_sample_demo: ADC parameter or efuse data error! i = %d, error_status = %d", i,
                       error_status);
//            APP_PRINT_INFO2("[ADC] adc_sample_demo: ADC parameter or efuse data error! i = %d, error_status = %d", i, error_status);
        }
        else
        {
            /* Notes: DBG_DIRECT is only used for debug demo, do not use in app!
               In app project, the APP_PRINT_INFO can be used.
            */
            DBG_DIRECT("[ADC]adc_sample_demo: ADC one shot mode sample data_%-4d = %d, voltage_%-4d = %fmV ",
                       i, sample_data[i], i, sample_voltage[i]);
//            APP_PRINT_INFO4("[ADC] adc_sample_demo: ADC one shot mode sample data_%-4d = %d, voltage_%-4d = %dmV ", i, sample_data[i], i, (uint32_t)sample_voltage[i]);
        }
    }
}

/**
  * @brief  Entry of app code
  * @return int (To avoid compile warning)
  */
int main(void)
{
    adc_demo();

    while (1)
    {
        /* Notes: DBG_DIRECT is only used for debug demo, do not use in app!
           In app project, the APP_PRINT_INFO can be used.
        */
        DBG_DIRECT("[ADC]Polling reads adc sample data !");
        adc_sample_demo();
        for (uint32_t i = 500000; i > 0; i--);
        ADC_Cmd(ADC, ADC_One_Shot_Mode, ENABLE);
    }
}


