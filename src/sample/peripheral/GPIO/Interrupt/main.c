/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of GPIO Interrupt mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"
#include "app_msg.h"
#include "app_task.h"
#include "gpio_app.h"
#include <trace.h>
#include <os_sched.h>
#include "board.h"

/**
 * @brief    pinmux configuration
 * @return   void
 */
void PINMUX_Configuration(void)
{
    Pinmux_Config(Output_Pin, DWGPIO);
    Pinmux_Config(Int_Pin, DWGPIO);
    return;
}
/**
 * @brief    pad configuration
 * @return   void
 */
void PAD_Configuration(void)
{
    Pad_Config(Output_Pin, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(Int_Pin, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
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
uint8_t keystatus;
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

    Gpio_Struct.GPIO_Pin = GPIO_Int_Pin;
    Gpio_Struct.GPIO_Mode = GPIO_Mode_IN;
    Gpio_Struct.GPIO_ITCmd = ENABLE;
    Gpio_Struct.GPIO_ITTrigger = GPIO_INT_Trigger_EDGE;
    Gpio_Struct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;
    Gpio_Struct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
    Gpio_Struct.GPIO_DebounceTime = 20;
    GPIO_Init(&Gpio_Struct);
//    keystatus = GPIO_ReadInputDataBit(GPIO_Input_Pin);
//    DBG_DIRECT("keystatus is %d",keystatus);
    GPIO_MaskINTConfig(GPIO_Int_Pin, DISABLE);
    GPIO_INTConfig(GPIO_Int_Pin, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GPIO_Int_Pin_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}


/**
* @brief GPIO interrupt handler function.
* @param   No parameter.
* @return  void
*/
void GPIOInputIntrHandler(void)
{
    T_IO_MSG bee_io_msg;
    GPIO_MaskINTConfig(GPIO_Int_Pin, ENABLE);
    APP_PRINT_INFO0("Enter GPIO Interrupt");
    keystatus = GPIO_ReadInputDataBit(GPIO_Int_Pin);
    if (keystatus == 0)
    {
        GPIO->INTPOLARITY |= GPIO_Int_Pin;
        bee_io_msg.type = IO_MSG_TYPE_GPIO;
        bee_io_msg.subtype = MSG_KEY_PRESS;
        app_send_msg_to_apptask(&bee_io_msg);
    }
    else
    {
        GPIO->INTPOLARITY &= ~GPIO_Int_Pin;
        bee_io_msg.type = IO_MSG_TYPE_GPIO;
        bee_io_msg.subtype = MSG_KEY_RELEASE;
        app_send_msg_to_apptask(&bee_io_msg);
    }

    GPIO_ClearINTPendingBit(GPIO_Int_Pin);
    GPIO_MaskINTConfig(GPIO_Int_Pin, DISABLE);
}
/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Scatternet APP, thus only one APP task is init here
 * @return   void
 */
void task_init(void)
{
    app_task_init();
}
int main(void)
{
    board_init();
    task_init();
    os_sched_start();
}



