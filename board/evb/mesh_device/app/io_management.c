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
#include "rtl876x_gpio.h"
#include "rtl876x_rcc.h"
#include "app_task.h"

#if ENABLE_DLPS
void lpn_board_init(void)
{
}

void gpio_driver_init(void)
{
	Pad_Config(LPN_BUTTON, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pinmux_Config(LPN_BUTTON, DWGPIO);
	
	
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
    GPIO_InitTypeDef Gpio_Struct;
    GPIO_StructInit(&Gpio_Struct);

    Gpio_Struct.GPIO_Pin = GPIO_GetPin(LPN_BUTTON);
    Gpio_Struct.GPIO_Mode = GPIO_Mode_IN;
    Gpio_Struct.GPIO_ITCmd = DISABLE;
    GPIO_Init(&Gpio_Struct);
	
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
