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

/* Globals ------------------------------------------------------------------*/
uint8_t flash_id_type = 0;


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
void driver_spi_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);

    SPI_InitTypeDef  SPI_InitStruct;
    SPI_StructInit(&SPI_InitStruct);

    SPI_InitStruct.SPI_Direction   = SPI_Direction_EEPROM;
    SPI_InitStruct.SPI_Mode        = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStruct.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_InitStruct.SPI_BaudRatePrescaler  = 16;
    /* SPI_Direction_EEPROM mode read data lenth. */
    SPI_InitStruct.SPI_RxThresholdLevel  = 1;/* Flash id lenth = 3*/
    SPI_InitStruct.SPI_NDF               = 0;/* Flash id lenth = 3*/
    /* cause SPI_INT_RXF interrupt if data length in receive FIFO  >= SPI_RxThresholdLevel + 1*/
    SPI_InitStruct.SPI_FrameFormat = SPI_Frame_Motorola;

    SPI_Init(FLASH_SPI, &SPI_InitStruct);
    SPI_Cmd(FLASH_SPI, ENABLE);
}

/**
  * @brief  Demo code of operation about spi.
  * @param  No parameter.
  * @return void
*/
void spi_demo(void)
{
    uint8_t flash_id[10] = {0};
    while (flash_id_type < 3)
    {
        spi_flash_read_id((Flash_ID_Type)flash_id_type, flash_id);
        APP_PRINT_INFO3("[io_spi] spi_demo: flash_id_type = %d, data_lenth = %d, flash_id = %b ",
                        flash_id_type, flash_id[0], TRACE_BINARY(flash_id[0], &flash_id[1]));
        flash_id_type++;
        memset(flash_id, 0, sizeof(flash_id));
    }
    flash_id_type = 0;

    uint8_t read_data[100] = {0};
    spi_flash_read(SPI_FLASH_READ_DATA, 0x000101, read_data, 100);
    APP_PRINT_INFO1("[io_spi] spi_demo: read_data = %b,", TRACE_BINARY(100, read_data));
    spi_flash_read(SPI_FLASH_FAST_READ, 0x000210, read_data, 100);
    APP_PRINT_INFO1("[io_spi] spi_demo: read_data = %b,", TRACE_BINARY(100, read_data));
}

