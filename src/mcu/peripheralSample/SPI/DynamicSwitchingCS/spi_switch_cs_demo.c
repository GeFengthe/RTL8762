/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     spi_switch_cs_demo.c
* @brief    This file provides demo code of spi comunication when dynamic switching cs signal.
            Attention: Only SPI1 have this function.
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
#define SPI1_SCK_PIN                P3_2
#define SPI1_MOSI_PIN               P1_2
#define SPI1_MISO_PIN               P1_3
#define SPI1_CS0_PIN                P1_4
#define SPI1_CS1_PIN                P2_4
#define SPI1_CS2_PIN                P2_5

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_spi_init(void)
{
    Pad_Config(SPI1_SCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI1_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI1_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI1_CS0_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI1_CS1_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI1_CS2_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Config(SPI1_SCK_PIN, SPI1_CLK_MASTER);
    Pinmux_Config(SPI1_MOSI_PIN, SPI1_MO_MASTER);
    Pinmux_Config(SPI1_MISO_PIN, SPI1_MI_MASTER);
    Pinmux_Config(SPI1_CS0_PIN, SPI1_SS_N_0_MASTER);
    Pinmux_Config(SPI1_CS1_PIN, SPI1_SS_N_1_MASTER);
    Pinmux_Config(SPI1_CS2_PIN, SPI1_SS_N_2_MASTER);
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

    RCC_PeriphClockCmd(APBPeriph_SPI1, APBPeriph_SPI1_CLOCK, ENABLE);

    SPI_InitStructure.SPI_Direction   = SPI_Direction_FullDuplex;
    SPI_InitStructure.SPI_Mode        = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_BaudRatePrescaler  = 100;

    SPI_InitStructure.SPI_RxThresholdLevel  =
        0;/* cause SPI_INT_RXF interrupt if data length in receive FIFO  >= SPI_RxThresholdLevel + 1*/
    SPI_InitStructure.SPI_FrameFormat = SPI_Frame_Motorola;

    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);

}


/**
  * @brief  Demo code of dlps+uart.
  * @param  No parameter.
  * @return void
*/
void spi_demo(void)
{
    uint8_t spiWriteBuf[16] = {0, 0x01, 0x02, 0x00};

    board_spi_init();
    driver_spi_init();

    SPI_SendBuffer(SPI1, spiWriteBuf, 3);
    /* Wait Tx FIFO empty */
    while (SPI_GetFlagState(SPI1, SPI_FLAG_TFE) == RESET);
    while (SPI_GetFlagState(SPI1, SPI_FLAG_BUSY));

#if 1
    /* switch to CS1 signal */
    SPI1->SER = BIT(1);
#else
    /* switch to CS1 signal */
    SPI_SetCSNumber(SPI1, 1);
#endif

    SPI_SendBuffer(SPI1, spiWriteBuf, 3);
    /* Wait Tx FIFO empty */
    while (SPI_GetFlagState(SPI1, SPI_FLAG_TFE) == RESET);
    while (SPI_GetFlagState(SPI1, SPI_FLAG_BUSY));

#if 1
    /* switch to CS2 signal */
    SPI1->SER = BIT(2);
#else
    /* switch to CS2 signal */
    SPI_SetCSNumber(SPI1, 2);
#endif

    SPI_SendBuffer(SPI1, spiWriteBuf, 3);
    /* Wait Tx FIFO empty */
    while (SPI_GetFlagState(SPI1, SPI_FLAG_TFE) == RESET);
    while (SPI_GetFlagState(SPI1, SPI_FLAG_BUSY));
}


