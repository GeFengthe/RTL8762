/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     user_data.c
  * @brief    Source file for user data.
  * @details  Load and store user data to flash.
  * @author   hector
  * @date     2018-08-13
  * @version  v1.0
  * *************************************************************************************
  */

#include <string.h>
#include "user_data.h"
#include "flash_device.h"
#include "platform_diagnose.h"

/** flash sector size */
#define FLASH_SECTOR_SIZE    0x1000

#define ALI_DATA_FLAG                  "ALI"
#define ALI_DATA_FLAG_OFFSET           0
#define ALI_DATA_FLAG_SIZE             3
/** alibaba data macros */
#define ALI_PRODUCT_ID_OFFSET          4
#define ALI_PRODUCT_ID_SIZE            4

#define ALI_SECRET_KEY_OFFSET          8
#define ALI_SECRET_KEY_SIZE            32

bool user_data_contains_ali_data(void)
{
    uint8_t flag[3];
    if (!flash_read_locked(FLASH_ALI_DATA_ADDRESS + ALI_DATA_FLAG_OFFSET, ALI_DATA_FLAG_SIZE, flag))
    {
        return FALSE;
    }

    return (0 == memcmp(flag, ALI_DATA_FLAG, 3));
}

uint32_t user_data_read_ali_product_id(void)
{
    uint32_t id = 0;
    if (!flash_read_locked(FLASH_ALI_DATA_ADDRESS + ALI_PRODUCT_ID_OFFSET, ALI_PRODUCT_ID_SIZE,
                           (uint8_t *)&id))
    {
        return 0;
    }

    return id;
}

bool user_data_read_ali_secret_key(uint8_t *key)
{
    if (!flash_read_locked(FLASH_ALI_DATA_ADDRESS + ALI_SECRET_KEY_OFFSET, ALI_SECRET_KEY_SIZE, key))
    {
        return FALSE;
    }
    return TRUE;
}

bool user_data_write_ali_data(uint32_t id, const uint8_t *key)
{
    /** just erase and write, do not consider contains other user data */
    uint32_t sector_addr = FLASH_ALI_DATA_ADDRESS;
    sector_addr &= ~(FLASH_SECTOR_SIZE - 1);
    /** erase block */
    if (!flash_erase_locked(FLASH_ERASE_SECTOR, sector_addr))
    {
        return FALSE;
    }

    /** write flag */
    uint8_t ali_flag[] = ALI_DATA_FLAG; /** for avoid warning */
    if (!flash_write_locked(FLASH_ALI_DATA_ADDRESS + ALI_DATA_FLAG_OFFSET, ALI_DATA_FLAG_SIZE,
                            ali_flag))
    {
        return FALSE;
    }

    /** write data */
    if (!flash_write_locked(FLASH_ALI_DATA_ADDRESS + ALI_PRODUCT_ID_OFFSET, ALI_PRODUCT_ID_SIZE,
                            (uint8_t *)&id))
    {
        return FALSE;
    }

    if (!flash_write_locked(FLASH_ALI_DATA_ADDRESS + ALI_SECRET_KEY_OFFSET, ALI_SECRET_KEY_SIZE,
                            (uint8_t *)key))
    {
        return FALSE;
    }

    return TRUE;
}


