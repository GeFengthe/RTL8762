/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_uart.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __IO_UART_H
#define __IO_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_uart.h"

#include "board.h"

#include "app_msg.h"

/* Globals ------------------------------------------------------------------*/
extern bool IO_UART_DLPS_Enter_Allowed;

void global_data_uart_init(void);
void board_uart_init(void);
void driver_uart_init(void);
void io_uart_dlps_enter(void);
void io_uart_dlps_exit(void);
bool io_uart_dlps_check(void);
void io_handle_uart_msg(T_IO_MSG *io_uart_msg);
void uart_senddata_continuous(UART_TypeDef *UARTx, const uint8_t *pSend_Buf, uint16_t vCount);


#ifdef __cplusplus
}
#endif

#endif

