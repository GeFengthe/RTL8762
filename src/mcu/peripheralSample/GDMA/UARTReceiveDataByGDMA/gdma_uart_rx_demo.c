/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     gdma_uart_rx_demo.c
* @brief    This file provides demo code of uart receive data by gdma.
* @details
* @author   yuan
* @date     2018-06-28
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_uart.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#define UART_TX_PIN                     P3_2
#define UART_RX_PIN                     P3_3

#define GDMA_CHANNEL_NUM                3
#define GDMA_Channel                    GDMA_Channel3
#define GDMA_Channel_IRQn               GDMA0_Channel3_IRQn
#define GDMA_Channel_Handler            GDMA0_Channel3_Handler

#define GDMA_TRANSFER_SIZE              255

/* Globals ------------------------------------------------------------------*/
uint8_t GDMA_Recv_Buffer[GDMA_TRANSFER_SIZE];
//uint8_t String_Buf[100];
//uint8_t UART_Rev_Data[256];
//uint8_t UART_Rev_Data_End_Flag = 0;
//uint16_t UART_Rev_Data_Lenth = 0;
//uint16_t UART_Rev_Data_Index = 0;

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_uart_init(void)
{
    Pad_Config(UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);

    Pinmux_Config(UART_TX_PIN, UART0_TX);
    Pinmux_Config(UART_RX_PIN, UART0_RX);
}

/**
  * @brief  Initialize uart peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_uart_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);

    /* uart init */
    UART_InitTypeDef UART_InitStruct;
    UART_StructInit(&UART_InitStruct);

    UART_InitStruct.dmaEn   = UART_DMA_ENABLE;
    UART_InitStruct.RxDmaEn = ENABLE;
    UART_Init(UART, &UART_InitStruct);
}

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_gdma_uart_init(void)
{
    uint32_t i = 0;

    /* Initialize test buffer which for storing data from UART */
    for (i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        GDMA_Recv_Buffer[i] = 0;
    }

    /* GDMA init */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);

    GDMA_InitStruct.GDMA_ChannelNum      = GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR             = GDMA_DIR_PeripheralToMemory;
    GDMA_InitStruct.GDMA_BufferSize      = GDMA_TRANSFER_SIZE;//determine total transfer size
    GDMA_InitStruct.GDMA_SourceInc       = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc  = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize  = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize        = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize   = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr         = (uint32_t)(&(UART->RB_THR));
    GDMA_InitStruct.GDMA_DestinationAddr    = (uint32_t)GDMA_Recv_Buffer;
    GDMA_InitStruct.GDMA_SourceHandshake    = GDMA_Handshake_UART0_RX;
    GDMA_Init(GDMA_Channel, &GDMA_InitStruct);

    /* GDMA irq config */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);

    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    /* Start to receive data */
    GDMA_Cmd(GDMA_CHANNEL_NUM, ENABLE);
}

/**
  * @brief  Demo code of operation about uart + gdma.
  * @param  No parameter.
  * @return void
*/
void gdma_uart_demo(void)
{
    board_uart_init();
    driver_uart_init();
    driver_gdma_uart_init();
}

/**
  * @brief  Demo code of operation about uart + gdma.
  * @param  No parameter.
  * @return void
*/
void gdma_demo(void)
{
    gdma_uart_demo();
}

/**
  * @brief  GDMA0 channel interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void GDMA_Channel_Handler(void)
{
    GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
    for (uint32_t i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        DBG_DIRECT("GDMA receive data[%d] = %d", i, GDMA_Recv_Buffer[i]);
        GDMA_Recv_Buffer[i] = 0;
    }

    GDMA_Cmd(GDMA_CHANNEL_NUM, ENABLE);
}
