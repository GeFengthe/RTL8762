/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of IPS6404L-SQ SPI/QPI PSRAM mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_spi.h"
#include "rtl876x_uart.h"
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"
#include "rtl876x_wdg.h"
#include <os_sched.h>
#include <trace.h>
#include "board.h"


/**
 * @brief    pinmux configuration
 * @return   void
 */
void PINMUX_Configuration(void)
{

    Pinmux_Config(SPI0_SCK, SPI0_CLK_MASTER);
    Pinmux_Config(SPI0_MOSI, SPI0_MO_MASTER);
    Pinmux_Config(SPI0_MISO, SPI0_MI_MASTER);
    Pinmux_Config(SPI0_CS, SPI0_SS_N_0_MASTER);
    Pinmux_Config(UART_TX, UART0_TX);
    Pinmux_Config(UART_RX, UART0_RX);
    return;
}
/**
 * @brief    pad configuration
 * @return   void
 */
void PAD_Configuration(void)
{

    Pad_Config(SPI0_SCK, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MOSI, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MISO, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_CS, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(UART_TX, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(UART_RX, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    return;
}

/**
 * @brief    rcc configuration
 * @return   void
 */
void RCC_Configuration(void)
{
    /* Enable  clock */
    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);
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
  * @brief  Initialize SPI peripheral.
  * @param   No parameter.
  * @return  void
  */
void SPI_InitConfiguration(void)
{
    SPI_InitTypeDef  SPI_InitStructure;

    SPI_StructInit(&SPI_InitStructure);
    SPI_InitStructure.SPI_Direction   = SPI_Direction_EEPROM;
    SPI_InitStructure.SPI_Mode        = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_BaudRatePrescaler  = 100;//SPI Clock Speed = clk source/SPI_ClkDIV
    SPI_InitStructure.SPI_FrameFormat = SPI_Frame_Motorola;
    SPI_InitStructure.SPI_NDF         = 9;
    SPI_Init(SPI0, &SPI_InitStructure);

    SPI_Cmd(SPI0, ENABLE);
}
void DataUART_Init(void)
{
    /* Initialize UART */
    UART_InitTypeDef uartInitStruct;
    UART_StructInit(&uartInitStruct);
    uartInitStruct.dmaEn = UART_DMA_ENABLE;
    uartInitStruct.TxDmaEn = UART_DMA_ENABLE;
    /* Baudrate = 9600 */
    uartInitStruct.div = 271;
    uartInitStruct.ovsr = 10;
    uartInitStruct.ovsr_adj = 0x24A;
    UART_Init(UART, &uartInitStruct);
}
void GDMA_Configuration(void)
{
    GDMA_InitTypeDef GDMA_InitStruct;
    /*---------------------GDMA initial------------------------------*/
    GDMA_InitStruct.GDMA_ChannelNum          = 1;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_PeripheralToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = 10;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;

    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;

    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)SPI0->DR;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(&(UART->RB_THR));
    GDMA_InitStruct.GDMA_SourceHandshake     = GDMA_Handshake_SPI0_RX;
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_UART0_TX;
    GDMA_Init(GDMA_Channel1, &GDMA_InitStruct);

    /*-----------------GDMA IRQ-----------------------------*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GDMA0_Channel1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /* Enable transfer interrupt */
    GDMA_INTConfig(1, GDMA_INT_Transfer, ENABLE);


}
/**
  * @brief   SPI Read PSRAM ID.
  * @param   No parameter.
  * @return  void
  */
void SPI_ReadID(void)
{
    uint8_t sendBuf[16] = {0};
    uint32_t address = 0;

    /* Configure read command and address */
    sendBuf[0] = 0x9f;
    sendBuf[1] = (address >> 16) & 0xff;
    sendBuf[2] = (address >> 8) & 0xff;
    sendBuf[3] = address & 0xff;
    SPI_SendBuffer(SPI0, sendBuf, 4);

    /*Waiting for SPI data transfer to end*/
    while (SPI_GetFlagState(SPI0, SPI_FLAG_BUSY));
}

int main(void)
{
    WDG_Disable();
    __enable_irq();
    board_init();
    SPI_InitConfiguration();
    DataUART_Init();
    GDMA_Configuration();
    SPI_ReadID();
    while (!SPI_GetFlagState(SPI0, SPI_FLAG_RFNE));
    SPI_GDMACmd(SPI0, SPI_GDMAReq_Rx, ENABLE);
    GDMA_Cmd(1, ENABLE);
    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
    }

}
void GDMA0_Channel1_Handler(void)
{
    GDMA_ClearAllTypeINT(1);
}
