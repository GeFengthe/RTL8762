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
/* Only channel 0 and channel 2 have multiple block mode. */
#define GDMA_CHANNEL_NUM                2
#define GDMA_Channel                    GDMA_Channel2
#define GDMA_Channel_IRQn               GDMA0_Channel2_IRQn
#define GDMA_Channel_Handler            GDMA0_Channel2_Handler

/* The maximum number of GDMA single block is 4095. */
#define GDMA_TRANSFER_SIZE              200

#define GDMA_MULTIBLOCK_SIZE            12

/** GDMA multi block mode.
  * AUTO_RELOAD_WITH_CONTIGUOUS_SAR
  * AUTO_RELOAD_WITH_CONTIGUOUS_DAR
  * AUTO_RELOAD_TRANSFER
  * LLI_WITH_CONTIGUOUS_SAR
  * LLI_WITH_AUTO_RELOAD_SAR
  * LLI_WITH_CONTIGUOUS_DAR
  * LLI_WITH_AUTO_RELOAD_DAR
  * LLI_TRANSFER
  */
#define GDMA_MULTIBLOCK_MODE            LLI_TRANSFER

#define INT_TRANSFER                    0
#define INT_BLOCK                       1
#define GDMA_INTERRUPT_MODE             INT_BLOCK

void driver_gdma_init(void);
void io_handle_gdma_msg(T_IO_MSG *io_gdma_msg);


#ifdef __cplusplus
}
#endif

#endif

