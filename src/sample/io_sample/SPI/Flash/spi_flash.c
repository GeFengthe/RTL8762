/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of spi write and ead data from GD25Q16 flash.
* @details
* @author   yuan
* @date     2019-07-10
* @version  v1.0
*********************************************************************************************************
*/
#include "spi_flash.h"
#include "io_spi.h"
#include "trace.h"
#include <string.h>


void spi_flash_read_id(uint8_t *pFlashId)
{
    uint8_t send_buf[4] = {SPI_FLASH_JEDEC_ID, 0, 0, 0};
    uint8_t recv_buf[10] = {0};
    uint8_t recv_len = 3;

#if (SPI_MODE_EEPROM == SPI_MODE)
    SPI_SetReadLen(FLASH_SPI, 3);
    SPI_SendBuffer(FLASH_SPI, send_buf, 1);
#elif (SPI_MODE_FULLDUPLEX == SPI_MODE)
    SPI_SendBuffer(FLASH_SPI, send_buf, 4);
    recv_len += 1;
#endif

    uint8_t idx = 0;
    while (recv_len--)
    {
        while (RESET == SPI_GetFlagState(FLASH_SPI, SPI_FLAG_RFNE));
        recv_buf[idx++] = SPI_ReceiveData(FLASH_SPI);
    }

#if (SPI_MODE_EEPROM == SPI_MODE)
    memcpy(pFlashId, recv_buf, 3);
#elif (SPI_MODE_FULLDUPLEX == SPI_MODE)
    memcpy(pFlashId, &recv_buf[1], 3);
#endif
}

void spi_flash_busy_check(void)
{
    uint8_t send_buf[2] = {SPI_FLASH_READ_STATUS_REG_1, 0};
    uint8_t recv_len = 0;
    uint8_t recv_data[2] = {0};

    while (1)
    {
        SPI_SendBuffer(FLASH_SPI, send_buf, 2);
        while (SPI_GetFlagState(FLASH_SPI, SPI_FLAG_BUSY) == SET);
        recv_len = SPI_GetRxFIFOLen(FLASH_SPI);
        uint8_t i = 0;
        while (recv_len--)
        {
            recv_data[i++] = SPI_ReceiveData(FLASH_SPI);
        }
        i = 0;
        if ((recv_data[1] & 0x01) == 0)
        {
            break;
        }
    }
}

void spi_flash_write_enable(void)
{
    uint8_t len = 0;

    SPI_SendData(FLASH_SPI, SPI_FLASH_WRITE_ENABLE);
    while (SPI_GetFlagState(FLASH_SPI, SPI_FLAG_BUSY) == SET);
    len = SPI_GetRxFIFOLen(FLASH_SPI);
    while (len--)
    {
        SPI_ReceiveData(FLASH_SPI);
    }
}

void spi_flash_status_reg_write(uint8_t vStatus)
{
    uint8_t len = 0;
    uint8_t send_buf[2] = {SPI_FLASH_WRITE_STATUS_REG_1, 0};

    send_buf[1] = vStatus & 0xff;

    /* Enable write */
    spi_flash_write_enable();

    /* Write status register */
    SPI_SendBuffer(FLASH_SPI, send_buf, 2);

    while (SPI_GetFlagState(FLASH_SPI, SPI_FLAG_BUSY) == SET);
    len = SPI_GetRxFIFOLen(FLASH_SPI);
    while (len--)
    {
        SPI_ReceiveData(FLASH_SPI);
    }
    spi_flash_busy_check();
}

void spi_flash_sector_erase(uint32_t vAddress)
{
    uint8_t send_buf[4] = {SPI_FLASH_SECTOR_ERASE, 0, 0, 0};
    uint8_t recv_len = 0;

    send_buf[1] = (vAddress >> 16) & 0xff;
    send_buf[2] = (vAddress >> 8) & 0xff;
    send_buf[3] = vAddress & 0xff;

    /* enable write */
    spi_flash_write_enable();

    /* erase data */
    SPI_SendBuffer(FLASH_SPI, send_buf, 4);

    while (SPI_GetFlagState(FLASH_SPI, SPI_FLAG_BUSY) == SET);
    recv_len = SPI_GetRxFIFOLen(FLASH_SPI);
    while (recv_len--)
    {
        SPI_ReceiveData(FLASH_SPI);
    }
    spi_flash_busy_check();
}

void spi_flash_block_erase(uint32_t vAddress)
{
    uint8_t send_buf[4] = {SPI_FLASH_BLOCK_ERASE_32K, 0, 0, 0};
    uint8_t recv_len = 0;

    send_buf[1] = (vAddress >> 16) & 0xff;
    send_buf[2] = (vAddress >> 8) & 0xff;
    send_buf[3] = vAddress & 0xff;

    /* Enable write */
    spi_flash_write_enable();
    /* erase data */
    SPI_SendBuffer(FLASH_SPI, send_buf, 4);

    while (SPI_GetFlagState(FLASH_SPI, SPI_FLAG_BUSY) == SET);
    recv_len = SPI_GetRxFIFOLen(FLASH_SPI);
    while (recv_len--)
    {
        SPI_ReceiveData(FLASH_SPI);
    }
    spi_flash_busy_check();
}

