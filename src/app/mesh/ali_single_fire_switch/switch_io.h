/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      switch_io.h
* @brief
* @details
* @author    Elliot Chen
* @date      2018-9-13
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _SWITCH_IO_
#define _SWITCH_IO_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"
#include "board.h"
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"
#include "switch_dlps_ctrl.h"
#include "app_task.h"
#include "string.h"

typedef enum
{
    LEFT_SWITCH_PRESS,
    LEFT_SWITCH_RELEASE,
} SIWTCH_MSG_TYPE;

typedef union
{
    uint8_t all_switch_status;
    struct
    {
        uint8_t left_switch_status_bit: 1;
        uint8_t resvd: 7;
    } switch_status_bit;
} SWITCH_STATUS;

void board_switch_io_init(void);
void driver_switch_io_init(void);
SWITCH_STATUS *switch_get_status(void);
void switch_io_enter_dlps_config(void);
void switch_io_exit_dlps_config(void);
void switch_io_handle_msg_exit_dlps(void);
void switch_handle_io_msg(T_IO_MSG *io_msg);
void switch_light_cmd(bool is_on);
#ifdef __cplusplus
}
#endif

#endif

/* _SWITCH_IO_ */

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

