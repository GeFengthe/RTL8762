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
/* Only channel 1 and channel 3 have scatter gather function. */
#define GDMA_CHANNEL_NUM                3
#define GDMA_Channel                    GDMA_Channel3
#define GDMA_Channel_IRQn               GDMA0_Channel3_IRQn
#define GDMA_Channel_Handler            GDMA0_Channel3_Handler

#define GDMA_TRANSFER_SIZE              (20)
#define GDMA_GATHER_EN                  ENABLE
#define GDMA_SCATTER_EN                 ENABLE
#define GDMA_GATHER_COUNT               (4)
#define GDMA_GATHER_INTERVAL            (4)
#define GDMA_SCATTER_COUNT              (4)
#define GDMA_SCATTER_INTERVAL           (4)

#define SOURCE_DATA_SIZE                (((GDMA_TRANSFER_SIZE/GDMA_GATHER_COUNT)+1)*(GDMA_GATHER_COUNT+GDMA_GATHER_INTERVAL)-GDMA_GATHER_INTERVAL)
#define DESTINATION_DATA_SIZE           (((GDMA_TRANSFER_SIZE/GDMA_SCATTER_COUNT)+1)*(GDMA_SCATTER_COUNT+GDMA_SCATTER_INTERVAL)-GDMA_SCATTER_INTERVAL)


void driver_gdma_init(void);
void io_handle_gdma_msg(T_IO_MSG *io_gdma_msg);


#ifdef __cplusplus
}
#endif

#endif

