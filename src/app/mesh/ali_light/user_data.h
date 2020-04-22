/**
*****************************************************************************************
*     copyright(c) 2015, realtek semiconductor corporation. all rights reserved.
*****************************************************************************************
  * @file     user_data.h
  * @brief    source file for command parse.
  * @details  data types and external functions declaration.
  * @author   hector
  * @date     2018-08-13
  * @version  v1.0
  * *************************************************************************************
  */

#ifndef _USER_DATA_H
#define _USER_DATA_H

#include "platform_types.h"
#include "flash_map.h"

BEGIN_DECLS

/**
 * @addtogroup USER_DATA
 * @{
 */

/**
 * @defgroup User_Data_Exported_Macros User Data Exported Macros
 * @brief
 * @{
 */

/**
 * @brief alibaba data storage address
 * @note this address must be equal or greater than OTA_TEMP + OTA_TEMP_SIZE address in flash map and
 *  must be 4k alignment
 */
#define FLASH_ALI_DATA_ADDRESS    (((OTA_TMP_ADDR + OTA_TMP_SIZE + 4095)>>12)<<12)
/** @} */

/**
 * @defgroup User_Data_Exported_Functions User Data Exported Functions
 * @brief
 * @{
 */
/**
 * @brief read alibaba secret key from flash
 * @param[out] key: alibaba specified secret key
 * @retval TRUE: read data from flash success
 * @retval FALSE: read data from flash failed
 */
bool user_data_read_ali_secret_key(uint8_t *key);

/**
 * @brief read alibaba product id from flash
 * @return alibaba product id
 */
uint32_t user_data_read_ali_product_id(void);

/**
 * @brief write alibaba specified data to flash
 * @param[in] id: alibaba specified product id
 * @param[in] key: alibaba specified secret key
 * @retval TRUE: write data to flash success
 * @retval FALSE: write data to flash failed
 */
bool user_data_write_ali_data(uint32_t id, const uint8_t *key);

/**
 * @brief check whether flash contains alibaba private date or not
 * @retval TRUE: flash contains alibaba data
 * @retval FALSE: flash has no alibaba data
 */
bool user_data_contains_ali_data(void);
/** @} */
/** @} */

END_DECLS

#endif /* _USER_DATA_H_ */


