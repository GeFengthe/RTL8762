/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      ir_driver_rx.h
* @brief
* @details
* @author    yuan
* @date      2019-01-23
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __IR_DRIVER_RX_H
#define __IR_DRIVER_RX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <string.h>

#include "rtl876x_ir.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"

#include "board.h"

#include "trace.h"

#include "app_msg.h"

#include "ir_learn_config.h"


/** @defgroup pFn_IR_RX_Handler_CB
  * @brief Function pointer type used by IR learn interrupt handle to general callback and send events to application.
  * @{
  */
typedef void (*pFn_IR_RX_Handler_CB_t)(uint8_t vFlag);
/**
  * @} End of pFn_IR_RX_Handler_CB
  */
extern pFn_IR_RX_Handler_CB_t pFn_IR_RX_Handler_CB;


void board_ir_rx_init(void);
void driver_ir_rx_init(void);
void int_ir_rx_config(FunctionalState state);
void ir_rx_handler_cb(pFn_IR_RX_Handler_CB_t pFunc);
void IR_Handler(void);
void IR_RX_Handler(void);


#ifdef __cplusplus
}
#endif

#endif /*__IR_DRIVER_RX_H*/

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/

