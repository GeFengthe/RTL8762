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
#include "rtl876x_spi.h"
#include "trace.h"

#include "board.h"

#include "app_task.h"


void driver_spi_gdma_init(void);
void io_handle_gdma_msg(T_IO_MSG *io_gdma_msg);


#ifdef __cplusplus
}
#endif

#endif

