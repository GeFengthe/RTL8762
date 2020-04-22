/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_gdma.h
* @brief
* @details
* @author   yuan
* @date     2019-01-11
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __IO_GDMA_H
#define __IO_GDMA_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "trace.h"

#include "board.h"

#include "app_task.h"

/* Defines ------------------------------------------------------------------*/
#define GDMA_CHANNEL_NUM                4
#define GDMA_Channel                    GDMA_Channel4
#define GDMA_Channel_IRQn               GDMA0_Channel4_IRQn
#define GDMA_Channel_Handler            GDMA0_Channel4_Handler

/* The maximum number of GDMA single block is 4095. */
#define GDMA_TRANSFER_SIZE              200

void driver_gdma_init(void);
void io_handle_gdma_msg(T_IO_MSG *io_gdma_msg);


#ifdef __cplusplus
}
#endif

#endif

