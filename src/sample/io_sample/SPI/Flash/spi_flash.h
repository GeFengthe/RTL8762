/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     spi_flash.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_spi.h"

/* Defines ------------------------------------------------------------------*/
#define SPI_FLASH_PAGE_SIZE             256

#define SPI_FLASH_WRITE_ENABLE          0x06
#define SPI_FLASH_WRITE_DISABLE         0x04
#define SPI_FLASH_READ_STATUS_REG_1     0x05
#define SPI_FLASH_WRITE_STATUS_REG_1    0x01
#define SPI_FLASH_READ_STATUS_REG_2     0x35
#define SPI_FLASH_WRITE_STATUS_REG_2    0x31
#define SPI_FLASH_PAGE_PROGRAM          0x02
#define SPI_FLASH_SECTOR_ERASE          0x20
#define SPI_FLASH_BLOCK_ERASE_32K       0x52
#define SPI_FLASH_BLOCK_ERASE_64K       0xD8
#define SPI_FLASH_CHIP_ERASE            0xC7
#define SPI_FLASH_POWER_DOWN            0xB9
#define SPI_FLASH_READ_DATA             0x03
#define SPI_FLASH_FAST_READ             0x0B
#define SPI_FLASH_RELEASE_POWER_DOWN    0xAB
#define SPI_FLASH_DEVICE_ID             0xAB
#define SPI_FLASH_MANU_ID               0x90
#define SPI_FLASH_JEDEC_ID              0x9F
#define SPI_FLASH_ENABLE_RESET          0x66
#define SPI_FLASH_RESET                 0x99

#define EWIP_FLAG                       0x01  /* Erase/Write in progress (WIP) flag */


void spi_flash_read_id(uint8_t *pFlashId);
void spi_flash_busy_check(void);
void spi_flash_write_enable(void);
void spi_flash_status_reg_write(uint8_t vStatus);
void spi_flash_sector_erase(uint32_t vAddress);
void spi_flash_block_erase(uint32_t vAddress);
void spi_flash_chip_erase(void);
void spi_flash_page_write(uint32_t vWriteAddr, uint8_t *pBuffer, uint16_t vLength);
void spi_flash_buffer_write(uint32_t vWriteAddr, uint8_t *pBuffer, uint16_t vLength);
void spi_flash_read(uint8_t vReadCmd, uint32_t vReadAddr, uint8_t *pBuffer, uint16_t vLength);


#ifdef __cplusplus
}
#endif

#endif

