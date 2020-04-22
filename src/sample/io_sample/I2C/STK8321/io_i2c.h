/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_i2c.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __IO_I2C_H
#define __IO_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_i2c.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"

#include "board.h"

#include "app_msg.h"


void board_i2c_init(void);
void driver_i2c_init(void);
void i2c_demo(void);
void io_handle_i2c_msg(T_IO_MSG *io_i2c_msg);


#ifdef __cplusplus
}
#endif

#endif

