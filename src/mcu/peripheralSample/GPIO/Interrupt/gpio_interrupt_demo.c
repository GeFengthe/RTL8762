/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
    * @file     gpio_interrupt_demo.c
    * @brief    This file provides demo code of gpio interrupt mode.
    * @details
    * @author   yuan
    * @date     2018-06-22
    * @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#define GPIO_INPUT_PIN_0        P2_2
#define GPIO_PIN_INPUT          GPIO_GetPin(GPIO_INPUT_PIN_0)
#define GPIO_PIN_INPUT_IRQN     GPIO18_IRQn
#define GPIO_Input_Handler      GPIO18_Handler

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
    GPIO_InitStruct.GPIO_ITCmd  = ENABLE;
    GPIO_InitStruct.GPIO_ITTrigger  = GPIO_INT_Trigger_LEVEL;
    GPIO_InitStruct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_HIGH;
    GPIO_InitStruct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_DISABLE;
    GPIO_Init(&GPIO_InitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GPIO_PIN_INPUT_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    GPIO_MaskINTConfig(GPIO_PIN_INPUT, DISABLE);
    GPIO_INTConfig(GPIO_PIN_INPUT, ENABLE);
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

void GPIO_Input_Handler(void)
{
    GPIO_INTConfig(GPIO_PIN_INPUT, DISABLE);
    GPIO_MaskINTConfig(GPIO_PIN_INPUT, ENABLE);
//    APP_PRINT_INFO0("Enter GPIO Interrupt");
    DBG_DIRECT("Enter GPIO Interrupt!");

    GPIO_ClearINTPendingBit(GPIO_PIN_INPUT);
    GPIO_MaskINTConfig(GPIO_PIN_INPUT, DISABLE);
    GPIO_INTConfig(GPIO_PIN_INPUT, ENABLE);

}
