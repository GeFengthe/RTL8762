/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      link_mgr.h
   * @brief     Define multilink manager struct and functions.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */
#ifndef _LINK_MANAGER_H_
#define _LINK_MANAGER_H_
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <app_msg.h>
#include <gap_conn_le.h>
#include <profile_client.h>
#include <simple_ble_client.h>
#include <gaps_client.h>
#include <bas_client.h>

/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @brief  Define device list table size. */
#define APP_MAX_DEVICE_INFO 6

/** @addtogroup  CENTRAL_SRV_DIS
    * @{
    */
/** @defgroup APP_DISCOV_BITS Application discover services Bits
* @{
 */
#define APP_DISCOV_GAPS_FLAG    0x01
#define APP_DISCOV_SIMP_FLAG    0x02
#define APP_DISCOV_BAS_FLAG     0x04
/**
  * @}
  */
/** @} */
/** @addtogroup  CENTRAL_GAP_MSG
    * @{
    */
/**
 * @brief  Application Link control block definition.
 */
typedef struct
{
    T_GAP_CONN_STATE        conn_state;          /**< Connection state. */
    uint8_t                 discovered_flags;    /**< discovered flags. */
    uint8_t                 srv_found_flags;     /**< service founded flogs. */
    T_GAP_REMOTE_ADDR_TYPE  bd_type;             /**< remote BD type*/
    uint8_t                 bd_addr[GAP_BD_ADDR_LEN]; /**< remote BD */
} T_APP_LINK;
/** @} */ /* End of group CENTRAL_GAP_MSG */
/** @addtogroup  CENTRAL_SCAN_MGR
    * @{
    */
/**
 * @brief  Device list block definition.
 */
typedef struct
{
    uint8_t      bd_addr[GAP_BD_ADDR_LEN];  /**< remote BD */
    uint8_t      bd_type;              /**< remote BD type*/
} T_DEV_INFO;
/** @} */
#if F_BT_GATT_SRV_HANDLE_STORAGE
/** @addtogroup  CENTRAL_SRV_DIS
    * @{
    */
/** @brief  App link table */
typedef struct
{
    uint8_t      srv_found_flags;
    uint8_t      bd_type;         /**< remote BD type*/
    uint8_t      bd_addr[GAP_BD_ADDR_LEN];  /**< remote BD */
    uint32_t     reserved;
    uint16_t     gaps_hdl_cache[HDL_GAPS_CACHE_LEN];
    uint16_t     simp_hdl_cache[HDL_SIMBLE_CACHE_LEN];
    uint16_t     bas_hdl_cache[HDL_BAS_CACHE_LEN];
} T_APP_SRVS_HDL_TABLE;
/** @} */
#endif

/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @brief  App link table */
extern T_APP_LINK app_link_table[APP_MAX_LINKS];
/** @brief  Device list table, used to save discovered device informations. */
extern T_DEV_INFO dev_list[APP_MAX_DEVICE_INFO];
/** @brief  The number of device informations saved in dev_list. */
extern uint8_t dev_list_count;

/*============================================================================*
 *                              Functions
 *============================================================================*/
bool link_mgr_add_device(uint8_t *bd_addr, uint8_t bd_type);
void link_mgr_clear_device_list(void);
#if F_BT_GATT_SRV_HANDLE_STORAGE
uint32_t app_save_srvs_hdl_table(T_APP_SRVS_HDL_TABLE *p_info);
uint32_t app_load_srvs_hdl_table(T_APP_SRVS_HDL_TABLE *p_info);
#endif

#endif
