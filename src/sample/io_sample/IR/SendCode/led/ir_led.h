/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      ir_led.h
* @brief
* @details
* @author    elliot chen
* @date      2018-07-23
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __IR_LED_H
#define __IR_LED_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"

/* Function declaration --------------------------------------------------------*/
void Board_All_LED_Init(void);
void Driver_All_LED_Init(void);
void LED_IR_Send_Swap(void);
void LED_IR_Learn_Swap(void);

#ifdef __cplusplus
}
#endif

#endif /*__IR_LED_H*/


/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

