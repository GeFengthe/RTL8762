/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      central_app.c
   * @brief     This file handles BLE central application routines.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <app_msg.h>
#include <string.h>
#include <trace.h>
#include <gap_scan.h>
#include <gap.h>
#include <gap_msg.h>
#include <gap_bond_le.h>
#include <central_app.h>
#include <link_mgr.h>
#include <user_cmd.h>
#include <user_cmd_parse.h>
#include <simple_ble_client.h>
#include <gaps_client.h>
#include <bas_client.h>


/** @defgroup  CENTRAL_APP Central Application
    * @brief This file handles BLE central application routines.
    * @{
    */
/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @addtogroup  CENTRAL_CLIIENT_CALLBACK
    * @{
    */
T_CLIENT_ID   simple_ble_client_id;  /**< Simple ble service client id*/
T_CLIENT_ID   gaps_client_id;        /**< gap service client id*/
T_CLIENT_ID   bas_client_id;         /**< battery service client id*/
/** @} */ /* End of group CENTRAL_CLIIENT_CALLBACK */

/** @defgroup  CENTRAL_GAP_MSG GAP Message Handler
    * @brief Handle GAP Message
    * @{
    */
T_GAP_DEV_STATE gap_dev_state = {0, 0, 0, 0};                 /**< GAP device state */
/*============================================================================*
 *                              Functions
 *============================================================================*/
void app_discov_services(uint8_t conn_id, bool start);
void app_handle_gap_msg(T_IO_MSG  *p_gap_msg);
/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg)
{
    uint16_t msg_type = io_msg.type;
    uint8_t rx_char;

    switch (msg_type)
    {
    case IO_MSG_TYPE_BT_STATUS:
        {
            app_handle_gap_msg(&io_msg);
        }
        break;
    case IO_MSG_TYPE_UART:
        /* We handle user command informations from Data UART in this branch. */
        rx_char = (uint8_t)io_msg.subtype;
        user_cmd_collect(&user_cmd_if, &rx_char, sizeof(rx_char), user_cmd_table);
        break;
    default:
        break;
    }
}

/**
 * @brief    Handle msg GAP_MSG_LE_DEV_STATE_CHANGE
 * @note     All the gap device state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] new_state  New gap device state
 * @param[in] cause GAP device state change cause
 * @return   void
 */
void app_handle_dev_state_evt(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    APP_PRINT_INFO3("app_handle_dev_state_evt: init state  %d, scan state %d, cause 0x%x",
                    new_state.gap_init_state,
                    new_state.gap_scan_state, cause);
    if (gap_dev_state.gap_init_state != new_state.gap_init_state)
    {
        if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
        {
            uint8_t bt_addr[6];
            APP_PRINT_INFO0("GAP stack ready");
            /*stack ready*/
            gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);
            data_uart_print("local bd addr: 0x%2x:%2x:%2x:%2x:%2x:%2x\r\n",
                            bt_addr[5],
                            bt_addr[4],
                            bt_addr[3],
                            bt_addr[2],
                            bt_addr[1],
                            bt_addr[0]);
        }
    }

    if (gap_dev_state.gap_scan_state != new_state.gap_scan_state)
    {
        if (new_state.gap_scan_state == GAP_SCAN_STATE_IDLE)
        {
            APP_PRINT_INFO0("GAP scan stop");
            data_uart_print("GAP scan stop\r\n");
        }
        else if (new_state.gap_scan_state == GAP_SCAN_STATE_SCANNING)
        {
            APP_PRINT_INFO0("GAP scan start");
            data_uart_print("GAP scan start\r\n");
        }
    }

    gap_dev_state = new_state;
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_STATE_CHANGE
 * @note     All the gap conn state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] conn_id Connection ID
 * @param[in] new_state  New gap connection state
 * @param[in] disc_cause Use this cause when new_state is GAP_CONN_STATE_DISCONNECTED
 * @return   void
 */
