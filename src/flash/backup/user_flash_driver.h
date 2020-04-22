/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file    user_flash_driver.h
  * @author
  * @version V0.0.1
  * @date    2018-06-25
  * @brief   This file contains all the functions prototypes for the spi flash
  *          library.
  ******************************************************************************
  * @attention
  *
  * This module is a confidential and proprietary property of RealTek and
  * possession or use of this module requires written permission of RealTek.
  *
  * Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
  ******************************************************************************
  */

/*****************************************************************************************
* Introduction
*****************************************************************************************
* - This module implements the spic & flash functionality.
* - Since different spi flash would define different op code for the same commands,
* - so this driver will mitigate this situation and provide the same APIs.
******************************************************************************************/



#ifndef _USER_FLASH_DRIVER_H
#define _USER_FLASH_DRIVER_H
#include <stdint.h>
#include <stdbool.h>
#include "rtl876x.h"
#include "app_section.h"
#include "flash_device.h"


/** @defgroup FLASH_DRIVER    Flash Driver
    * @brief Flash driver
    * @{
    */

/*****************************************************************************************
* Introduction
*****************************************************************************************
* - This module enables spi flash controller functionality for Bee2
* - The spi flash controller spec locates @
* - https://wiki.realtek.com/display/BTFWKNOW/BumbleBee3+Project
* - The design is based on Rel2.0 spec, and may subject to change without notification
* The SPIC is used to communicate with spi flash, and it supports
* - user mode and auto mode
*     User mode is typical software flow to implement all serial transfer, while auto mode will
*     leverage HW mechanism to access SPI flash as easy as access memory.
* - 1/2/4 channel data bit to TX/RX data
* - flexible command register to support different flash vendor.
******************************************************************************************/

/*============================================================================*
  *                                   Macros
  *============================================================================*/
/** @defgroup FLASH_DRIVER_Exported_Macros Flash Driver Exported Macros
    * @brief
    * @{
    */
#define NO_RECOMMAND_API

#define FLASH_SECTOR_SIZE       (4  * 1024)
#define FLASH_BLOCK_SIZE        (16 * FLASH_SECTOR_SIZE)
#define FLASH_DR_ADDR           (0x40080060)

#define FLASH_SIZE_MAX          (8 * 1024 * 1024)   /*!< depends on flash auto mode limit, size can't > 8MB. */
#define FLASH_ERASE_RETRY_MAX   10


/**magic value for flash calibration, use magic pattern in patch ext header*/
#define MAGIC_ADDR_OFFS  (FMC_PATCH_ADDR - FMC_MAIN + sizeof(T_IMAGE_HEADER))
//won't flash patch into fpga. TODO: need find some better place for calibration.


#define FLASH_VENDOR_MXIC           0XC2
#define FLASH_VENDOR_GD             0XC8
#define FLASH_VENDOR_MICRON         0X20
#define FLASH_VENDOR_WINBOND        0XEF
#define FLASH_VENDOR_ATMEL          0X1F
#define FLASH_VENDOR_SST            0XBF
#define FLASH_VENDOR_SPANSION       0X01
#define FLASH_VENDOR_PARAGON        0xE0
#define FLASH_VENDOR_FUDAN          0xA1
#define FLASH_VENDOR_ESMT           0x1C


/** End of FLASH_DRIVER_Exported_Macros
    * @}
    */

/*============================================================================*
  *                                   Types
  *============================================================================*/

///********************  Access mode of DR register in user mode  *******************/
typedef enum
{
    DATA_BYTE         = 0,
    DATA_HALF         = 1,
    DATA_WORD         = 2
} T_SPIC_BYTE_NUM;

/** @defgroup FLASH_DRIVER_Registers_Definitions Flash Driver Registers Definitions
  * @brief
  * @{
  */
/*****************************************************************************************
* Registers Definitions --------------------------------------------------------*
********************  Bits definition for SPIC_CTRLR0 register  *******************/
#define BIT_CK_MTIMES(x)        (((x) & 0x0000001F) << 23)
#define BIT_FAST_RD(x)          (((x) & 0x00000001) << 22)
#define BIT_CMD_CH(x)           (((x) & 0x00000003) << 20)
#define BIT_DATA_CH(x)          (((x) & 0x00000003) << 18)
#define BIT_ADDR_CH(x)          (((x) & 0x00000003) << 16)
#define BIT_TMOD(x)             (((x) & 0x00000003) << 8)
#define BIT_SCPOL               (0x00000001 << 7)
#define BIT_SCPH                (0x00000001 << 6)


