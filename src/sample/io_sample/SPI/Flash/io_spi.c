/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_spi.c
* @brief    This file provides demo code of spi master.
            FM25Q16A is slave.
            Read the chip id of FM25Q16A.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "io_spi.h"

#include "spi_flash.h"
#include "app_task.h"


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

    Pinmux_Deinit(P4_0);
    Pinmux_Deinit(P4_1);
    Pinmux_Deinit(P4_2);
    Pinmux_Deinit(P4_3);

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
//void SPI_InitConfiguration(void)
//{
//    SPI_InitTypeDef  SPI_InitStructure;

//    SPI_StructInit(&SPI_InitStructure);
//    SPI_InitStructure.SPI_Direction   = SPI_Direction_EEPROM;
//    SPI_InitStructure.SPI_Mode        = SPI_Mode_Master;
//    SPI_InitStructure.SPI_DataSize    = SPI_DataSize_8b;
//    SPI_InitStructure.SPI_CPOL        = SPI_CPOL_High;
//    SPI_InitStructure.SPI_CPHA        = SPI_CPHA_2Edge;
//    SPI_InitStructure.SPI_BaudRatePrescaler  = 16;
//    SPI_InitStructure.SPI_RxThresholdLevel  = 8;
//    SPI_InitStructure.SPI_FrameFormat = SPI_Frame_Motorola;
//    SPI_InitStructure.SPI_NDF         = 0;
//    SPI_Init(SPI0, &SPI_InitStructure);

//    SPI_Cmd(SPI0, ENABLE);
//}

void driver_spi_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);

    SPI_InitTypeDef  SPI_InitStruct;
    SPI_StructInit(&SPI_InitStruct);

    SPI_InitStruct.SPI_Direction   = SPI_Direction_FullDuplex;
    SPI_InitStruct.SPI_Mode        = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStruct.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_InitStruct.SPI_BaudRatePrescaler  = 8;
    /* SPI_Direction_EEPROM mode read data lenth. */
    SPI_InitStruct.SPI_RxThresholdLevel  = 1;/* Flash id lenth = 3*/
    SPI_InitStruct.SPI_NDF               = 0;/* Flash id lenth = 3*/
    SPI_InitStruct.SPI_FrameFormat = SPI_Frame_Motorola;
    SPI_Init(SPI0, &SPI_InitStruct);

    SPI_Cmd(SPI0, ENABLE);

}

/**
  * @brief  Demo code of operation about spi.
  * @param  No parameter.
  * @return void
*/
void spi_demo(void)
{
    uint8_t flash_id[10] = {0};
    uint8_t write_data[100];
    uint8_t read_data[105] = {0};
    for (uint16_t i = 0; i < 100; i++)
    {
        write_data[i] = i & 0xFF;
    }

    spi_flash_read_id(flash_id);
    APP_PRINT_INFO1("[io_spi] spi_demo: flash_id = %b ", TRACE_BINARY(3, flash_id));

    spi_flash_sector_erase(0x001000);
    APP_PRINT_INFO0("[io_spi] spi_demo: spi_flash_sector_erase done");

    spi_flash_read(SPI_FLASH_FAST_READ, 0x001000, read_data, 100);
    APP_PRINT_INFO1("[io_spi] spi_demo: read_data = %b,", TRACE_BINARY(100, &read_data[5]));

    spi_flash_page_write(0x001000, write_data, 100);

    spi_flash_read(SPI_FLASH_FAST_READ, 0x001000, read_data, 100);
    APP_PRINT_INFO1("[io_spi] spi_demo: read_data = %b,", TRACE_BINARY(100, &read_data[5]));
}


