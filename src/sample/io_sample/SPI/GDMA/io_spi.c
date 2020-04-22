/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_spi.c
* @brief    This file provides demo code of spi master.
            FM25Q16A is slave.
            Read data from FM25Q16A by GDMA.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "io_spi.h"
#include "io_gdma.h"



/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_spi_init(void)
{
    Pad_Config(SPI_SCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI_CS_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

//    Pinmux_Deinit(P4_0);
//    Pinmux_Deinit(P4_1);
//    Pinmux_Deinit(P4_2);
//    Pinmux_Deinit(P4_3);

    Pinmux_Config(SPI_SCK_PIN, SPI0_CLK_MASTER);
    Pinmux_Config(SPI_MOSI_PIN, SPI0_MO_MASTER);
    Pinmux_Config(SPI_MISO_PIN, SPI0_MI_MASTER);
    Pinmux_Config(SPI_CS_PIN, SPI0_SS_N_0_MASTER);
}

/**
  * @brief  Initialize spi peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_spi_init(void)
{
}

/**
  * @brief  Demo code of operation about spi.
  * @param  No parameter.
  * @return void
*/
void spi_demo(void)
{
    APP_PRINT_INFO0("[io_spi] spi_demo");
    driver_spi_gdma_init();
}

