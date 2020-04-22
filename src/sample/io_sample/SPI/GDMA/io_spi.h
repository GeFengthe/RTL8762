/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_spi.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __IO_SPI_H
#define __IO_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_spi.h"

#include "board.h"

#include "app_msg.h"


void board_spi_init(void);
void driver_spi_init(void);
void spi_demo(void);
void io_handle_spi_msg(T_IO_MSG *io_spi_msg);


#ifdef __cplusplus
}
#endif

#endif

