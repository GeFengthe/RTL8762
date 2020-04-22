/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     spi_flash.c
* @brief    This file provides demo code of adc continuous mode.
* @details
* @author   yuan
* @date     2019-07-03
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "spi_flash.h"
#include "board.h"


/* Globals ------------------------------------------------------------------*/
uint8_t Flash_Data[10];
uint8_t Flash_ID_Length = 0;

/**
  * @brief  SPI write and read function.
  * @param  pWriteBuf: write data buf point.
  * @param  vWriteLen: write data length.
  * @param  pReadBuf: write data buf point.
  * @param  vReadLen: write data length.
  * @return void
  */
void spi_flash_write_read(uint8_t *pWriteBuf, uint16_t vWriteLen, uint8_t *pReadBuf,
                          uint16_t vReadLen)
{
    SPI_SetReadLen(FLASH_SPI, vReadLen);
    SPI_SendBuffer(FLASH_SPI, pWriteBuf, vWriteLen);
}

/**
  * @brief  Read spi flash(fm25q16a) id.
  * @param  Flash_ID_Type.
  * @return void
  */
void spi_flash_read_id(Flash_ID_Type vIdType, uint8_t *pId)
{
    uint8_t write_data = 0;
    uint16_t write_length = 1;

    switch (vIdType)
    {
    case DEVICE_ID:
        write_data = SPI_FLASH_DEVICE_ID;
        Flash_ID_Length = 4;
        break;
    case MF_DEVICE_ID:
        write_data = SPI_FLASH_MANU_ID;
        Flash_ID_Length = 5;
        break;
    case JEDEC_ID:
        write_data = SPI_FLASH_JEDEC_ID;
        Flash_ID_Length = 3;
        break;
    default:
        return;
    }
    spi_flash_write_read(&write_data, write_length, &pId[1], Flash_ID_Length);
    pId[0] = Flash_ID_Length;
}
