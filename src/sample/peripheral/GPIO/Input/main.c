/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of GPIO input mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_wdg.h"
#include "rtl876x_gpio.h"
#include "board.h"
#include <trace.h>

/**
 * @brief    pinmux configuration
 * @return   void
 */
void PINMUX_Configuration(void)
{
    Pinmux_Config(Output_Pin, DWGPIO);
    Pinmux_Config(Input_Pin, DWGPIO);
    return;
}
/**
 * @brief    pad configuration
 * @return   void
 */
void PAD_Configuration(void)
{
    Pad_Config(Output_Pin, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(Input_Pin, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    return;
}

/**
 * @brief    rcc configuration
 * @return   void
 */
void RCC_Configuration(void)
{
    /* Enable GPIO clock */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
    return;
}

/**
 * @brief    Contains the initialization of pinmux settings and pad settings
 * @note     All the pinmux settings and pad settings shall be initiated in this function,
 *           but if legacy driver is used, the initialization of pinmux setting and pad setting
 *           should be peformed with the IO initializing.
 * @return   void
 */
void board_init(void)
{
    RCC_Configuration();
    PINMUX_Configuration();
    PAD_Configuration();
}

/**
  * @brief  Initialize GPIO peripheral.
  * @param   No parameter.
  * @return  void
  */
void driver_init(void)
{
    GPIO_InitTypeDef Gpio_Struct;
    GPIO_StructInit(&Gpio_Struct);

    Gpio_Struct.GPIO_Pin = GPIO_GetPin(Output_Pin);
    Gpio_Struct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(&Gpio_Struct);

    Gpio_Struct.GPIO_Pin = GPIO_GetPin(Input_Pin);
    Gpio_Struct.GPIO_Mode = GPIO_Mode_IN;
    Gpio_Struct.GPIO_ITCmd = DISABLE;
    GPIO_Init(&Gpio_Struct);

}
/**
  * @brief  common operation of GPIO input mode.
  * @param   No parameter.
  * @return  void
  */
//void GPIO_DemoCode(void)
//{
//    uint8_t bit_status =  GPIO_ReadInputDataBit(GPIO_Input_Pin);
//}
int main(void)
{
    WDG_Disable();
    board_init();
    driver_init();
//    GPIO_DemoCode();

    while (1)
    {
        if (!GPIO_ReadInputDataBit(GPIO_Input_Pin))
        {
            GPIO_SetBits(GPIO_Output_Pin);
        }
        else
        {
            GPIO_ResetBits(GPIO_Output_Pin);
        }
    };

}
