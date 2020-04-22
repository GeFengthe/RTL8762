/**
*****************************************************************************************
*     Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      bt5_peripheral_stack_api.c
   * @brief     This file handles BLE BT5 peripheral application routines.
   * @author    berni
   * @date      2018-04-27
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2018 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "gap_ext_adv.h"
#include "bt5_peripheral_stack_api.h"

/** @defgroup  BT5_PERIPH_STACK_API BT5 Peripheral Stack API
    * @brief This file provides APIs about extended advertising parameters.
    * @{
    */
/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @brief  Default minimum advertising interval (units of 625us, 320=200ms) */
#define DEFAULT_ADVERTISING_INTERVAL_MIN            320
/** @brief  Default maximum advertising interval */
#define DEFAULT_ADVERTISING_INTERVAL_MAX            320

/** @brief  GAP - Advertisement data (best kept short to conserve power) */
static const uint8_t ext_adv_data[] =
{
    /* Flags */
    0x02,             /* length */
    GAP_ADTYPE_FLAGS, /* type="Flags" */
    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    /* Local name */
    0x13,             /* length */
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'B', 'L', 'E', '_', 'B', 'T', '5', '_', 'P', 'e', 'r', 'i', 'p', 'h', 'e', 'r', 'a', 'l',
    /* Manufacturer Specific Data */
    0xdd,             /* length */
    GAP_ADTYPE_MANUFACTURER_SPECIFIC,
    0x5d, 0x00,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
    0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
    0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
    0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
    0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb,
    0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd
};

/** @brief  GAP - Scan response data (best kept short to conserve power) */
static const uint8_t ext_scan_rsp_data[] =
{
    /* Manufacturer Specific Data */
    0xfc,             /* length */
    GAP_ADTYPE_MANUFACTURER_SPECIFIC,
    0x5d, 0x00,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
    0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
    0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5,
    0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
    0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb,
    0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xc, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd,
    0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf
};

/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @brief Identify advertising set */
uint8_t adv_handle; /**< Advertising handle*/
/** @brief Extended advertising state */
T_APP_EXT_ADV_STATE ext_adv_state[APP_MAX_ADV_SET] = {{APP_IDLE_ADV_SET, EXT_ADV_STATE_IDLE}, {APP_IDLE_ADV_SET, EXT_ADV_STATE_IDLE}, {APP_IDLE_ADV_SET, EXT_ADV_STATE_IDLE}, {APP_IDLE_ADV_SET, EXT_ADV_STATE_IDLE}}; /**< Extended advertising state */
uint8_t adv_set_num = 0;              /**< Advertising set number */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/**
  * @brief  Initialize parameters of non-connectable and non-scannable undirected
            advertising using extended advertising PDUs
  * @return void
  */
void le_init_ext_adv_params_ext(void)
{
    T_LE_EXT_ADV_EXTENDED_ADV_PROPERTY adv_event_prop =
        LE_EXT_ADV_EXTENDED_ADV_NON_SCAN_NON_CONN_UNDIRECTED;
    uint32_t primary_adv_interval_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint32_t primary_adv_interval_max = DEFAULT_ADVERTISING_INTERVAL_MAX;
    uint8_t  primary_adv_channel_map = GAP_ADVCHAN_ALL;
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_PUBLIC;
    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t p_peer_address[6] = {0};
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;
    /* Host has no preference. */
    uint8_t tx_power = 127;
    T_GAP_PHYS_PRIM_ADV_TYPE primary_adv_phy = GAP_PHYS_PRIM_ADV_1M;
    uint8_t secondary_adv_max_skip = 0;
    T_GAP_PHYS_TYPE secondary_adv_phy = GAP_PHYS_2M;
    uint8_t adv_sid = 0;
    bool scan_req_notification_enable = false;

    /* Initialize extended advertising parameters */
    adv_handle = le_ext_adv_create_adv_handle();
    le_ext_adv_set_adv_param(adv_handle,
                             adv_event_prop,
                             primary_adv_interval_min,
                             primary_adv_interval_max,
                             primary_adv_channel_map,
                             own_address_type,
                             peer_address_type,
                             p_peer_address,
                             filter_policy,
                             tx_power,
                             primary_adv_phy,
                             secondary_adv_max_skip,
                             secondary_adv_phy,
                             adv_sid,
                             scan_req_notification_enable);

    /* Initialize extended advertising data(max size when only one advertising set is using = 1024 bytes)*/
    le_ext_adv_set_adv_data(adv_handle, sizeof(ext_adv_data), (uint8_t *)ext_adv_data);
}

/**
  * @brief  Initialize parameters of non-connectable and non-scannable directed advertising using
            extended advertising PDUs
  * @return void
  */
void le_init_ext_adv_params_ext_directed(void)
{
    T_LE_EXT_ADV_EXTENDED_ADV_PROPERTY adv_event_prop =
        LE_EXT_ADV_EXTENDED_ADV_NON_SCAN_NON_CONN_DIRECTED ;
    uint32_t primary_adv_interval_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint32_t primary_adv_interval_max = DEFAULT_ADVERTISING_INTERVAL_MAX;
    uint8_t  primary_adv_channel_map = GAP_ADVCHAN_ALL;
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_PUBLIC;
    /* peer_address_type and peer_address shall be valid */
    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t p_peer_address[6] = {0x42, 0x44, 0x33, 0x22, 0x11, 0x00};
    /* filter_policy shall be ignored */
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;
    /* Host has no preference. */
    uint8_t tx_power = 127;
    T_GAP_PHYS_PRIM_ADV_TYPE primary_adv_phy = GAP_PHYS_PRIM_ADV_1M;
    uint8_t secondary_adv_max_skip = 0;
    T_GAP_PHYS_TYPE secondary_adv_phy = GAP_PHYS_2M;
    uint8_t adv_sid = 0;
    bool scan_req_notification_enable = false;

    /* Initialize extended advertising parameters */
    adv_handle = le_ext_adv_create_adv_handle();
    le_ext_adv_set_adv_param(adv_handle,
                             adv_event_prop,
                             primary_adv_interval_min,
                             primary_adv_interval_max,
                             primary_adv_channel_map,
                             own_address_type,
                             peer_address_type,
                             p_peer_address,
                             filter_policy,
                             tx_power,
                             primary_adv_phy,
                             secondary_adv_max_skip,
                             secondary_adv_phy,
                             adv_sid,
                             scan_req_notification_enable);

    /* Initialize extended advertising data(max size when only one advertising set is using = 1024 bytes)*/
    le_ext_adv_set_adv_data(adv_handle, sizeof(ext_adv_data), (uint8_t *)ext_adv_data);
}

/**
  * @brief  Initialize parameters of connectable undirected advertising using
            extended advertising PDUs
  * @return void
  */
void le_init_ext_adv_params_ext_conn(void)
{
    T_LE_EXT_ADV_EXTENDED_ADV_PROPERTY adv_event_prop = LE_EXT_ADV_EXTENDED_ADV_CONN_UNDIRECTED;
    uint32_t primary_adv_interval_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint32_t primary_adv_interval_max = DEFAULT_ADVERTISING_INTERVAL_MAX;
    uint8_t  primary_adv_channel_map = GAP_ADVCHAN_ALL;
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_PUBLIC;
    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t p_peer_address[6] = {0};
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;
    /* Host has no preference. */
    uint8_t tx_power = 127;
    T_GAP_PHYS_PRIM_ADV_TYPE primary_adv_phy;
    uint8_t secondary_adv_max_skip = 0;
    T_GAP_PHYS_TYPE secondary_adv_phy;
    uint8_t adv_sid = 0;
    bool scan_req_notification_enable = false;

    /* Initialize primary advertisement PHY and secondary advertisement PHY */
    if (ADVERTISING_PHY == APP_PRIMARY_1M_SECONDARY_2M)
    {
        primary_adv_phy = GAP_PHYS_PRIM_ADV_1M;
        secondary_adv_phy = GAP_PHYS_2M;
    }
    else if (ADVERTISING_PHY == APP_PRIMARY_CODED_SECONDARY_CODED)
    {
        primary_adv_phy = GAP_PHYS_PRIM_ADV_CODED;
        secondary_adv_phy = GAP_PHYS_CODED;
    }

    /* Initialize extended advertising parameters */
    adv_handle = le_ext_adv_create_adv_handle();
    if (adv_handle == APP_IDLE_ADV_SET)
    {
        return;
    }
    if (adv_set_num < APP_MAX_ADV_SET)
    {
        ext_adv_state[adv_set_num++].adv_handle = adv_handle;
    }

    le_ext_adv_set_adv_param(adv_handle,
                             adv_event_prop,
                             primary_adv_interval_min,
                             primary_adv_interval_max,
                             primary_adv_channel_map,
                             own_address_type,
                             peer_address_type,
                             p_peer_address,
                             filter_policy,
                             tx_power,
                             primary_adv_phy,
                             secondary_adv_max_skip,
                             secondary_adv_phy,
                             adv_sid,
                             scan_req_notification_enable);

    /* Initialize extended advertising data(max size = 245 bytes)*/
    le_ext_adv_set_adv_data(adv_handle, sizeof(ext_adv_data), (uint8_t *)ext_adv_data);
}


/**
  * @brief  Initialize parameters of connectable directed advertising using
            extended advertising PDUs
  * @return void
  */
void le_init_ext_adv_params_ext_conn_directed(void)
{
    T_LE_EXT_ADV_EXTENDED_ADV_PROPERTY adv_event_prop = LE_EXT_ADV_EXTENDED_ADV_CONN_DIRECTED ;
    uint32_t primary_adv_interval_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint32_t primary_adv_interval_max = DEFAULT_ADVERTISING_INTERVAL_MAX;
    uint8_t  primary_adv_channel_map = GAP_ADVCHAN_ALL;
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_PUBLIC;
    /* peer_address_type and peer_address shall be valid */
    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t p_peer_address[6] = {0x42, 0x44, 0x33, 0x22, 0x11, 0x00};
    /* filter_policy shall be ignored */
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;
    /* Host has no preference. */
    uint8_t tx_power = 127;
    T_GAP_PHYS_PRIM_ADV_TYPE primary_adv_phy = GAP_PHYS_PRIM_ADV_1M;
    uint8_t secondary_adv_max_skip = 0;
    T_GAP_PHYS_TYPE secondary_adv_phy = GAP_PHYS_2M;
    uint8_t adv_sid = 0;
    bool scan_req_notification_enable = false;

    /* Initialize extended advertising parameters */
    adv_handle = le_ext_adv_create_adv_handle();
    le_ext_adv_set_adv_param(adv_handle,
                             adv_event_prop,
                             primary_adv_interval_min,
                             primary_adv_interval_max,
                             primary_adv_channel_map,
                             own_address_type,
                             peer_address_type,
                             p_peer_address,
                             filter_policy,
                             tx_power,
                             primary_adv_phy,
                             secondary_adv_max_skip,
                             secondary_adv_phy,
                             adv_sid,
                             scan_req_notification_enable);

    /* Initialize extended advertising data(max size = 239 bytes)*/
    le_ext_adv_set_adv_data(adv_handle, sizeof(ext_adv_data), (uint8_t *)ext_adv_data);
}

/**
  * @brief  Initialize parameters of scannable undirected advertising using
            extended advertising PDUs
  * @return void
  */
void le_init_ext_adv_params_ext_scan(void)
{
    T_LE_EXT_ADV_EXTENDED_ADV_PROPERTY adv_event_prop = LE_EXT_ADV_EXTENDED_ADV_SCAN_UNDIRECTED;
    uint32_t primary_adv_interval_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint32_t primary_adv_interval_max = DEFAULT_ADVERTISING_INTERVAL_MAX;
    uint8_t  primary_adv_channel_map = GAP_ADVCHAN_ALL;
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_PUBLIC;
    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t p_peer_address[6] = {0};
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;
    /* Host has no preference. */
    uint8_t tx_power = 127;
    T_GAP_PHYS_PRIM_ADV_TYPE primary_adv_phy = GAP_PHYS_PRIM_ADV_1M;
    uint8_t secondary_adv_max_skip = 0;
    T_GAP_PHYS_TYPE secondary_adv_phy = GAP_PHYS_2M;
    uint8_t adv_sid = 0;
    bool scan_req_notification_enable = false;

    /* Initialize extended advertising parameters */
    adv_handle = le_ext_adv_create_adv_handle();
    le_ext_adv_set_adv_param(adv_handle,
                             adv_event_prop,
                             primary_adv_interval_min,
                             primary_adv_interval_max,
                             primary_adv_channel_map,
                             own_address_type,
                             peer_address_type,
                             p_peer_address,
                             filter_policy,
                             tx_power,
                             primary_adv_phy,
                             secondary_adv_max_skip,
                             secondary_adv_phy,
                             adv_sid,
                             scan_req_notification_enable);

    le_ext_adv_set_scan_response_data(adv_handle, sizeof(ext_scan_rsp_data),
                                      (uint8_t *)ext_scan_rsp_data);
}

/**
  * @brief  Initialize parameters of scannable directed advertising using
            extended advertising PDUs
  * @return void
  */
void le_init_ext_adv_params_ext_scan_directed(void)
{
    T_LE_EXT_ADV_EXTENDED_ADV_PROPERTY adv_event_prop = LE_EXT_ADV_EXTENDED_ADV_SCAN_DIRECTED;
    uint32_t primary_adv_interval_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint32_t primary_adv_interval_max = DEFAULT_ADVERTISING_INTERVAL_MAX;
    uint8_t  primary_adv_channel_map = GAP_ADVCHAN_ALL;
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_PUBLIC;
    /* peer_address_type and peer_address shall be valid */
    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t p_peer_address[6] = {0x42, 0x44, 0x33, 0x22, 0x11, 0x00};
    /* filter_policy shall be ignored */
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;
    /* Host has no preference. */
    uint8_t tx_power = 127;
    T_GAP_PHYS_PRIM_ADV_TYPE primary_adv_phy = GAP_PHYS_PRIM_ADV_1M;
    uint8_t secondary_adv_max_skip = 0;
    T_GAP_PHYS_TYPE secondary_adv_phy = GAP_PHYS_2M;
    uint8_t adv_sid = 0;
    bool scan_req_notification_enable = false;

    /* Initialize extended advertising parameters */
    adv_handle = le_ext_adv_create_adv_handle();
    le_ext_adv_set_adv_param(adv_handle,
                             adv_event_prop,
                             primary_adv_interval_min,
                             primary_adv_interval_max,
                             primary_adv_channel_map,
                             own_address_type,
                             peer_address_type,
                             p_peer_address,
                             filter_policy,
                             tx_power,
                             primary_adv_phy,
                             secondary_adv_max_skip,
                             secondary_adv_phy,
                             adv_sid,
                             scan_req_notification_enable);

    le_ext_adv_set_scan_response_data(adv_handle, sizeof(ext_scan_rsp_data),
                                      (uint8_t *)ext_scan_rsp_data);
}

/**
  * @brief  Continue advertising until advertising is disabled, or a connection is created.
  * @param[in] adv_handle Identify advertising set
  * @return void
  */
void le_init_ext_adv_enable_params(uint8_t adv_handle)
{
    uint16_t duration = 0;
    uint8_t  max_ext_adv_evt  = 0;

    le_ext_adv_set_adv_enable_param(adv_handle, duration, max_ext_adv_evt);
}

/**
  * @brief  Stop advertising when duration expires.
  * @param[in] adv_handle Identify advertising set
  * @return void
  */
void le_init_ext_adv_enable_params_duration(uint8_t adv_handle)
{
    /* Time = 5000 ms */
    uint16_t duration = 500;
    uint8_t  max_ext_adv_evt  = 0;

    le_ext_adv_set_adv_enable_param(adv_handle, duration, max_ext_adv_evt);
}

/**
  * @brief  Stop advertising when number of extended advertising events transmitted for
            the advertising set exceeds maximum number of extended advertising events.
  * @param[in] adv_handle Identify advertising set
  * @return void
  */
void le_init_ext_adv_enable_params_event(uint8_t adv_handle)
{
    uint16_t duration = 0;
    uint8_t  max_ext_adv_evt  = 100;

    le_ext_adv_set_adv_enable_param(adv_handle, duration, max_ext_adv_evt);
}

/** @} */ /* End of group BT5_PERIPH_STACK_API */
