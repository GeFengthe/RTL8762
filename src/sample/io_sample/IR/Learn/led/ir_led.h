/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      ir_led.h
* @brief
* @details
* @author    yuan
* @date      2019-01-24
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __IR_LED_H
#define __IR_LED_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"

#if (IR_FUN_EN && LED_IR_FUN_EN)
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"

/* Defines ------------------------------------------------------------------*/
#define LED_IR_SEND_OUT_PIN      GPIO_GetPin(LED_IR_SEND_PIN)
#if (IR_FUN_LEARN_EN)
#define LED_IR_LEARN_OUT_PIN     GPIO_GetPin(LED_IR_LEARN_PIN)
#endif

/* Function declaration --------------------------------------------------------*/
void board_ir_led_init(void);
void driver_ir_led_init(void);
void led_ir_send_swap(void);
void led_ir_learn_swap(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /*__IR_LED_H*/


/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/

