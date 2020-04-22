/**
*****************************************************************************************
*     Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file    beacon_sample_config.h
  * @brief   This header contains UUIDs for beacon modification service
  * @details
  * @author  Ethan
  * @date    2018-12-18
  * @version v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion **/
#ifndef _SIMPLE_BLE_CONFIG_H_
#define _SIMPLE_BLE_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif


/*============================================================================*
 *                         Macros
 *============================================================================*/

#define GATT_UUID_BEACON_SAMPLE_PROFILE                    0xA00A
#define GATT_UUID_CHAR_BEACON_SAMPLE_V1_READ               0xB001
#define GATT_UUID_CHAR_BEACON_UUID_WRITE                   0xB002
#define GATT_UUID_CHAR_BEACON_MAJOR_ID_WRITE               0xB003
#define GATT_UUID_CHAR_BEACON_MINOR_ID_WRITE               0xB004
#define GATT_UUID_CHAR_BEACON_TX_POWER_WRITE               0xB005



#ifdef __cplusplus
}
#endif

#endif
