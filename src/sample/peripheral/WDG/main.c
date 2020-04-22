/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of WDG mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
#include "rtl876x_wdg.h"
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"
#include "trace.h"
#include "os_sched.h"
#include "board.h"

/**
 * @brief    pinmux configuration
 * @return   void
 */
void PINMUX_Configuration(void)
{

    Pinmux_Config(Input_Pin, DWGPIO);
    return;
}
/**
 * @brief    pad configuration
 * @return   void
 */
void PAD_Configuration(void)
{

    Pad_Config(Input_Pin, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
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
    Gpio_Struct.GPIO_Pin = GPIO_Input_Pin;
    Gpio_Struct.GPIO_Mode = GPIO_Mode_IN;
    Gpio_Struct.GPIO_ITCmd = ENABLE;
    Gpio_Struct.GPIO_ITTrigger = GPIO_INT_Trigger_EDGE;
    Gpio_Struct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;
    Gpio_Struct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
    Gpio_Struct.GPIO_DebounceTime = 20;
    GPIO_Init(&Gpio_Struct);
    GPIO_MaskINTConfig(GPIO_Input_Pin, DISABLE);
    GPIO_INTConfig(GPIO_Input_Pin, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GPIO_Input_Pin_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void WDG_SampleCode(void)
{
    WDG_ClockEnable();
    WDG_Config(150, 11, RESET_ALL);
    WDG_Enable();
    //Add application code here
}
int main()
{
    WDG_Disable();
    __enable_irq();
    board_init();
    driver_init();
    WDG_SampleCode();
    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
    }
}
/**
* @brief GPIO interrupt handler function.
* @param   No parameter.
* @return  void
*/
void GPIOInputIntrHandler(void)
{
    GPIO_MaskINTConfig(GPIO_Input_Pin, ENABLE);
    // add user code here
    WDG_SampleCode();
    GPIO_ClearINTPendingBit(GPIO_Input_Pin);
    GPIO_MaskINTConfig(GPIO_Input_Pin, DISABLE);
}


