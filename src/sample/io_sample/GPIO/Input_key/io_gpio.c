/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_gpio.c
* @brief    This file provides demo code of gpio input mode.
            Get key value.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "io_gpio.h"

#include "app_task.h"

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
    GPIO_InitStruct.GPIO_Pin        = GPIO_PIN_INPUT;
    GPIO_InitStruct.GPIO_Mode       = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd      = ENABLE;
    GPIO_InitStruct.GPIO_ITTrigger  = GPIO_INT_Trigger_EDGE;
    GPIO_InitStruct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;
    GPIO_InitStruct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
    GPIO_InitStruct.GPIO_DebounceTime = 10;/* unit:ms , can be 1~64 ms */
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
  * @brief  GPIO interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void GPIO_Input_Handler(void)
{
    GPIO_INTConfig(GPIO_PIN_INPUT, DISABLE);
    GPIO_MaskINTConfig(GPIO_PIN_INPUT, ENABLE);

    T_IO_MSG int_gpio_msg;

    int_gpio_msg.type = IO_MSG_TYPE_GPIO;
    int_gpio_msg.subtype = 0;
    if (false == app_send_msg_to_apptask(&int_gpio_msg))
    {
        APP_PRINT_ERROR0("[io_gpio] GPIO_Input_Handler: Send int_gpio_msg failed!");
        //Add user code here!
        GPIO_ClearINTPendingBit(GPIO_PIN_INPUT);
        return;
    }

    GPIO_ClearINTPendingBit(GPIO_PIN_INPUT);
    GPIO_MaskINTConfig(GPIO_PIN_INPUT, DISABLE);
    GPIO_INTConfig(GPIO_PIN_INPUT, ENABLE);
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
