/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_adc.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __IO_ADC_H
#define __IO_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_adc.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"

#include "board.h"

#include "app_msg.h"

#include "bee2_adc_lib.h"


/* Defines ------------------------------------------------------------------*/
/** ADC sample channel config.
  * ADC pin select:P2_0~P2_7 and VBAT.
  * ADC sample channel index select: #define ADC_Channel_Index_0         0
  *                                  #define ADC_Channel_Index_1         1
  *                                  #define ADC_Channel_Index_2         2
  *                                  #define ADC_Channel_Index_3         3
  *                                  #define ADC_Channel_Index_4         4
  *                                  #define ADC_Channel_Index_5         5
  *                                  #define ADC_Channel_Index_6         6
  *                                  #define ADC_Channel_Index_7         7
  * If ADC_SAMPLE_PIN = P2_0, then ADC_SAMPLE_CHANNEL = ADC_Channel_Index_0;
  * If ADC_SAMPLE_PIN = P2_4, then ADC_SAMPLE_CHANNEL = ADC_Channel_Index_4;
  */
#define ADC_SAMPLE_CHANNEL_0        ADC_SAMPLE_PIN_0 - P2_0 //ADC_Channel_Index_3
#define ADC_SAMPLE_CHANNEL_1        ADC_SAMPLE_PIN_1 - P2_0 //ADC_Channel_Index_3

/** ADC mode config.
  * ADC pin hardware mode: bypass mode
  *                        divide mode
  */
#define ADC_DIVIDE_MODE                     0
#define ADC_BYPASS_MODE                     1
#define ADC_MODE_DIVIDE_OR_BYPASS           ADC_DIVIDE_MODE

#define ADC_CONTINUOUS_SAMPLE_PERIOD        (200-1)//sampling once 20ms

#define ADC_FIFO_MAX                        (32)

/* ADC global data struct*/
typedef struct
{

    uint16_t RawData[ADC_FIFO_MAX + 1];
//    float voltage[ADC_FIFO_THD];
} ADC_Data_TypeDef;

extern ADC_Data_TypeDef ADC_Global_Data;

void global_data_adc_init(void);
void board_adc_init(void);
void driver_adc_init(void);
void io_handle_adc_msg(T_IO_MSG *io_adc_msg);


#ifdef __cplusplus
}
#endif

#endif

