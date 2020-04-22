/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_lpc.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __IO_LPC_H
#define __IO_LPC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_lpc.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "trace.h"

#include "board.h"

#include "trace.h"

/* Defines ------------------------------------------------------------------*/
/** LPC voltage detect channel config.
  * PIN select:P2_0~P2_7 and VBAT.
  * LPC channel select: #define LPC_CHANNEL_P2_0                ((uint32_t)0)
  *                     #define LPC_CHANNEL_P2_1                ((uint32_t)1)
  *                     #define LPC_CHANNEL_P2_2                ((uint32_t)2)
  *                     #define LPC_CHANNEL_P2_3                ((uint32_t)3)
  *                     #define LPC_CHANNEL_P2_4                ((uint32_t)4)
  *                     #define LPC_CHANNEL_P2_5                ((uint32_t)5)
  *                     #define LPC_CHANNEL_P2_6                ((uint32_t)6)
  *                     #define LPC_CHANNEL_P2_7                ((uint32_t)7)
  *                     #define LPC_CHANNEL_VBAT                ((uint32_t)8)
  * If LPC_CAPTURE_PIN = P2_0, then LPC_CAPTURE_CHANNEL = LPC_CHANNEL_P2_0;
  * If LPC_CAPTURE_PIN = P2_5, then LPC_CAPTURE_CHANNEL = LPC_CHANNEL_P2_5;
  */
#define LPC_CAPTURE_CHANNEL             LPC_CAPTURE_PIN - P2_0

#define LPC_VOLTAGE_DETECT_EDGE         LPC_Vin_Below_Vth
#define LPC_VOLTAGE_DETECT_THRESHOLD    LPC_2000_mV


void board_lpc_init(void);
void driver_lpc_init(void);
void nvic_lpc_config(void);


#ifdef __cplusplus
}
#endif

#endif