/********************  Bits definition for SPIC_SR register  *******************/
#define BIT_TXE                 (0x00000001 << 5)
#define BIT_RFF                 (0x00000001 << 4)
#define BIT_RFNE                (0x00000001 << 3)
#define BIT_TFE                 (0x00000001 << 2)
#define BIT_TFNF                    (0x00000001 << 1)
#define BIT_BUSY                    (0x00000001)

#define BIT_STATUS_WIP  BIT(0)
#define BIT_STATUS_SUSPEND  BIT(7)
#define BIT_STATUS_WEL   BIT(1)
#define CMD_SECTOR_ERASE 0x20

/** End of FLASH_DRIVER_Registers_Definitions
  * @}
  */


/**
 * @brief set spic enable or disable.
 *
 * @param enable          DISABLE to disable cache, ENABLE to enable cache.
 * @return
 * @note: spic should be disabled before programming any register in user mode
*/
DATA_RAM_FUNCTION __STATIC_INLINE void spic_enable(uint32_t enable)
{
    SPIC->ssienr = enable;
}

/**
 * @brief clr addr channel and data channel in ctrl0 register.
 *
 * @return
*/
DATA_RAM_FUNCTION __STATIC_INLINE void spic_clr_multi_ch(void)
{
    SPIC->ctrlr0 &= ~(BIT_CMD_CH(3) | BIT_ADDR_CH(3) | BIT_DATA_CH(3));
}

/**
 * @brief set RX mode in ctrl0 register in user mode.
 *
 * @return
*/
DATA_RAM_FUNCTION __STATIC_INLINE void spic_set_rx_mode(void)
{
    SPIC->ctrlr0 |= BIT_TMOD(3);
}

/**
 * @brief set spic dr register to implement user mode programming
 *
 * @param byte_num    byte number for every tx.
 * @param data        data to be write into fifo
 * @return
 * @note: dr0 is used to do programming, and endian issue is considered as well.
*/
DATA_RAM_FUNCTION __STATIC_INLINE void spic_set_dr(T_SPIC_BYTE_NUM byte_num, uint32_t data)
{
    uint32_t wr_data;
    if (byte_num == DATA_HALF)
    {
        wr_data = rtk_cpu_to_le16(data);
        SPIC->dr[0].half = wr_data;
    }
    else if (byte_num == DATA_WORD)
    {
        wr_data = rtk_cpu_to_le32(data);
        SPIC->dr[0].word = wr_data;
    }
    else
    {
        SPIC->dr[0].byte = data;
    }
    return;
}

/**
 * @brief get spic dr register to implement user mode reading
 *
 * @param byte_num    byte number for every tx.
 * @return  data poping from FIFO
 * @note: dr0 is used to do reading, and endian issue is considered as well.
*/
DATA_RAM_FUNCTION __STATIC_INLINE uint32_t spic_get_dr(T_SPIC_BYTE_NUM byte_num)
{
    uint32_t rd_data = 0;
    if (byte_num == DATA_HALF)
    {
        rd_data = rtk_le16_to_cpu(SPIC->dr[0].half);
    }
    else if (byte_num == DATA_WORD)
    {
        rd_data = rtk_le32_to_cpu(SPIC->dr[0].word);
    }
    else
    {
        rd_data = SPIC->dr[0].byte;
    }
    return rd_data;
}



DATA_RAM_FUNCTION __STATIC_INLINE uint32_t flash_auto_read(uint32_t addr)
{
    return *(uint32_t *)addr;
}


/**
 * @brief set addr channel and data channel in ctrl0 register.
 *
 * @return
*/
DATA_RAM_FUNCTION __STATIC_INLINE void spic_set_multi_ch(uint32_t data_ch, uint32_t addr_ch)
{
    SPIC->ctrlr0 &= ~(BIT_CMD_CH(3) | BIT_ADDR_CH(3) | BIT_DATA_CH(3));

    SPIC->ctrlr0 |= (BIT_ADDR_CH(addr_ch) | BIT_DATA_CH(data_ch));
}


/**
 * @brief set TX mode in ctrl0 register in user mode.
 *
 * @return
*/
DATA_RAM_FUNCTION __STATIC_INLINE void spic_set_tx_mode(void)
{
    SPIC->ctrlr0 &= ~(BIT_TMOD(3));
}

/**
 * @brief wait until busy states of SR register is cleared.
 *
 * @return true if success
*/
DATA_RAM_FUNCTION __STATIC_INLINE bool spic_wait_busy(void)
{
    uint32_t ctr = 0;

    //break only when tx error or busy state is cleared
    do
    {
        if (ctr++ >= 0x4000)
        {
            return false;
        }
    }
    while ((SPIC->sr & BIT_BUSY) && (!(SPIC->sr & BIT_TXE)));

    return true;
}


