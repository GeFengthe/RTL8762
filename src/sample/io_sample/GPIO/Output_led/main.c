/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of gpio output mode.
            Control LED flashing.
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

/* Defines ------------------------------------------------------------------*/
/* P0_1 is connected to LED0 on EVB board */
#define GPIO_OUTPUT_PIN_0       P0_1
#define GPIO_PIN_OUTPUT         GPIO_GetPin(GPIO_OUTPUT_PIN_0)

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_gpio_init(void)
{
    Pad_Config(GPIO_OUTPUT_PIN_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pinmux_Config(GPIO_OUTPUT_PIN_0, DWGPIO);
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
    GPIO_InitStruct.GPIO_Pin    = GPIO_PIN_OUTPUT;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
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
  * @brief    Entry of app code
  * @return   int(To avoid compile warning)
  */
int main(void)
{
    gpio_demo();

    while (1)
    {
        /* Light up LED0 */
        GPIO_WriteBit(GPIO_PIN_OUTPUT, (BitAction)(1));
        for (uint32_t i = 0; i < 100000; i++);
        /* Lights out LED0 */
        GPIO_WriteBit(GPIO_PIN_OUTPUT, (BitAction)(0));
        for (uint32_t i = 0; i < 100000; i++);
    }
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
