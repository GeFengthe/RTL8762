/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     board.h
* @brief    Pin definitions
* @details
* @author   Chuanguo Xue
* @date     2015-4-7
* @version  v0.1
* *********************************************************************************************************
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/* Includes ------------------------------------------------------------------*/
#include "otp_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************
*                 RCU Keyscan Config
*******************************************************/
/* keypad row and column */
#define KEYPAD_ROW_SIZE       4
#define KEYPAD_COLUMN_SIZE    4

#define ROW0                  P2_2
#define ROW1                  P2_3
#define ROW2                  P2_4
#define ROW3                  P2_5
#define COLUMN0               P0_0
#define COLUMN1               P0_1
#define COLUMN2               P0_2
#define COLUMN3               P0_4

/*******************************************************
*              Battery Module Config
*******************************************************/
#define BAT_EN

#ifdef BAT_EN
#define BAT_LOW_POWER_THRESHOLD    2000        /*2.0v threshold for rcu system*/
#define BAT_LPC_CONFIG             LPC_2000_mV
#define BAT_ADC_DETECT_THRESHOLD   (BAT_LOW_POWER_THRESHOLD + 200)

//#define BAT_LOW_POWER_INDICATE     1 /*1 indicate enable, 0 disable */

#endif

/*******************************************************
*              LED Module Config
*******************************************************/
#define LED_EN          1

#define  LED_NUM_MAX   0x01
#define  LED_INDEX(n)   (n<<8)
/*uint16_t, first byte led index, last byte led pin*/
#define  LED_1         (LED_INDEX(0) | P0_5)

/* voltage level to trigger LED On action */
#define LED_ON_LEVEL_HIGH 1
#define LED_ON_LEVEL_LOW  0

#define LED_ON_LEVEL_TRIG LED_ON_LEVEL_HIGH

/*******************************************************
*                 DLPS Module Config
*******************************************************/
#define DLPS_EN        1

/** @defgroup IO Driver Config
  * @note user must config it firstly!! Do not change macro names!!
  * @{
  */

/* if use user define dlps enter/dlps exit callback function */
#define USE_USER_DEFINE_DLPS_EXIT_CB      1
#define USE_USER_DEFINE_DLPS_ENTER_CB     1

/* if use any peripherals below, #define it 1 */
#define USE_I2C0_DLPS        0
#define USE_I2C1_DLPS        0
#if (ROM_WATCH_DOG_ENABLE == 1)
#define USE_TIM_DLPS         1 //must be 1 if enable watch dog
#else
#define USE_TIM_DLPS         0
#endif
#define USE_QDECODER_DLPS    0
#define USE_IR_DLPS          0
#define USE_RTC_DLPS         0
#define USE_UART_DLPS        1
#define USE_ADC_DLPS         0
#define USE_SPI0_DLPS        0
#define USE_SPI1_DLPS        0
#define USE_SPI2W_DLPS       0
#define USE_KEYSCAN_DLPS     0
#define USE_DMIC_DLPS        0
#define USE_GPIO_DLPS        1
#define USE_PWM0_DLPS        0
#define USE_PWM1_DLPS        0
#define USE_PWM2_DLPS        0
#define USE_PWM3_DLPS        0


/* do not modify USE_IO_DRIVER_DLPS macro */
#define USE_IO_DRIVER_DLPS   (USE_I2C0_DLPS | USE_I2C1_DLPS | USE_TIM_DLPS | USE_QDECODER_DLPS\
                              | USE_IR_DLPS | USE_RTC_DLPS | USE_UART_DLPS | USE_SPI0_DLPS\
                              | USE_SPI1_DLPS | USE_SPI2W_DLPS | USE_KEYSCAN_DLPS | USE_DMIC_DLPS\
                              | USE_GPIO_DLPS | USE_USER_DEFINE_DLPS_EXIT_CB\
                              | USE_RTC_DLPS | USE_PWM0_DLPS | USE_PWM1_DLPS | USE_PWM2_DLPS\
                              | USE_PWM3_DLPS | USE_USER_DEFINE_DLPS_ENTER_CB)


/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif

