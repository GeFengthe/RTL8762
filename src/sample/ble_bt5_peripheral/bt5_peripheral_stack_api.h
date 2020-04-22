/**
*****************************************************************************************
*     Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      bt5_peripheral_stack_api.h
   * @brief     This file handles BLE BT5 peripheral application routines.
   * @author    berni
   * @date      2018-04-27
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2018 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

#ifndef _BT5_PERIPHERAL_STACK_API__
#define _BT5_PERIPHERAL_STACK_API__

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "gap_le.h"
#include "gap_ext_adv.h"

/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @brief  Idle advertising set */
#define APP_IDLE_ADV_SET 0xFF
/** @brief  Maximum advertising set */
#define APP_MAX_ADV_SET 4

/*============================================================================*
 *                         Types
 *============================================================================*/
/** @brief Advertising PHY types */
typedef enum
{
    APP_PRIMARY_1M                      = 0x10,//!< Using legacy advertising PDUs, primary advertising PHY is LE 1M PHY, and secondary advertising PHY parameter is invalid.
    APP_PRIMARY_1M_SECONDARY_1M         = 0x11,//!< Using extended advertising PDUs, primary advertising PHY is LE 1M PHY, and secondary advertising PHY is LE 1M PHY.
    APP_PRIMARY_1M_SECONDARY_2M         = 0x12,//!< Using extended advertising PDUs, primary advertising PHY is LE 1M PHY, and secondary advertising PHY is LE 2M PHY.
    APP_PRIMARY_1M_SECONDARY_CODED      = 0x13,//!< Using extended advertising PDUs, primary advertising PHY is LE 1M PHY, and secondary advertising PHY is LE Coded PHY.
    APP_PRIMARY_CODED_SECONDARY_1M      = 0x31,//!< Using extended advertising PDUs, primary advertising PHY is LE Coded PHY, and secondary advertising PHY is LE 1M PHY.
    APP_PRIMARY_CODED_SECONDARY_2M      = 0x32,//!< Using extended advertising PDUs, primary advertising PHY is LE Coded PHY, and secondary advertising PHY is LE 2M PHY.
    APP_PRIMARY_CODED_SECONDARY_CODED   = 0x33,//!< Using extended advertising PDUs, primary advertising PHY is LE Coded PHY, and secondary advertising PHY is LE Coded PHY.
} T_APP_EXT_ADV_PHYS_TYPE;

typedef struct
{
    uint8_t             adv_handle;
    T_GAP_EXT_ADV_STATE ext_adv_state;
} T_APP_EXT_ADV_STATE;

/*============================================================================*
 *                              Variables
 *============================================================================*/
extern uint8_t adv_handle; /**< Advertising handle*/
extern T_APP_EXT_ADV_STATE ext_adv_state[APP_MAX_ADV_SET]; /**< Extended advertising state */
extern uint8_t adv_set_num;              /**< Advertising set number */

/*============================================================================*
 *                              Functisons
 *============================================================================*/
/**
  * @brief  Initialize parameters of non-connectable and non-scannable undirected
            advertising using extended advertising PDUs
  * @return void
  */
void le_init_ext_adv_params_ext(void);

/**
  * @brief  Initialize parameters of non-connectable and non-scannable directed advertising using
            extended advertising PDUs
  * @return void
  */
void le_init_ext_adv_params_ext_directed(void);

/**
  * @brief  Initialize parameters of connectable undirected advertising using
            extended advertising PDUs
  * @return void
  */
void le_init_ext_adv_params_ext_conn(void);

/**
  * @brief  Initialize parameters of connectable directed advertising using
            extended advertising PDUs
  * @return void
  */
void le_init_ext_adv_params_ext_conn_directed(void);

/**
  * @brief  Initialize parameters of scannable undirected advertising using
            extended advertising PDUs
  * @return void
  */
void le_init_ext_adv_params_ext_scan(void);

/**
  * @brief  Initialize parameters of scannable directed advertising using
            extended advertising PDUs
  * @return void
  */
void le_init_ext_adv_params_ext_scan_directed(void);

/**
  * @brief  Continue advertising until advertising is disabled, or a connection is created.
  * @param[in] adv_handle Identify advertising set
  * @return void
  */
void le_init_ext_adv_enable_params(uint8_t adv_handle);

/**
  * @brief  Stop advertising when duration expires.
  * @param[in] adv_handle Identify advertising set
  * @return void
  */
void le_init_ext_adv_enable_params_duration(uint8_t adv_handle);

/**
  * @brief  Stop advertising when number of extended advertising events transmitted for
            the advertising set exceeds maximum number of extended advertising events.
  * @param[in] adv_handle Identify advertising set
  * @return void
  */
void le_init_ext_adv_enable_params_event(uint8_t adv_handle);

#ifdef __cplusplus
}
#endif

#endif

