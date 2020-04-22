/**
************************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
************************************************************************************************************
* @file     rtl876x_lib_platform.h
* @brief
* @details
* @author
* @date     2018-07-17
* @version
*************************************************************************************************************
*/

#ifndef _RTL876X_LIB_PLATFORM_H_
#define _RTL876X_LIB_PLATFORM_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    READ_SUCCESS          = 0,
    READ_NOT_FIND_CONFIG  = 1,
    READ_NOT_FIND_BDADDR  = 2,
    READ_NOT_FIND_XTAL_OFFSET = 3,
} T_READ_BDADDR_RESULT;

typedef enum
{
    EFUSE_UPDATE_SUCCESS = 0,
    EFUSE_UPDATE_READ_PROTECTED,
    EFUSE_UPDATE_WRITE_MAC_FAIL,
    EFUSE_UPDATE_SPACE_EXHAUSTED,
    EFUSE_UPDATE_READ_FAIL,
} T_EFUSE_UPDATE_RESULT;

/**
  * @brief  read bd addr set in config file
  * @param  p_bdaddr: buffer to save bd addr
  * @retval read result
  *     @arg 0: read success
  *     @arg 1: no config file in flash
  *     @arg 2: no bd setting found
  */
T_READ_BDADDR_RESULT read_config_bdaddr(uint8_t *p_bdaddr);

/**
  * @brief  read xtal offset set in config file
  * @param  p_xtal_offset: buffer to save xtal offset
  * @retval read result
  *     @arg 0: read success
  *     @arg 1: no config file in flash
  *     @arg 3: no xtal offset found
  */
T_READ_BDADDR_RESULT read_config_xtal_offset(uint8_t *p_xtal_offset);

/**
  * @brief  update bd address set in config file
  * @param  p_cfg_read_addr: the address of read config file
    * @param  p_cfg_write_addr: the address of write config file
    * @param  p_bdaddr: buffer to the updated bd address
  * @retval update result
  *     @arg true: update success
  *     @arg false: update fail
  */
bool update_bdaddr(uint8_t *p_cfg_read_addr, uint8_t *p_cfg_write_addr, uint8_t *p_bdaddr);

/**
  * @brief  update xtal offset set in config file
  * @param  p_cfg_read_addr: the address of read config file
    * @param  p_cfg_write_addr: the address of write config file
    * @param  xtal: the updated xtal offset
  * @retval update result
  *     @arg true: update success
  *     @arg false: update fail
  */
bool update_xtal_offset(uint8_t *p_cfg_read_addr, uint8_t *p_cfg_write_addr, uint8_t xtal);

/**
  * @brief Write MAC address to config, this is mainly used on production line.
  * @param[in] p_mac_addr         The buffer hold MAC address (48 bits).
  * @return Write MAC to config fail or success.
  *     @retval true              Write MAC to config success.
  *     @retval false             Write MAC to config fails or not write existed MAC.
  */
bool UpdateMAC(uint8_t *p_mac_addr);

/**
  * @brief Write MAC address to eFuse, this is mainly used on production line.
  *        Because eFuse space limitation, only write MAC to eFuse once supported.
  * @param[in] p_mac_addr         The buffer hold MAC address (48 bits).
  * @return Write MAC to config fail or success.
  *     @retval EFUSE_UPDATE_SUCCESS         Write MAC to eFuse success.
  *     @retval EFUSE_UPDATE_READ_PROTECTED  Can not update eFuse while it is read protected.
  *     @retval EFUSE_UPDATE_WRITE_MAC_FAIL  Write MAC to eFuse fails.
  *     @retval EFUSE_UPDATE_SPACE_EXHAUSTED eFuse space is exhausted.
  */
T_EFUSE_UPDATE_RESULT update_mac_to_efuse(uint8_t *p_mac_addr);

/**
  * @brief Write 40M XTAL calibration data to config, this is mainly used on production line.
  * @param[in] xtal               The value of 40M XTAL calibration data
  * @return Write calibration data to config fail or success.
  *     @retval true              Success.
  *     @retval false             Fail.
  */
bool WriteXtalToConfig(uint8_t xtal);

/**
  * @brief Write 40M XTAL calibration data to Efuse, this is mainly used on production line.
  * @param[in] xtal               The value of 40M XTAL calibration data
  * @return Write calibration data to Efuse fail or success.
  *     @retval true              Success.
  *     @retval false             Fail.
  * @note The Efuse space is limited, please don't call this function more than 5 five times.
  */
bool WriteXtalToEfuse(uint8_t xtal);

#ifdef __cplusplus
}
#endif

#endif /* _RTL876X_LIB_PLATFORM_H_ */