void app_handle_conn_state_evt(uint8_t conn_id, T_GAP_CONN_STATE new_state, uint16_t disc_cause)
{
    if (conn_id >= APP_MAX_LINKS)
    {
        return;
    }

    APP_PRINT_INFO4("app_handle_conn_state_evt: conn_id %d, conn_state(%d -> %d), disc_cause 0x%x",
                    conn_id, app_link_table[conn_id].conn_state, new_state, disc_cause);

    app_link_table[conn_id].conn_state = new_state;
    switch (new_state)
    {
    case GAP_CONN_STATE_DISCONNECTED:
        {
            if ((disc_cause != (HCI_ERR | HCI_ERR_REMOTE_USER_TERMINATE))
                && (disc_cause != (HCI_ERR | HCI_ERR_LOCAL_HOST_TERMINATE)))
            {
                APP_PRINT_ERROR2("app_handle_conn_state_evt: connection lost, conn_id %d, cause 0x%x", conn_id,
                                 disc_cause);
            }

            data_uart_print("Disconnect conn_id %d\r\n", conn_id);
            memset(&app_link_table[conn_id], 0, sizeof(T_APP_LINK));
        }
        break;

    case GAP_CONN_STATE_CONNECTED:
        {
            le_get_conn_addr(conn_id, app_link_table[conn_id].bd_addr,
                             &app_link_table[conn_id].bd_type);
            data_uart_print("Connected success conn_id %d\r\n", conn_id);
        }
        break;

    default:
        break;

    }
}

/**
 * @brief    Handle msg GAP_MSG_LE_AUTHEN_STATE_CHANGE
 * @note     All the gap authentication state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] conn_id Connection ID
 * @param[in] new_state  New authentication state
 * @param[in] cause Use this cause when new_state is GAP_AUTHEN_STATE_COMPLETE
 * @return   void
 */