/**
* @brief  This function is used to send Rx command to flash to get status register or flash id.
* @param  cmd: command that need to be sent.
* @param  read_len: the number of bytes that will be read by SPIC after sending cmd.
* @param  read_buf: pointer to a byte array which is used to save datas received.
* @return true if success
* @note This function is only used for rx status/flash id ,not used for read flash data.
*/
extern bool flash_cmd_rx(uint8_t cmd, uint8_t read_len, uint8_t *read_buf);

/**
 * @brief  This function is used to send Flash Instruction with Data Phase.
 * @param  cmd: The command need to be sent.
 * @param  data_len: The length of Data phase.
 * @param  data_buf: pointer to byte array which are to be sent in Data phase.
 * @return true if success
 * @note
 *     - This function works User mode.
 *     - This function can only be used to tx cmd(WREN,WRSR,DP,RDP,ERASE etc.),not used to tx data.
 */
extern bool flash_cmd_tx(uint8_t cmd, uint8_t data_len, uint8_t *data_buf);

/**
 * @brief wait till flash is not busy.
 * @return true if success
*/
extern bool flash_wait_busy(void);

/**
 * @brief enable flash writing via sending WREN cmd to WLE bit in flash status register.
 * @return true if success
 * @note  before sending specific commands, this command is required to send to enable writig.
*/
extern bool flash_write_enable(void);

/**
 * @brief erase flash with specific type and address
 * @param type  erase type
 * @param addr  address to erase when erase block or sector
 * @return true if success
 * @note use this function only when no concern to be preempted, otherwise, use the locked one
*/
extern bool flash_erase(T_ERASE_TYPE type, uint32_t addr);

/**
 * @brief read flash split (continuous )read data via user mode
 * @param start_addr    start_addr address where is going to be read in flash
 * @param data_len      data length to be read
 * @param data          data buffer to be read into
 * @return              split counter to info that how many split times occur
 *                      return 0xFFFFFFFF if not align 4
 * @note In order to enhance throughput, only word align (4 bytes) is allowed to be written.
 *       auto mode performance is better than this function, and auto mode read is suggested.
 *       recommand to use auto read other than this api
*/
extern uint32_t flash_split_read(uint32_t start_addr, uint32_t data_len, uint8_t *data);


/**
 * @brief program bunch of data to flash via auto mode
 * @param start_addr    start_addr address where is going to be flash in flash
 * @param data          data to be program into
 * @param len           len of data buffer
 * @return none
 * @note  start address should be 4 byte align
*/
extern void flash_auto_write_buffer(uint32_t start_addr, uint32_t *data, uint32_t len);

/**
 * @brief program data to flash via auto mode
 * @param start_addr    start_addr address where is going to be flash in flash
 * @param data          data to be program into
 * @return none
 * @note
 *      1. auto write is sync with auto read. so if your use auto read, this api is preferred.
 *      2. if need sync with other, use api with lock postfix
 *      3. recommandate to use this other than flash_write
*/
extern void flash_auto_write(uint32_t start_addr, uint32_t data);

/**
 * @brief get rdid of external flash
 * @param none
 * @return rdid of flash
 * @note this is the first cmd, so some spic flash setting would do in this function either.
 *       the command may fail if it's prior to this function.
*/
extern uint32_t flash_get_rdid(void);


/**
 * @brief get flash id
 * @param none
 * @return flash_id
*/
extern uint32_t flash_get_flash_id(void);

/**
 * @brief configure SPIC enable
 * @param enable         enable / disable SPIC
 * @return none
*/
extern void flash_spic_enable(uint8_t enable);

/**
* @brief    get flash size info
* @return   flash size
*/
extern uint32_t flash_get_flash_size(void);

/**
* @brief    set block protect by map
* @param    bp_lv a set of BPx ~ BP0
* @retval  1 if success
* @retval  0 if not support
* @retval -1 if fail
*/
//extern T_FLASH_RET flash_set_block_protect(uint8_t bp_lv);

extern bool erase_flash_unlock(T_ERASE_TYPE type, uint32_t addr);

extern bool write_flash_unlock(uint32_t start_addr, uint32_t data_len, uint8_t *data);


/** @} */ /* End of group FLASH_DRIVER_Exported_Functions */


/** @} */ /* End of group FLASH_DRIVER */

#endif //_FLASH_DRIVER_H

