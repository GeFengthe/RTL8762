/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     ir_led.c
* @brief    This file provides led display for ir application code.
* @details
* @author   yuan
* @date     2019-01-24
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "ir_led.h"

#if (IR_FUN_EN && LED_IR_FUN_EN)

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_ir_led_init(void)
{
    Pad_Config(LED_IR_SEND_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pinmux_Config(LED_IR_SEND_PIN, DWGPIO);
#if (IR_FUN_LEARN_EN)
    Pad_Config(LED_IR_LEARN_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pinmux_Config(LED_IR_LEARN_PIN, DWGPIO);
#endif
}

/**
  * @brief  Initialize gpio peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_ir_led_init(void)
{
    /* Enable GPIO clock */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    /* Initialize GPIO */
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
#if (IR_FUN_LEARN_EN)
    GPIO_InitStruct.GPIO_Pin  = LED_IR_SEND_OUT_PIN | LED_IR_LEARN_OUT_PIN;
#else
    GPIO_InitStruct.GPIO_Pin  = LED_IR_SEND_OUT_PIN;
#endif
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(&GPIO_InitStruct);

    GPIO_ResetBits(LED_IR_SEND_OUT_PIN);
#if (IR_FUN_LEARN_EN)
    GPIO_ResetBits(LED_IR_LEARN_OUT_PIN);
#endif
}

void led_ir_send_swap(void)
{
    GPIO_WriteBit(LED_IR_SEND_OUT_PIN, (BitAction)(1 - GPIO_ReadOutputDataBit(LED_IR_SEND_OUT_PIN)));
}

#if (IR_FUN_LEARN_EN)
void led_ir_learn_swap(void)
{
    GPIO_WriteBit(LED_IR_LEARN_OUT_PIN, (BitAction)(1 - GPIO_ReadOutputDataBit(LED_IR_LEARN_OUT_PIN)));
}
#endif /* IR_FUN_LEARN_EN */

#endif /* (IR_FUN_EN && LED_IR_FUN_EN) */

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/

