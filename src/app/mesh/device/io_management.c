/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     io_management.c
* @brief    This file configures the peripherals.
* @details
* @author   astor zhang
* @date     2019-12-30
* @version  v1.0
*********************************************************************************************************
*/


#include "board.h"
#include "rtl876x_pinmux.h"
#include "io_management.h"
#include "rtl876x_uart.h"
#include "app_task.h"

#if ENABLE_DLPS
void lpn_board_init(void)
{
#if defined(MESH_DONGLE)
    Pad_Config(LPN_BUTTON, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(LED_R, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(LED_G, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(LED_B, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(LED_W, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
#elif defined(MESH_EVB)
    Pad_Config(LPN_BUTTON, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(LED_R, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(LED_G, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(LED_B, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(LED_W, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE, PAD_OUT_LOW);
#endif
}

void gpio_driver_init(void)
{
}

void uart_deinit(void)
{
    Pad_Config(P3_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(P3_1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);

    Pinmux_Config(P3_0, IDLE_MODE);
    Pinmux_Config(P3_1, IDLE_MODE);
}

void uart_re_init(void)
{
    uart_init();
}
#endif