void app_handle_authen_state_evt(uint8_t conn_id, uint8_t new_state, uint16_t cause)
{
    APP_PRINT_INFO2("app_handle_authen_state_evt:conn_id %d, cause 0x%x", conn_id, cause);

    switch (new_state)
    {
    case GAP_AUTHEN_STATE_STARTED:
        {
            APP_PRINT_INFO0("app_handle_authen_state_evt: GAP_AUTHEN_STATE_STARTED");
        }
        break;

    case GAP_AUTHEN_STATE_COMPLETE:
        {
            if (cause == GAP_SUCCESS)
            {
                data_uart_print("Pair success\r\n");
                APP_PRINT_INFO0("app_handle_authen_state_evt: GAP_AUTHEN_STATE_COMPLETE pair success");

            }
            else
            {
                data_uart_print("Pair failed: cause 0x%x\r\n", cause);
                APP_PRINT_INFO0("app_handle_authen_state_evt: GAP_AUTHEN_STATE_COMPLETE pair failed");
            }
        }
        break;

    default:
        {
            APP_PRINT_ERROR1("app_handle_authen_state_evt: unknown newstate %d", new_state);
        }
        break;
    }
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_MTU_INFO
 * @note     This msg is used to inform APP that exchange mtu procedure is completed.
 * @param[in] conn_id Connection ID
 * @param[in] mtu_size  New mtu size
 * @return   void
 */
void app_handle_conn_mtu_info_evt(uint8_t conn_id, uint16_t mtu_size)
{
    APP_PRINT_INFO2("app_handle_conn_mtu_info_evt: conn_id %d, mtu_size %d", conn_id, mtu_size);
    app_discov_services(conn_id, true);
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_PARAM_UPDATE
 * @note     All the connection parameter update change  events are pre-handled in this function.
 * @param[in] conn_id Connection ID
 * @param[in] status  New update state
 * @param[in] cause Use this cause when status is GAP_CONN_PARAM_UPDATE_STATUS_FAIL
 * @return   void
 */
void app_handle_conn_param_update_evt(uint8_t conn_id, uint8_t status, uint16_t cause)
{
    switch (status)
    {
    case GAP_CONN_PARAM_UPDATE_STATUS_SUCCESS:
        {
            uint16_t conn_interval;
            uint16_t conn_slave_latency;
            uint16_t conn_supervision_timeout;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_slave_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            APP_PRINT_INFO4("app_handle_conn_param_update_evt update success:conn_id %d, conn_interval 0x%x, conn_slave_latency 0x%x, conn_supervision_timeout 0x%x",
                            conn_id, conn_interval, conn_slave_latency, conn_supervision_timeout);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_FAIL:
        {
            APP_PRINT_ERROR2("app_handle_conn_param_update_evt update failed: conn_id %d, cause 0x%x",
                             conn_id, cause);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_PENDING:
        {
            APP_PRINT_INFO1("app_handle_conn_param_update_evt update pending: conn_id %d", conn_id);
        }
        break;

    default:
        break;
    }
}

/**
 * @brief    All the BT GAP MSG are pre-handled in this function.
 * @note     Then the event handling function shall be called according to the
 *           subtype of T_IO_MSG
 * @param[in] p_gap_msg Pointer to GAP msg
 * @return   void
 */
void app_handle_gap_msg(T_IO_MSG *p_gap_msg)
{
    T_LE_GAP_MSG gap_msg;
    uint8_t conn_id;
    memcpy(&gap_msg, &p_gap_msg->u.param, sizeof(p_gap_msg->u.param));

    APP_PRINT_TRACE1("app_handle_gap_msg: subtype %d", p_gap_msg->subtype);
    switch (p_gap_msg->subtype)
    {
    case GAP_MSG_LE_DEV_STATE_CHANGE:
        {
            app_handle_dev_state_evt(gap_msg.msg_data.gap_dev_state_change.new_state,
                                     gap_msg.msg_data.gap_dev_state_change.cause);
        }
        break;

    case GAP_MSG_LE_CONN_STATE_CHANGE:
        {
            app_handle_conn_state_evt(gap_msg.msg_data.gap_conn_state_change.conn_id,
                                      (T_GAP_CONN_STATE)gap_msg.msg_data.gap_conn_state_change.new_state,
                                      gap_msg.msg_data.gap_conn_state_change.disc_cause);
        }
        break;

    case GAP_MSG_LE_CONN_MTU_INFO:
        {
            app_handle_conn_mtu_info_evt(gap_msg.msg_data.gap_conn_mtu_info.conn_id,
                                         gap_msg.msg_data.gap_conn_mtu_info.mtu_size);
        }
        break;

    case GAP_MSG_LE_CONN_PARAM_UPDATE:
        {
            app_handle_conn_param_update_evt(gap_msg.msg_data.gap_conn_param_update.conn_id,
                                             gap_msg.msg_data.gap_conn_param_update.status,
                                             gap_msg.msg_data.gap_conn_param_update.cause);
        }
        break;

    case GAP_MSG_LE_AUTHEN_STATE_CHANGE:
        {
            app_handle_authen_state_evt(gap_msg.msg_data.gap_authen_state.conn_id,
                                        gap_msg.msg_data.gap_authen_state.new_state,
                                        gap_msg.msg_data.gap_authen_state.status);
        }
        break;

    case GAP_MSG_LE_BOND_JUST_WORK:
        {
            conn_id = gap_msg.msg_data.gap_bond_just_work_conf.conn_id;
            le_bond_just_work_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
            APP_PRINT_INFO0("GAP_MSG_LE_BOND_JUST_WORK");
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_DISPLAY:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_passkey_display.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO2("GAP_MSG_LE_BOND_PASSKEY_DISPLAY: conn_id %d, passkey %d",
                            conn_id, display_value);
            le_bond_passkey_display_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
            data_uart_print("GAP_MSG_LE_BOND_PASSKEY_DISPLAY: conn_id %d, passkey %d\r\n",
                            conn_id,
                            display_value);
        }
        break;

    case GAP_MSG_LE_BOND_USER_CONFIRMATION:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_user_conf.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO2("GAP_MSG_LE_BOND_USER_CONFIRMATION: conn_id %d, passkey %d",
                            conn_id, display_value);
            data_uart_print("GAP_MSG_LE_BOND_USER_CONFIRMATION: conn_id %d, passkey %d\r\n",
                            conn_id,
                            display_value);
            //le_bond_user_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_INPUT:
        {
            //uint32_t passkey = 888888;
            conn_id = gap_msg.msg_data.gap_bond_passkey_input.conn_id;
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d", conn_id);
            data_uart_print("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d\r\n", conn_id);
            //le_bond_passkey_input_confirm(conn_id, passkey, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_OOB_INPUT:
        {
            uint8_t oob_data[GAP_OOB_LEN] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            conn_id = gap_msg.msg_data.gap_bond_oob_input.conn_id;
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_OOB_INPUT: conn_id %d", conn_id);
            le_bond_set_param(GAP_PARAM_BOND_OOB_DATA, GAP_OOB_LEN, oob_data);
            le_bond_oob_input_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    default:
        APP_PRINT_ERROR1("app_handle_gap_msg: unknown subtype %d", p_gap_msg->subtype);
        break;
    }
}
/** @} */ /* End of group CENTRAL_GAP_MSG */

/** @defgroup  CENTRAL_SCAN_MGR Scan Information manager
    * @brief Scan Information manager
    * @{
    */
/**
  * @brief Use 16 bit uuid to filter scan information
  * @param[in] uuid 16 bit UUID.
  * @param[in] scan_info point to scan information data.
  * @return filter result
  * @retval true found success
  * @retval false not found
  */
bool filter_scan_info_by_uuid(uint16_t uuid, T_LE_SCAN_INFO *scan_info)
{
    uint8_t buffer[32];
    uint8_t pos = 0;

    while (pos < scan_info->data_len)
    {
        /* Length of the AD structure. */
        uint8_t length = scan_info->data[pos++];
        uint8_t type;

        if ((length < 1) || (length >= 31))
        {
            return false;
        }

        if ((length > 0x01) && ((pos + length) <= 31))
        {
            /* Copy the AD Data to buffer. */
            memcpy(buffer, scan_info->data + pos + 1, length - 1);
            /* AD Type, one octet. */
            type = scan_info->data[pos];

            switch (type)
            {
            case GAP_ADTYPE_16BIT_MORE:
            case GAP_ADTYPE_16BIT_COMPLETE:
            case GAP_ADTYPE_SERVICES_LIST_16BIT:
                {
                    uint16_t *p_uuid = (uint16_t *)(buffer);
                    uint8_t i = length - 1;

                    while (i >= 2)
                    {
                        APP_PRINT_INFO2("  AD Data: UUID16 List Item %d = 0x%x", i / 2, *p_uuid);
                        if (*p_uuid == uuid)
                        {
                            return true;
                        }
                        p_uuid++;
                        i -= 2;
                    }
                }
                break;

            default:
                break;
            }
        }

        pos += length;
    }
    return false;
}
/** @} */ /* End of group CENTRAL_SCAN_MGR */

/** @defgroup  CENTRAL_GAP_CALLBACK GAP Callback Event Handler
    * @brief Handle GAP callback event
    * @{
    */
/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data)
{
    T_APP_RESULT result = APP_RESULT_SUCCESS;
    T_LE_CB_DATA *p_data = (T_LE_CB_DATA *)p_cb_data;

    switch (cb_type)
    {
    case GAP_MSG_LE_SCAN_INFO:
        APP_PRINT_INFO5("GAP_MSG_LE_SCAN_INFO:adv_type 0x%x, bd_addr %s, remote_addr_type %d, rssi %d, data_len %d",
                        p_data->p_le_scan_info->adv_type,
                        TRACE_BDADDR(p_data->p_le_scan_info->bd_addr),
                        p_data->p_le_scan_info->remote_addr_type,
                        p_data->p_le_scan_info->rssi,
                        p_data->p_le_scan_info->data_len);
        /* User can split interested information by using the function as follow. */
        if (filter_scan_info_by_uuid(GATT_UUID_SIMPLE_PROFILE, p_data->p_le_scan_info))
        {
            APP_PRINT_INFO0("Found simple ble service");
            link_mgr_add_device(p_data->p_le_scan_info->bd_addr, p_data->p_le_scan_info->remote_addr_type);
        }
        /* If you want to parse the scan info, please reference function app_parse_scan_info in observer app. */
        break;

    case GAP_MSG_LE_CONN_UPDATE_IND:
        APP_PRINT_INFO5("GAP_MSG_LE_CONN_UPDATE_IND: conn_id %d, conn_interval_max 0x%x, conn_interval_min 0x%x, conn_latency 0x%x,supervision_timeout 0x%x",
                        p_data->p_le_conn_update_ind->conn_id,
                        p_data->p_le_conn_update_ind->conn_interval_max,
                        p_data->p_le_conn_update_ind->conn_interval_min,
                        p_data->p_le_conn_update_ind->conn_latency,
                        p_data->p_le_conn_update_ind->supervision_timeout);
        /* if reject the proposed connection parameter from peer device, use APP_RESULT_REJECT. */
        result = APP_RESULT_ACCEPT;
        break;

    default:
        APP_PRINT_ERROR1("app_gap_callback: unhandled cb_type 0x%x", cb_type);
        break;
    }
    return result;
}
/** @} */ /* End of group CENTRAL_GAP_CALLBACK */

/** @defgroup  CENTRAL_SRV_DIS GATT Services discovery and storage
    * @brief GATT Services discovery and storage
    * @{
    */
/**
 * @brief  Discovery GATT services
 * @param  conn_id connection ID.
 * @param  start first call. true - first call this function after conncection, false - not first
 * @retval None
 */
void app_discov_services(uint8_t conn_id, bool start)
{
    if (app_link_table[conn_id].conn_state != GAP_CONN_STATE_CONNECTED)
    {
        APP_PRINT_ERROR1("app_discov_services: conn_id %d not connected ", conn_id);
        return;
    }
    if (start)
    {
#if F_BT_GATT_SRV_HANDLE_STORAGE
        bool is_disc = true;
        T_APP_SRVS_HDL_TABLE app_srvs_table;
        if (app_load_srvs_hdl_table(&app_srvs_table) == 0)
        {
            if ((app_srvs_table.srv_found_flags != 0) &&
                (app_srvs_table.bd_type == app_link_table[conn_id].bd_type) &&
                (memcmp(app_srvs_table.bd_addr, app_link_table[conn_id].bd_addr, GAP_BD_ADDR_LEN) == 0))
            {
                APP_PRINT_INFO1("app_discov_services: load from flash, srv_found_flags 0x%x",
                                app_srvs_table.srv_found_flags);
                app_link_table[conn_id].srv_found_flags = app_srvs_table.srv_found_flags;
                if (app_srvs_table.srv_found_flags & APP_DISCOV_GAPS_FLAG)
                {
                    gaps_set_hdl_cache(conn_id, app_srvs_table.gaps_hdl_cache, sizeof(uint16_t) * HDL_GAPS_CACHE_LEN);
                }
                if (app_srvs_table.srv_found_flags & APP_DISCOV_SIMP_FLAG)
                {
                    simp_ble_client_set_hdl_cache(conn_id, app_srvs_table.simp_hdl_cache,
                                                  sizeof(uint16_t) * HDL_SIMBLE_CACHE_LEN);
                }
                if (app_srvs_table.srv_found_flags & APP_DISCOV_BAS_FLAG)
                {
                    bas_set_hdl_cache(conn_id, app_srvs_table.bas_hdl_cache, sizeof(uint16_t) * HDL_BAS_CACHE_LEN);
                }
                is_disc = false;
            }
        }
        else
        {
            APP_PRINT_ERROR0("app_load_srvs_hdl_table: failed");
        }

        if (is_disc)
        {
            if (gaps_start_discovery(conn_id) == false)
            {
                APP_PRINT_ERROR1("app_discov_services: discover gaps failed conn_id %d", conn_id);
            }
        }
#else
        if (gaps_start_discovery(conn_id) == false)
        {
            APP_PRINT_ERROR1("app_discov_services: discover gaps failed conn_id %d", conn_id);
        }
#endif
        return;
    }
    if ((app_link_table[conn_id].discovered_flags & APP_DISCOV_SIMP_FLAG) == 0)
    {
        if (simp_ble_client_start_discovery(conn_id) == false)
        {
            APP_PRINT_ERROR1("app_discov_services: discover simp failed conn_id %d", conn_id);
        }
    }
    else if ((app_link_table[conn_id].discovered_flags & APP_DISCOV_BAS_FLAG) == 0)
    {
        if (bas_start_discovery(conn_id) == false)
        {
            APP_PRINT_ERROR1("app_discov_services: discover bas failed conn_id %d", conn_id);
        }
    }
    else
    {
        APP_PRINT_INFO2("app_discov_services: discover complete, conn_id %d, srv_found_flags 0x%x",
                        conn_id, app_link_table[conn_id].srv_found_flags);
#if F_BT_GATT_SRV_HANDLE_STORAGE
        if (app_link_table[conn_id].srv_found_flags != 0)
        {
            T_APP_SRVS_HDL_TABLE app_srvs_table;
            memset(&app_srvs_table, 0, sizeof(T_APP_SRVS_HDL_TABLE));
            app_srvs_table.bd_type = app_link_table[conn_id].bd_type;
            app_srvs_table.srv_found_flags = app_link_table[conn_id].srv_found_flags;
            memcpy(app_srvs_table.bd_addr, app_link_table[conn_id].bd_addr, GAP_BD_ADDR_LEN);
            gaps_get_hdl_cache(conn_id, app_srvs_table.gaps_hdl_cache, sizeof(uint16_t) * HDL_GAPS_CACHE_LEN);
            simp_ble_client_get_hdl_cache(conn_id, app_srvs_table.simp_hdl_cache,
                                          sizeof(uint16_t) * HDL_SIMBLE_CACHE_LEN);
            bas_get_hdl_cache(conn_id, app_srvs_table.bas_hdl_cache, sizeof(uint16_t) * HDL_BAS_CACHE_LEN);
            if (app_save_srvs_hdl_table(&app_srvs_table) != 0)
            {
                APP_PRINT_ERROR0("app_save_srvs_hdl_table: failed");
            }
        }
#endif
    }

    return;
}
/** @} */ /* End of group CENTRAL_SRV_DIS */

/** @defgroup  CENTRAL_CLIIENT_CALLBACK Profile Client Callback Event Handler
    * @brief Handle profile client callback event
    * @{
    */

/**
 * @brief  Callback will be called when data sent from profile client layer.
 * @param  client_id the ID distinguish which module sent the data.
 * @param  conn_id connection ID.
 * @param  p_data  pointer to data.
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT app_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data)
{
    T_APP_RESULT  result = APP_RESULT_SUCCESS;
    APP_PRINT_INFO2("app_client_callback: client_id %d, conn_id %d",
                    client_id, conn_id);
    if (client_id == gaps_client_id)
    {
        T_GAPS_CLIENT_CB_DATA *p_gaps_cb_data = (T_GAPS_CLIENT_CB_DATA *)p_data;
        switch (p_gaps_cb_data->cb_type)
        {
        case GAPS_CLIENT_CB_TYPE_DISC_STATE:
            switch (p_gaps_cb_data->cb_content.disc_state)
            {
            case DISC_GAPS_DONE:
                app_link_table[conn_id].discovered_flags |= APP_DISCOV_GAPS_FLAG;
                app_link_table[conn_id].srv_found_flags |= APP_DISCOV_GAPS_FLAG;
                app_discov_services(conn_id, false);
                /* Discovery Simple BLE service procedure successfully done. */
                APP_PRINT_INFO0("app_client_callback: discover gaps procedure done.");
                break;
            case DISC_GAPS_FAILED:
                app_link_table[conn_id].discovered_flags |= APP_DISCOV_GAPS_FLAG;
                app_discov_services(conn_id, false);
                /* Discovery Request failed. */
                APP_PRINT_INFO0("app_client_callback: discover gaps request failed.");
                break;
            default:
                break;
            }
            break;
        case GAPS_CLIENT_CB_TYPE_READ_RESULT:
            switch (p_gaps_cb_data->cb_content.read_result.type)
            {
            case GAPS_READ_DEVICE_NAME:
                if (p_gaps_cb_data->cb_content.read_result.cause == GAP_SUCCESS)
                {
                    APP_PRINT_INFO1("GAPS_READ_DEVICE_NAME: device name %s.",
                                    TRACE_STRING(p_gaps_cb_data->cb_content.read_result.data.device_name.p_value));
                }
                else
                {
                    APP_PRINT_INFO1("GAPS_READ_DEVICE_NAME: failded cause 0x%x",
                                    p_gaps_cb_data->cb_content.read_result.cause);
                }
                break;
            case GAPS_READ_APPEARANCE:
                if (p_gaps_cb_data->cb_content.read_result.cause == GAP_SUCCESS)
                {
                    APP_PRINT_INFO1("GAPS_READ_APPEARANCE: appearance %d",
                                    p_gaps_cb_data->cb_content.read_result.data.appearance);
                }
                else
                {
                    APP_PRINT_INFO1("GAPS_READ_APPEARANCE: failded cause 0x%x",
                                    p_gaps_cb_data->cb_content.read_result.cause);
                }
                break;
            case GAPS_READ_CENTRAL_ADDR_RESOLUTION:
                if (p_gaps_cb_data->cb_content.read_result.cause == GAP_SUCCESS)
                {
                    APP_PRINT_INFO1("GAPS_READ_CENTRAL_ADDR_RESOLUTION: central_addr_res %d",
                                    p_gaps_cb_data->cb_content.read_result.data.central_addr_res);
                }
                else
                {
                    APP_PRINT_INFO1("GAPS_READ_CENTRAL_ADDR_RESOLUTION: failded cause 0x%x",
                                    p_gaps_cb_data->cb_content.read_result.cause);
                }
                break;
            default:
                break;
            }
            break;

        default:
            break;
        }
    }
    else if (client_id == simple_ble_client_id)
    {
        T_SIMP_CLIENT_CB_DATA *p_simp_client_cb_data = (T_SIMP_CLIENT_CB_DATA *)p_data;
        uint16_t value_size;
        uint8_t *p_value;
        switch (p_simp_client_cb_data->cb_type)
        {
        case SIMP_CLIENT_CB_TYPE_DISC_STATE:
            switch (p_simp_client_cb_data->cb_content.disc_state)
            {
            case DISC_SIMP_DONE:
                /* Discovery Simple BLE service procedure successfully done. */
                app_link_table[conn_id].discovered_flags |= APP_DISCOV_SIMP_FLAG;
                app_link_table[conn_id].srv_found_flags |= APP_DISCOV_SIMP_FLAG;
                app_discov_services(conn_id, false);
                APP_PRINT_INFO0("app_client_callback: discover simp procedure done.");
                break;
            case DISC_SIMP_FAILED:
                /* Discovery Request failed. */
                app_link_table[conn_id].discovered_flags |= APP_DISCOV_SIMP_FLAG;
                app_discov_services(conn_id, false);
                APP_PRINT_INFO0("app_client_callback: discover simp request failed.");
                break;
            default:
                break;
            }
            break;
        case SIMP_CLIENT_CB_TYPE_READ_RESULT:
            switch (p_simp_client_cb_data->cb_content.read_result.type)
            {
            case SIMP_READ_V1_READ:
                if (p_simp_client_cb_data->cb_content.read_result.cause == GAP_SUCCESS)
                {
                    value_size = p_simp_client_cb_data->cb_content.read_result.data.v1_read.value_size;
                    p_value = p_simp_client_cb_data->cb_content.read_result.data.v1_read.p_value;
                    APP_PRINT_INFO2("SIMP_READ_V1_READ: value_size %d, value %b",
                                    value_size, TRACE_BINARY(value_size, p_value));
                }
                else
                {
                    APP_PRINT_ERROR1("SIMP_READ_V1_READ: failed cause 0x%x",
                                     p_simp_client_cb_data->cb_content.read_result.cause);
                }
                break;
            case SIMP_READ_V3_NOTIFY_CCCD:
                if (p_simp_client_cb_data->cb_content.read_result.cause == GAP_SUCCESS)
                {
                    APP_PRINT_INFO1("SIMP_READ_V3_NOTIFY_CCCD: notify %d",
                                    p_simp_client_cb_data->cb_content.read_result.data.v3_notify_cccd);
                }
                else
                {
                    APP_PRINT_ERROR1("SIMP_READ_V3_NOTIFY_CCCD: failed cause 0x%x",
                                     p_simp_client_cb_data->cb_content.read_result.cause);
                };
                break;
            case SIMP_READ_V4_INDICATE_CCCD:
                if (p_simp_client_cb_data->cb_content.read_result.cause == GAP_SUCCESS)
                {
                    APP_PRINT_INFO1("SIMP_READ_V4_INDICATE_CCCD: indicate %d",
                                    p_simp_client_cb_data->cb_content.read_result.data.v4_indicate_cccd);
                }
                else
                {
                    APP_PRINT_ERROR1("SIMP_READ_V4_INDICATE_CCCD: failed cause 0x%x",
                                     p_simp_client_cb_data->cb_content.read_result.cause);
                };
                break;

            default:
                break;
            }
            break;
        case SIMP_CLIENT_CB_TYPE_WRITE_RESULT:
            switch (p_simp_client_cb_data->cb_content.write_result.type)
            {
            case SIMP_WRITE_V2_WRITE:
                APP_PRINT_INFO1("SIMP_WRITE_V2_WRITE: write result 0x%x",
                                p_simp_client_cb_data->cb_content.write_result.cause);
                break;
            case SIMP_WRITE_V3_NOTIFY_CCCD:
                APP_PRINT_INFO1("SIMP_WRITE_V3_NOTIFY_CCCD: write result 0x%x",
                                p_simp_client_cb_data->cb_content.write_result.cause);
                break;
            case SIMP_WRITE_V4_INDICATE_CCCD:
                APP_PRINT_INFO1("SIMP_WRITE_V4_INDICATE_CCCD: write result 0x%x",
                                p_simp_client_cb_data->cb_content.write_result.cause);
                break;
            default:
                break;
            }
            break;
        case SIMP_CLIENT_CB_TYPE_NOTIF_IND_RESULT:
            switch (p_simp_client_cb_data->cb_content.notif_ind_data.type)
            {
            case SIMP_V3_NOTIFY:
                value_size = p_simp_client_cb_data->cb_content.notif_ind_data.data.value_size;
                p_value = p_simp_client_cb_data->cb_content.notif_ind_data.data.p_value;
                APP_PRINT_INFO2("SIMP_V3_NOTIFY: value_size %d, value %b",
                                value_size, TRACE_BINARY(value_size, p_value));
                break;
            case SIMP_V4_INDICATE:
                value_size = p_simp_client_cb_data->cb_content.notif_ind_data.data.value_size;
                p_value = p_simp_client_cb_data->cb_content.notif_ind_data.data.p_value;
                APP_PRINT_INFO2("SIMP_V4_INDICATE: value_size %d, value %b",
                                value_size, TRACE_BINARY(value_size, p_value));
                break;
            default:
                break;
            }
            break;

        default:
            break;
        }
    }
    else if (client_id == bas_client_id)
    {
        T_BAS_CLIENT_CB_DATA *p_bas_cb_data = (T_BAS_CLIENT_CB_DATA *)p_data;
        switch (p_bas_cb_data->cb_type)
        {
        case BAS_CLIENT_CB_TYPE_DISC_STATE:
            switch (p_bas_cb_data->cb_content.disc_state)
            {
            case DISC_BAS_DONE:
                /* Discovery BAS procedure successfully done. */
                app_link_table[conn_id].discovered_flags |= APP_DISCOV_BAS_FLAG;
                app_link_table[conn_id].srv_found_flags |= APP_DISCOV_BAS_FLAG;
                app_discov_services(conn_id, false);
                APP_PRINT_INFO0("app_client_callback: discover bas procedure done");
                break;
            case DISC_BAS_FAILED:
                /* Discovery Request failed. */
                app_link_table[conn_id].discovered_flags |= APP_DISCOV_BAS_FLAG;
                app_discov_services(conn_id, false);
                APP_PRINT_INFO0("app_client_callback: discover bas procedure failed");
                break;
            default:
                break;
            }
            break;
        case BAS_CLIENT_CB_TYPE_READ_RESULT:
            switch (p_bas_cb_data->cb_content.read_result.type)
            {
            case BAS_READ_BATTERY_LEVEL:
                if (p_bas_cb_data->cb_content.read_result.cause == GAP_SUCCESS)
                {
                    APP_PRINT_INFO1("BAS_READ_BATTERY_LEVEL: battery level %d",
                                    p_bas_cb_data->cb_content.read_result.data.battery_level);
                }
                else
                {
                    APP_PRINT_ERROR1("BAS_READ_BATTERY_LEVEL: failed cause 0x%x",
                                     p_bas_cb_data->cb_content.read_result.cause);
                }
                break;
            case BAS_READ_NOTIFY:
                if (p_bas_cb_data->cb_content.read_result.cause == GAP_SUCCESS)
                {
                    APP_PRINT_INFO1("BAS_READ_NOTIFY: notify %d",
                                    p_bas_cb_data->cb_content.read_result.data.notify);
                }
                else
                {
                    APP_PRINT_ERROR1("BAS_READ_NOTIFY: failed cause 0x%x",
                                     p_bas_cb_data->cb_content.read_result.cause);
                };
                break;

            default:
                break;
            }
            break;
        case BAS_CLIENT_CB_TYPE_WRITE_RESULT:
            switch (p_bas_cb_data->cb_content.write_result.type)
            {
            case BAS_WRITE_NOTIFY_ENABLE:
                APP_PRINT_INFO1("BAS_WRITE_NOTIFY_ENABLE: write result 0x%x",
                                p_bas_cb_data->cb_content.write_result.cause);
                break;
            case BAS_WRITE_NOTIFY_DISABLE:
                APP_PRINT_INFO1("BAS_WRITE_NOTIFY_DISABLE: write result 0x%x",
                                p_bas_cb_data->cb_content.write_result.cause);
                break;
            default:
                break;
            }
            break;
        case BAS_CLIENT_CB_TYPE_NOTIF_IND_RESULT:
            APP_PRINT_INFO1("BAS_CLIENT_CB_TYPE_NOTIF_IND_RESULT: battery level %d",
                            p_bas_cb_data->cb_content.notify_data.battery_level);
            break;

        default:
            break;
        }
    }

    return result;
}

/** @} */ /* End of group CENTRAL_CLIENT_CALLBACK */
/** @} */ /* End of group CENTRAL_APP */

