/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of LPC mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "rtl876x_rcc.h"
#include "rtl876x_lpc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_nvic.h"
#include "rtl876x_gpio.h"
#include "rtl876x_wdg.h"
#include "board.h"
#include "trace.h"
#include "os_sched.h"
/* Defines ------------------------------------------------------------------*/

void PINMUX_Configuration(void)
{

    Pinmux_Config(Output_Pin, DWGPIO);
    Pinmux_Config(LPC_CAPTURE_PIN, IDLE_MODE);
    return;
}
/**
  * @brief  initialization of pad settings.
  * @param   No parameter.
  * @return  void
  */
void Board_LPC_Init(void)
{
    Pad_Config(LPC_CAPTURE_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);
    Pad_Config(Output_Pin, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
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
  * @brief  Initialize GPIO peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_GPIO_init(void)
{
    GPIO_InitTypeDef Gpio_Struct;
    GPIO_StructInit(&Gpio_Struct);
    Gpio_Struct.GPIO_Pin = GPIO_GetPin(Output_Pin);
    Gpio_Struct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(&Gpio_Struct);

}
/**
  * @brief  Initialize LPC peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_LPC_Init(void)
{
    LPC_InitTypeDef LPC_InitStruct;
    LPC_StructInit(&LPC_InitStruct);
    LPC_InitStruct.LPC_Channel   = LPC_CHANNEL_P2_7;
    LPC_InitStruct.LPC_Edge      = LPC_Vin_Over_Vth;
    /*!< threshold voltage = (LPC_Threshold +1) *80mv*/
    LPC_InitStruct.LPC_Threshold = LPC_2000_mV;
    LPC_Init(&LPC_InitStruct);
    LPC_Cmd(ENABLE);

    LPC_INTConfig(LPC_INT_VOLTAGE_COMP, ENABLE);
    RTC_SystemWakeupConfig(ENABLE);

    /* Config LPC interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = LPCOMP_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  demo code of operation about LPC.
  * @param   No parameter.
  * @return  void
  */
void LPC_SampleCode(void)
{
    PINMUX_Configuration();
    /* Configure PAD and pinmux firstly! */
    Board_LPC_Init();
    RCC_Configuration();
    /* Initialize LPC peripheral */
    Driver_LPC_Init();
    Driver_GPIO_init();
}
int main()
{
    WDG_Disable();
    __enable_irq();
    LPC_SampleCode();
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
  * @brief  LPC interrupt handle function.
  * @param  None.
  * @return None.
  */

void LPCOMP_Handler(void)
{
    /* add code */
    APP_PRINT_INFO0("LPCOMP");
    GPIO_SetBits(GPIO_Output_Pin);
    LPC_INTConfig(LPC_INT_VOLTAGE_COMP, DISABLE);
}
