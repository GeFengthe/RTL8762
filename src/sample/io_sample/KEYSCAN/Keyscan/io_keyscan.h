/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_keyscan.h
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
#include "rtl876x_keyscan.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"

#include "board.h"

#include "app_msg.h"


/* Defines -------------------------------------------------------------------*/
#define KEYSCAN_SW_INTERVAL             (200)/* 200ms */
#define KEYSCAN_SW_RELEASE_TIMEOUT      (10)/* 10ms */

/* Typedefs ------------------------------------------------------------------*/
typedef struct
{
    uint32_t length;
    struct
    {
        uint16_t column: 5;   /**< keyscan column buffer data   */
        uint16_t row: 4;      /**< keyscan raw buffer data      */
    } key[26];
} KeyScan_Data_TypeDef;


extern void *KeyScan_Timer_Handle;
extern KeyScan_Data_TypeDef Current_Key_Data;


void global_data_keyscan_init(void);
void board_keyboard_init(void);
void driver_keyboard_init(uint32_t vDebounce_En);
void timer_keyscan_init(void);
void io_handle_keyscan_msg(T_IO_MSG *io_keyscan_msg);

#ifdef __cplusplus
}
#endif

#endif

