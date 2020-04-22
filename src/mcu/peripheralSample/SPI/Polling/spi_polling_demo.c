/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     spi_interrupt_demo.c
* @brief    This file provides demo code of spi comunication.
* @details
* @author   yuan
* @date     2018-06-28
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
//#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_spi.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#define SPI0_SCK_PIN                P3_2
#define SPI0_MOSI_PIN               P1_2
#define SPI0_MISO_PIN               P1_3
#define SPI0_CS_PIN                 P1_4


/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_spi_init(void)
{
    Pad_Config(SPI0_SCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_CS_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Config(SPI0_SCK_PIN, SPI0_CLK_MASTER);
    Pinmux_Config(SPI0_MOSI_PIN, SPI0_MO_MASTER);
    Pinmux_Config(SPI0_MISO_PIN, SPI0_MI_MASTER);
    Pinmux_Config(SPI0_CS_PIN, SPI0_SS_N_0_MASTER);
}

/**
  * @brief  Initialize spi peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_spi_init(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    SPI_StructInit(&SPI_InitStructure);

    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);

    SPI_InitStructure.SPI_Direction   = SPI_Direction_FullDuplex;
    SPI_InitStructure.SPI_Mode        = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA        = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_BaudRatePrescaler  = 100;

    SPI_InitStructure.SPI_RxThresholdLevel  =
        0;/* cause SPI_INT_RXF interrupt if data length in receive FIFO  >= SPI_RxThresholdLevel + 1*/
    SPI_InitStructure.SPI_FrameFormat = SPI_Frame_Motorola;

    SPI_Init(SPI0, &SPI_InitStructure);
    SPI_Cmd(SPI0, ENABLE);

}

/**
  * @brief  Demo code of dlps+uart.
  * @param  No parameter.
  * @return void
*/
void spi_demo(void)
{
    uint8_t spiWriteBuf[16] = {0, 0x01, 0x02, 0x00};
    uint8_t spiReadBuf[16] = {0, 0, 0, 0};
    uint8_t idx = 0;
    uint8_t len = 0;

    board_spi_init();
    driver_spi_init();

    /*---------------Read flash ID--------------*/
    spiWriteBuf[0] = 0x9f;
    SPI_SendBuffer(SPI0, spiWriteBuf, 4);

    /* Waiting for SPI data transfer to end */
    while (SPI_GetFlagState(SPI0, SPI_FLAG_BUSY));

    /*Read ID number of flash GD25Q20*/
    len = SPI_GetRxFIFOLen(SPI0);
    for (idx = 0; idx < len; idx++)
    {
        spiReadBuf[idx] = SPI_ReceiveData(SPI0);
        //DBG_BUFFER(MODULE_APP, LEVEL_INFO, "SPI_ReadBuf[%d] = 0x%x", 2, idx, SPI_ReadBuf[idx]);
        DBG_DIRECT("SPI_ReadBuf[%d] = 0x%X", idx, spiReadBuf[idx]);
    }

}