void spi_flash_chip_erase(void)
{
    uint8_t recv_len = 0;

    /* Enable write */
    spi_flash_write_enable();

    SPI_SendData(FLASH_SPI, SPI_FLASH_CHIP_ERASE);

    while (SPI_GetFlagState(FLASH_SPI, SPI_FLAG_BUSY) == SET);
    recv_len = SPI_GetRxFIFOLen(FLASH_SPI);
    while (recv_len--)
    {
        SPI_ReceiveData(FLASH_SPI);
    }
    spi_flash_busy_check();
}

void spi_flash_page_write(uint32_t vWriteAddr, uint8_t *pBuffer, uint16_t vLength)
{
    uint16_t send_data_len = vLength;
    uint8_t recv_data_len = 0;
    uint8_t send_buf[4] = {SPI_FLASH_PAGE_PROGRAM, 0, 0, 0};
    send_buf[1] = (vWriteAddr >> 16) & 0xff;
    send_buf[2] = (vWriteAddr >> 8) & 0xff;
    send_buf[3] = vWriteAddr & 0xff;

    if (vLength > SPI_FLASH_PAGE_SIZE)
    {
        send_data_len = SPI_FLASH_PAGE_SIZE;
    }

    /* Enable write */
    spi_flash_write_enable();
    SPI_SendBuffer(FLASH_SPI, send_buf, 4);
    SPI_SendBuffer(FLASH_SPI, pBuffer, send_data_len);

    while (SPI_GetFlagState(FLASH_SPI, SPI_FLAG_BUSY) == SET);
    recv_data_len = SPI_GetRxFIFOLen(FLASH_SPI);
    while (recv_data_len--)
    {
        SPI_ReceiveData(FLASH_SPI);
    }
    spi_flash_busy_check();
}

void spi_flash_buffer_write(uint32_t vWriteAddr, uint8_t *pBuffer, uint16_t vLength)
{
    uint32_t write_addr = vWriteAddr;
    uint8_t *p_write_data = pBuffer;
    uint16_t write_data_len = vLength;

    uint8_t offset = vWriteAddr % SPI_FLASH_PAGE_SIZE;
    uint8_t free_len = SPI_FLASH_PAGE_SIZE - offset;
    uint8_t page_num = vLength / SPI_FLASH_PAGE_SIZE;
    uint8_t remain_len = vLength % SPI_FLASH_PAGE_SIZE;

    if (offset == 0)
    {
        if (page_num == 0) // write_data_len < SPI_FLASH_PAGE_SIZE
        {
            spi_flash_page_write(write_addr, p_write_data, remain_len);
        }
        else // write_data_len > SPI_FLASH_PAGE_SIZE
        {
            while (page_num--)
            {
                spi_flash_page_write(write_addr, p_write_data, SPI_FLASH_PAGE_SIZE);
                write_addr +=  SPI_FLASH_PAGE_SIZE;
                p_write_data += SPI_FLASH_PAGE_SIZE;
            }
            spi_flash_page_write(write_addr, p_write_data, remain_len);
        }
    }
    else // write_addr is not SPI_FLASH_PAGE_SIZE aligned
    {
        if (page_num == 0)
        {
            if (remain_len > free_len) /* (write_data_len + write_addr) > SPI_FLASH_PAGE_SIZE */
            {
                uint8_t temp = remain_len - free_len;
                spi_flash_page_write(write_addr, p_write_data, free_len);
                write_addr +=  free_len;
                p_write_data += free_len;
                spi_flash_page_write(vWriteAddr, p_write_data, temp);
            }
            else
            {
                spi_flash_page_write(write_addr, p_write_data, write_data_len);
            }

        }
        else /* NumByteToWrite > SPI_FLASH_PAGE_SIZE */
        {
            write_data_len -= free_len;
            page_num =  write_data_len / SPI_FLASH_PAGE_SIZE;
            remain_len = write_data_len % SPI_FLASH_PAGE_SIZE;

            spi_flash_page_write(write_addr, p_write_data, free_len);
            write_addr +=  free_len;
            p_write_data += free_len;

            while (page_num--)
            {
                spi_flash_page_write(write_addr, p_write_data, SPI_FLASH_PAGE_SIZE);
                write_addr +=  SPI_FLASH_PAGE_SIZE;
                p_write_data += SPI_FLASH_PAGE_SIZE;
            }

            if (remain_len != 0)
            {
                spi_flash_page_write(write_addr, p_write_data, remain_len);
            }
        }
    }
}

void spi_flash_read(uint8_t vReadCmd, uint32_t vReadAddr, uint8_t *pBuffer, uint16_t vLength)
{
    uint8_t send_buf[10] = {0};
    uint8_t send_len = 0;
    uint16_t recv_len = 0;

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

    SPI_SendBuffer(FLASH_SPI, send_buf, send_len);
    recv_len = vLength + send_len;
    while (recv_len--)
    {
        SPI_SendBuffer(FLASH_SPI, &send_buf[9], 1);
        while (SPI_GetFlagState(FLASH_SPI, SPI_FLAG_RFNE) == RESET);
        *pBuffer++ = SPI_ReceiveData(FLASH_SPI);
    }
    SPI_SetReadLen(FLASH_SPI, 1);
}
