/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of UART mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
#include "os_timer.h"
#include "os_sched.h"
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"
#include "rtl876x_uart.h"
#include "rtl876x_wdg.h"
#include "app_task.h"
#include "trace.h"
#include "polling_app.h"



/**
 * @brief    pinmux configuration
 * @return   void
 */
void PINMUX_Configuration(void)
{

    Pinmux_Config(P3_0, UART0_TX);
    Pinmux_Config(P3_1, UART0_RX);
    return;
}
/**
 * @brief    pad configuration
 * @return   void
 */
void PAD_Configuration(void)
{

    Pad_Config(P3_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(P3_1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    return;
}

/**
 * @brief    rcc configuration
 * @return   void
 */
void RCC_Configuration(void)
{
    /* Enable UART clock */
    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);
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
  * @brief  Initialize UART peripheral.
  * @param   No parameter.
  * @return  void
  */
void DataUART_Init(void)
{
    /* Initialize UART */
    UART_InitTypeDef uartInitStruct;
    UART_StructInit(&uartInitStruct);
    uartInitStruct.rxTriggerLevel = UART_RX_FIFO_TRIGGER_LEVEL_14BYTE;
    /* Baudrate = 9600 */
    uartInitStruct.div = 271;
    uartInitStruct.ovsr = 10;
    uartInitStruct.ovsr_adj = 0x24A;
//    uartInitStruct.div             = 4;
//    uartInitStruct.ovsr            = 5;
//    uartInitStruct.ovsr_adj        = 0;
    uartInitStruct.TxWaterlevel    = 12;
    UART_Init(UART, &uartInitStruct);

    /* NVIC init */
    UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_LINE_STS,  ENABLE);
    NVIC_ClearPendingIRQ(UART0_IRQn);

    /*  Enable UART IRQ  */
    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = UART0_IRQn;
    nvic_init_struct.NVIC_IRQChannelCmd      = ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&nvic_init_struct);

}
int main(void)
{
    WDG_Disable();
    __enable_irq();
    board_init();
    DataUART_Init();
    app_task_init();
    LOOP_BUFFER_Init();
    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
    }
}
