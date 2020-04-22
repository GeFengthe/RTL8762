/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      ir_led.c
* @brief    This file provides led display for ir application code.
* @details
* @author   elliot chen
* @date      2018-7-23
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "ir_led.h"

/* Defines ------------------------------------------------------------------*/
#define LED_IR_SEND_PIN             P0_1
#define LED_IR_LEARN_PIN            P0_2

#define LED_IR_SEND_GPIO_INDEX      GPIO_GetPin(LED_IR_SEND_PIN)
#define LED_IR_LEARN_GPIO_INDEX     GPIO_GetPin(LED_IR_LEARN_PIN)

/* Globals -------------------------------------------------------------------*/

/**
  * @brief  initialization of pinmux settings and pad settings.
  * @param   No parameter.
  * @return  void
  */
void Board_All_LED_Init(void)
{
    Pad_Config(LED_IR_SEND_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(LED_IR_LEARN_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pinmux_Config(LED_IR_SEND_PIN, DWGPIO);
    Pinmux_Config(LED_IR_LEARN_PIN, DWGPIO);
}

/**
  * @brief  Initialize GPIO peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_All_LED_Init(void)
{
    /* Enable GPIO clock */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    /* Initialize GPIO */
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin  = LED_IR_SEND_GPIO_INDEX | LED_IR_LEARN_GPIO_INDEX;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_ResetBits(LED_IR_SEND_GPIO_INDEX);
    GPIO_ResetBits(LED_IR_LEARN_GPIO_INDEX);
}

void LED_IR_Send_Swap(void)
{
    GPIO_WriteBit(LED_IR_SEND_GPIO_INDEX,
                  (BitAction)(1 - GPIO_ReadOutputDataBit(LED_IR_SEND_GPIO_INDEX)));
}

void LED_IR_Learn_Swap(void)
{
    GPIO_WriteBit(LED_IR_LEARN_GPIO_INDEX,
                  (BitAction)(1 - GPIO_ReadOutputDataBit(LED_IR_LEARN_GPIO_INDEX)));
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

