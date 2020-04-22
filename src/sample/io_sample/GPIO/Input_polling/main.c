/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of gpio input mode.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#define GPIO_INPUT_PIN_0        P2_2
#define GPIO_PIN_INPUT          GPIO_GetPin(GPIO_INPUT_PIN_0)

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_gpio_init(void)
{
    Pad_Config(GPIO_INPUT_PIN_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);

    Pinmux_Config(GPIO_INPUT_PIN_0, DWGPIO);
}

/**
  * @brief  Initialize GPIO peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_gpio_init(void)
{
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_PIN_INPUT;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
}

/**
  * @brief  Demo code of operation about gpio.
  * @param  No parameter.
  * @return void
  */
void gpio_demo(void)
{
    /* Configure pad and pinmux firstly! */
    board_gpio_init();

    /* Initialize gpio peripheral */
    driver_gpio_init();

}

/**
  * @brief  Entry of APP code
  * @return int (To avoid compile warning)
  */
int main(void)
{
    gpio_demo();

    while (1)
    {
        /* Read GPIO input value: gpio_input_data */
        uint8_t gpio_input_data = GPIO_ReadInputDataBit(GPIO_PIN_INPUT);
        for (uint32_t i = 0; i < 100000; i++);
        /** User code here.
          * for example: print the value of gpio_input_data;
          */
    }
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
