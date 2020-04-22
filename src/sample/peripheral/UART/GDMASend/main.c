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
#include "rtl876x_gdma.h"
#include "rtl876x_wdg.h"
#include "trace.h"
#include <string.h>
/* GDAM parameters configuration */
#define USER_GDMA_INDEX               2
#define USER_GDMA_CHANNEL             GDMA_Channel2
#define USER_GDMA_CHANNEL_IRQ       GDMA0_Channel2_IRQn
#define USERGDMAIntrHandler         GDMA0_Channel2_Handler


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
    /* Enable UART and GDMA clock */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
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
    uartInitStruct.dmaEn = UART_DMA_ENABLE;
    uartInitStruct.TxDmaEn = UART_DMA_ENABLE;
    /* Baudrate = 9600 */
    uartInitStruct.div = 271;
    uartInitStruct.ovsr = 10;
    uartInitStruct.ovsr_adj = 0x24A;
    UART_Init(UART, &uartInitStruct);
}
void GDMA_TXConfiguration(void)
{
    /* Initialize GDMA */
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = USER_GDMA_INDEX;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = 0;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)(0);
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(&(UART->RB_THR));
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_UART0_TX;
    GDMA_Init(USER_GDMA_CHANNEL, &GDMA_InitStruct);
    /* Enable transfer finish interrupt */
    GDMA_INTConfig(USER_GDMA_INDEX, GDMA_INT_Transfer, ENABLE);
    /* Configure NVIC of GDMA */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USER_GDMA_CHANNEL_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}
int main(void)
{
    uint8_t strLen = 0;
    char *Str = "### Welcome to use RealTek Bee2 ###\r\n";
    WDG_Disable();
    __enable_irq();
    board_init();
    DataUART_Init();
    GDMA_TXConfiguration();
    strLen = strlen(Str);

    GDMA_SetBufferSize(USER_GDMA_CHANNEL, strLen);
    /* Configure GDMA Source address */
    GDMA_SetSourceAddress(USER_GDMA_CHANNEL, (uint32_t)Str);
    /* Enable GDMA to transmission */
    GDMA_Cmd(USER_GDMA_INDEX, ENABLE);
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
* @brief GDMA interrupt handler function.
* @param   No parameter.
* @return  void
*/
void USERGDMAIntrHandler(void)
{
    GDMA_ClearAllTypeINT(USER_GDMA_INDEX);
}
