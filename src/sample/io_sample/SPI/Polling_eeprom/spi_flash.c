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

/**
  * @brief  Read spi flash(fm25q16a) id.
  * @param  Flash_ID_Type.
  * @return void
  */
void spi_flash_read_id(Flash_ID_Type vFlashIdType, uint8_t *pFlashId)
{
    uint8_t send_buf[4] = {SPI_FLASH_JEDEC_ID, 0, 0, 0};
    uint8_t recv_len = 3;

    switch (vFlashIdType)
    {
    case DEVICE_ID:
        send_buf[0] = SPI_FLASH_DEVICE_ID;
        recv_len = 4;
        break;
    case MF_DEVICE_ID:
        send_buf[0] = SPI_FLASH_MANU_ID;
        recv_len = 5;
        break;
    case JEDEC_ID:
        send_buf[0] = SPI_FLASH_JEDEC_ID;
        recv_len = 3;
        break;
    default:
        return;
    }
    *pFlashId++ = recv_len;

    SPI_SetReadLen(FLASH_SPI, recv_len);
    SPI_SendBuffer(FLASH_SPI, send_buf, 1);
    while (recv_len--)
    {
        while (RESET == SPI_GetFlagState(FLASH_SPI, SPI_FLAG_RFNE));
        *pFlashId++ = SPI_ReceiveData(FLASH_SPI);
    }
}

void spi_flash_read(uint8_t vReadCmd, uint32_t vReadAddr, uint8_t *pBuffer, uint16_t vLength)
{
    uint8_t send_buf[10] = {0};
    uint8_t send_len = 0;
    uint16_t recv_len = vLength;

    if (SPI_FLASH_READ_DATA == vReadCmd)
    {
        send_len = 4;
    }
    else if (SPI_FLASH_FAST_READ == vReadCmd)
    {
        send_len = 5;
    }
    send_buf[0] = vReadCmd;
    send_buf[1] = (vReadAddr >> 16) & 0xFF;
    send_buf[2] = (vReadAddr >> 8) & 0xFF;
    send_buf[3] = (vReadAddr) & 0xFF;

    SPI_SetReadLen(FLASH_SPI, recv_len);
    SPI_SendBuffer(FLASH_SPI, send_buf, send_len);

    while (recv_len--)
    {
        while (SPI_GetFlagState(FLASH_SPI, SPI_FLAG_RFNE) == RESET);
        *pBuffer++ = SPI_ReceiveData(FLASH_SPI);
    }
    SPI_SetReadLen(FLASH_SPI, 1);
}
