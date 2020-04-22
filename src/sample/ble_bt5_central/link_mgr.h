/**
*****************************************************************************************
*     Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      link_mgr.h
   * @brief     Define struct and functions about link.
   * @author    berni
   * @date      2018-04-27
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2018 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */
#ifndef _LINK_MANAGER_H_
#define _LINK_MANAGER_H_
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "gap_conn_le.h"
#include "app_msg.h"

/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @brief  Define device list table size. */
#define APP_MAX_DEVICE_INFO 30
#if APP_RECOMBINE_ADV_DATA
/** @brief  Maximum advertising data size. */
#define APP_MAX_EXT_ADV_TOTAL_LEN 1024
#endif

/** @addtogroup  BT5_CENTRAL_SCAN_MGR
    * @{
    */
/** @brief  Extended Scan Mode List.*/
typedef enum
{
    SCAN_UNTIL_DISABLED, /**< If Duration paramter is zero, continue scanning until scanning is disabled. */
    PERIOD_SCAN_UNTIL_DISABLED, /**< If Duration and Period parameters are non-zero, scan for the duration within a scan period, and scan periods continue until scanning is disabled. */
    SCAN_UNTIL_DURATION_EXPIRED, /**< If Duration parameter is non-zero and Period parameter is zero, continue scanning until duration has expired. */
} T_EXT_SCAN_MODE;

/**
 * @brief  Device list block definition.
 */
typedef struct
{
    uint8_t      bd_addr[GAP_BD_ADDR_LEN];  /**< remote BD */
    uint8_t      bd_type;              /**< remote BD type*/
} T_DEV_INFO;

#if APP_RECOMBINE_ADV_DATA
/**
 * @brief  Advertising data.
 */
typedef struct
{
    uint8_t      bd_addr[GAP_BD_ADDR_LEN];           /**< remote BD */
    bool
    flag;                               /**< flag of recombining advertising data, true: recombining, false: waiting extended advertising PDUs */
    uint16_t     event_type;                         /**< advertising event type */
    uint16_t     data_len;                           /**< length of recombined advertising data */
    uint8_t      p_data[APP_MAX_EXT_ADV_TOTAL_LEN];  /**< recombined advertising data */
} T_EXT_ADV_DATA;
#endif
/** @} */

/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @brief  Device list table, used to save discovered device informations. */
extern T_DEV_INFO dev_list[APP_MAX_DEVICE_INFO];
/** @brief  The number of device informations saved in dev_list. */
extern uint8_t dev_list_count;
#if APP_RECOMBINE_ADV_DATA
/** @brief  Advertising data, used to save advertising data. */
extern T_EXT_ADV_DATA *ext_adv_data;
/** @brief  Advertising event type of last failed recombination. */
extern uint16_t fail_event_type;
/** @brief  Bluetooth device address of last failed recombination. */
extern uint8_t fail_bd_addr[GAP_BD_ADDR_LEN];
#endif

/*============================================================================*
 *                              Functions
 *============================================================================*/
/**
* @brief   Add device information to device list.
*
* @param[in] bd_addr Peer device address.
* @param[in] bd_type Peer device address type.
* @retval true Success.
* @retval false Failed, device list is full.
*/
bool link_mgr_add_device(uint8_t *bd_addr, uint8_t bd_type);

/**
 * @brief Clear device list.
 * @retval void
 */
void link_mgr_clear_device_list(void);

#endif
