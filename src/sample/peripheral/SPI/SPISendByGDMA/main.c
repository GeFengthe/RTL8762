/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of SPI Write PSRAM by GDMA.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_spi.h"
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"
#include "rtl876x_wdg.h"
#include <trace.h>
#include <os_sched.h>
#include "board.h"

/* GDAM parameters configuration */
#define USER_GDMA_INDEX               2
#define USER_GDMA_CHANNEL             GDMA_Channel2
#define USER_GDMA_CHANNEL_IRQ       GDMA0_Channel2_IRQn
#define USERGDMAIntrHandler         GDMA0_Channel2_Handler

volatile uint8_t GDMA_TxBuffer[2560];



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
    return;
}

/**
 * @brief    rcc configuration
 * @return   void
 */
void RCC_Configuration(void)
{
    /* Enable SPI clock */
    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);
    /* turn on GDMA clock */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
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
    SPI_InitStructure.SPI_Direction   = SPI_Direction_TxOnly;
    SPI_InitStructure.SPI_Mode        = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_Init(SPI0, &SPI_InitStructure);

    SPI_Cmd(SPI0, ENABLE);
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
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(SPI0->DR);
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_SPI0_TX;
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

void TXDataByGDMA(uint8_t *pSendBuf, uint32_t flash_address, uint16_t len)
{
    /* Configure read command and address */
    pSendBuf[0] = 0x02;
    pSendBuf[1] = (flash_address >> 16) & 0xff;
    pSendBuf[2] = (flash_address >> 8) & 0xff;
    pSendBuf[3] = flash_address & 0xff;
    /* Enable SPI_GDMAReq_Tx */
    SPI_GDMACmd(SPI0, SPI_GDMAReq_Tx, ENABLE);
    /* Switch SPI to EEPROM mode */
    SPI_ChangeDirection(SPI0, SPI_Direction_TxOnly);
    /* Configure GDMA transmission size */
    GDMA_SetBufferSize(USER_GDMA_CHANNEL, len);
    /* Configure GDMA destination address */
    GDMA_SetSourceAddress(USER_GDMA_CHANNEL, (uint32_t)pSendBuf);
    /* Enable GDMA to transmission */
    GDMA_Cmd(USER_GDMA_INDEX, ENABLE);
}
int main(void)
{
    uint16_t i = 0;
    WDG_Disable();
    __enable_irq();
    board_init();
    SPI_InitConfiguration();
    GDMA_TXConfiguration();
    for (i = 4; i < 256; i++)
    {
        GDMA_TxBuffer[i] = i;
    }
    TXDataByGDMA((uint8_t *)GDMA_TxBuffer, 0, 256);
    while (SPI_GetFlagState(SPI0, SPI_FLAG_BUSY));
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
    GDMA_Cmd(USER_GDMA_INDEX, ENABLE);
}
