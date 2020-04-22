/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      scatternet_app.c
   * @brief     This file handles BLE scatternet application routines.
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

#include <string.h>
#include <app_msg.h>
#include <trace.h>
#include <gap_scan.h>
#include <gap.h>
#include <gap_msg.h>
#include <gap_bond_le.h>
#include <scatternet_app.h>
#include <link_mgr.h>
#include <user_cmd.h>
#include <user_cmd_parse.h>
#include <simple_ble_client.h>
#include <gaps_client.h>
#include <bas_client.h>
#include <profile_server.h>
#include <simple_ble_service.h>
#include <bas.h>
#include <gatt_builtin_services.h>


/** @defgroup  SCATTERNET_APP Scatternet Application
    * @brief This file handles BLE scatternet application routines.
    * @{
    */
/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @addtogroup  SCATTERNET_CLIIENT_CALLBACK
    * @{
    */
T_CLIENT_ID   simple_ble_client_id;  /**< Simple ble service client id*/
T_CLIENT_ID   gaps_client_id;        /**< gap service client id*/
T_CLIENT_ID   bas_client_id;         /**< battery service client id*/
/** @} */ /* End of group SCATTERNET_CLIIENT_CALLBACK */

/** @addtogroup  SCATTERNET_SEVER_CALLBACK
    * @{
    */
T_SERVER_ID simp_srv_id; /**< Simple ble service id*/
T_SERVER_ID bas_srv_id;  /**< Battery service id */
/** @} */ /* End of group SCATTERNET_SEVER_CALLBACK */

/** @defgroup  SCATTERNET_GAP_MSG GAP Message Handler
    * @brief Handle GAP Message
    * @{
    */
T_GAP_DEV_STATE gap_dev_state = {0, 0, 0, 0};                 /**< GAP device state */
/*============================================================================*
 *                              Functions
 *============================================================================*/
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
    APP_PRINT_INFO4("app_handle_dev_state_evt: init state  %d, adv state %d, scan state %d, cause 0x%x",
                    new_state.gap_init_state, new_state.gap_adv_state,
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
                            bt_addr[5], bt_addr[4], bt_addr[3],
                            bt_addr[2], bt_addr[1], bt_addr[0]);
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

    if (gap_dev_state.gap_adv_state != new_state.gap_adv_state)
    {
        if (new_state.gap_adv_state == GAP_ADV_STATE_IDLE)
        {
            if (new_state.gap_adv_sub_state == GAP_ADV_TO_IDLE_CAUSE_CONN)
            {
                APP_PRINT_INFO0("GAP adv stoped: because connection created");
            }
            else
            {
                APP_PRINT_INFO0("GAP adv stoped");
            }
            data_uart_print("GAP adv stoped\r\n");
        }
        else if (new_state.gap_adv_state == GAP_ADV_STATE_ADVERTISING)
        {
            APP_PRINT_INFO0("GAP adv start");
            data_uart_print("GAP adv start\r\n");
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
            uint16_t conn_interval;
            uint16_t conn_latency;
            uint16_t conn_supervision_timeout;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            le_get_conn_addr(conn_id, app_link_table[conn_id].bd_addr,
                             &app_link_table[conn_id].bd_type);
            APP_PRINT_INFO5("GAP_CONN_STATE_CONNECTED:remote_bd %s, remote_addr_type %d, conn_interval 0x%x, conn_latency 0x%x, conn_supervision_timeout 0x%x",
                            TRACE_BDADDR(app_link_table[conn_id].bd_addr), app_link_table[conn_id].bd_type,
                            conn_interval, conn_latency, conn_supervision_timeout);
            data_uart_print("Connected success conn_id %d\r\n", conn_id);
#if F_BT_LE_5_0_SET_PHY_SUPPORT
            {
                uint8_t tx_phy;
                uint8_t rx_phy;
                le_get_conn_param(GAP_PARAM_CONN_RX_PHY_TYPE, &rx_phy, conn_id);
                le_get_conn_param(GAP_PARAM_CONN_TX_PHY_TYPE, &tx_phy, conn_id);
                APP_PRINT_INFO2("GAP_CONN_STATE_CONNECTED: tx_phy %d, rx_phy %d", tx_phy, rx_phy);
            }
#endif
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
            APP_PRINT_INFO2("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d, key_press %d",
                            conn_id, gap_msg.msg_data.gap_bond_passkey_input.key_press);
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
/** @} */ /* End of group SCATTERNET_GAP_MSG */

/** @defgroup  SCATTERNET_SCAN_MGR Scan Information manager
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
/** @} */ /* End of group SCATTERNET_SCAN_MGR */

/** @defgroup  SCATTERNET_GAP_CALLBACK GAP Callback Event Handler
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
    /* common msg*/
    case GAP_MSG_LE_READ_RSSI:
        APP_PRINT_INFO3("GAP_MSG_LE_READ_RSSI:conn_id 0x%x cause 0x%x rssi %d",
                        p_data->p_le_read_rssi_rsp->conn_id,
                        p_data->p_le_read_rssi_rsp->cause,
                        p_data->p_le_read_rssi_rsp->rssi);
        break;

    case GAP_MSG_LE_DATA_LEN_CHANGE_INFO:
        APP_PRINT_INFO3("GAP_MSG_LE_DATA_LEN_CHANGE_INFO: conn_id %d, tx octets 0x%x, max_tx_time 0x%x",
                        p_data->p_le_data_len_change_info->conn_id,
                        p_data->p_le_data_len_change_info->max_tx_octets,
                        p_data->p_le_data_len_change_info->max_tx_time);
        break;

    case GAP_MSG_LE_BOND_MODIFY_INFO:
        APP_PRINT_INFO1("GAP_MSG_LE_BOND_MODIFY_INFO: type 0x%x",
                        p_data->p_le_bond_modify_info->type);
        break;

    case GAP_MSG_LE_MODIFY_WHITE_LIST:
        APP_PRINT_INFO2("GAP_MSG_LE_MODIFY_WHITE_LIST: operation %d, cause 0x%x",
                        p_data->p_le_modify_white_list_rsp->operation,
                        p_data->p_le_modify_white_list_rsp->cause);
        break;
    /* central reference msg*/
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

    case GAP_MSG_LE_SET_HOST_CHANN_CLASSIF:
        APP_PRINT_INFO1("GAP_MSG_LE_SET_HOST_CHANN_CLASSIF: cause 0x%x",
                        p_data->p_le_set_host_chann_classif_rsp->cause);
        break;
    /* peripheral reference msg*/
    case GAP_MSG_LE_ADV_UPDATE_PARAM:
        APP_PRINT_INFO1("GAP_MSG_LE_ADV_UPDATE_PARAM: cause 0x%x",
                        p_data->p_le_adv_update_param_rsp->cause);
        break;

    case GAP_MSG_LE_DISABLE_SLAVE_LATENCY:
        APP_PRINT_INFO1("GAP_MSG_LE_DISABLE_SLAVE_LATENCY: cause 0x%x",
                        p_data->p_le_disable_slave_latency_rsp->cause);
        break;

    case GAP_MSG_LE_UPDATE_PASSED_CHANN_MAP:
        APP_PRINT_INFO1("GAP_MSG_LE_UPDATE_PASSED_CHANN_MAP:cause 0x%x",
                        p_data->p_le_update_passed_chann_map_rsp->cause);
        break;
#if F_BT_LE_5_0_SET_PHY_SUPPORT
    case GAP_MSG_LE_PHY_UPDATE_INFO:
        APP_PRINT_INFO4("GAP_MSG_LE_PHY_UPDATE_INFO:conn_id %d, cause 0x%x, rx_phy %d, tx_phy %d",
                        p_data->p_le_phy_update_info->conn_id,
                        p_data->p_le_phy_update_info->cause,
                        p_data->p_le_phy_update_info->rx_phy,
                        p_data->p_le_phy_update_info->tx_phy);
        break;

    case GAP_MSG_LE_REMOTE_FEATS_INFO:
        {
            uint8_t  remote_feats[8];
            APP_PRINT_INFO3("GAP_MSG_LE_REMOTE_FEATS_INFO: conn id %d, cause 0x%x, remote_feats %b",
                            p_data->p_le_remote_feats_info->conn_id,
                            p_data->p_le_remote_feats_info->cause,
                            TRACE_BINARY(8, p_data->p_le_remote_feats_info->remote_feats));
            if (p_data->p_le_remote_feats_info->cause == GAP_SUCCESS)
            {
                memcpy(remote_feats, p_data->p_le_remote_feats_info->remote_feats, 8);
                if (remote_feats[LE_SUPPORT_FEATURES_MASK_ARRAY_INDEX1] & LE_SUPPORT_FEATURES_LE_2M_MASK_BIT)
                {
                    APP_PRINT_INFO0("GAP_MSG_LE_REMOTE_FEATS_INFO: support 2M");
                }
                if (remote_feats[LE_SUPPORT_FEATURES_MASK_ARRAY_INDEX1] & LE_SUPPORT_FEATURES_LE_CODED_PHY_MASK_BIT)
                {
                    APP_PRINT_INFO0("GAP_MSG_LE_REMOTE_FEATS_INFO: support CODED");
                }
            }
        }
        break;
#endif
    default:
        APP_PRINT_ERROR1("app_gap_callback: unhandled cb_type 0x%x", cb_type);
        break;
    }
    return result;
}
/** @} */ /* End of group SCATTERNET_GAP_CALLBACK */

#if F_BT_AIRPLANE_MODE_SUPPORT
/** @defgroup  SCATTERNET_AIRPLANE Airplane Mode Handler
    * @brief Use @ref F_BT_AIRPLANE_MODE_SUPPORT to open
    * @{
    */
/**
  * @brief Callback for gap common module to notify app
  * @param[in] cb_type callback msy type @ref GAP_COMMON_MSG_TYPE.
  * @param[in] p_cb_data point to callback data @ref T_GAP_CB_DATA.
  * @retval void
  */
void app_gap_common_callback(uint8_t cb_type, void *p_cb_data)
{
    T_GAP_CB_DATA cb_data;
    memcpy(&cb_data, p_cb_data, sizeof(T_GAP_CB_DATA));
    APP_PRINT_INFO1("app_gap_common_callback: cb_type = %d", cb_type);
    switch (cb_type)
    {
    case GAP_MSG_WRITE_AIRPLAN_MODE:
        APP_PRINT_INFO1("GAP_MSG_WRITE_AIRPLAN_MODE: cause 0x%x",
                        cb_data.p_gap_write_airplan_mode_rsp->cause);
        break;
    case GAP_MSG_READ_AIRPLAN_MODE:
        APP_PRINT_INFO2("GAP_MSG_READ_AIRPLAN_MODE: cause 0x%x, mode %d",
                        cb_data.p_gap_read_airplan_mode_rsp->cause,
                        cb_data.p_gap_read_airplan_mode_rsp->mode);
        break;
    default:
        break;
    }
    return;
}
/** @} */
#endif

#if F_BT_GAPS_CHAR_WRITEABLE
/** @defgroup  SCATTERNET_GAPS_WRITE GAP Service Callback Handler
    * @brief Use @ref F_BT_GAPS_CHAR_WRITEABLE to open
    * @{
    */
/**
 * @brief    All the BT GAP service callback events are handled in this function
 * @param[in] service_id  Profile service ID
 * @param[in] p_para      Pointer to callback data
 * @return   Indicates the function call is successful or not
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT gap_service_callback(T_SERVER_ID service_id, void *p_para)
{
    T_APP_RESULT  result = APP_RESULT_SUCCESS;
    T_GAPS_CALLBACK_DATA *p_gap_data = (T_GAPS_CALLBACK_DATA *)p_para;
    APP_PRINT_INFO2("gap_service_callback conn_id = %d msg_type = %d\n", p_gap_data->conn_id,
                    p_gap_data->msg_type);
    if (p_gap_data->msg_type == SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE)
    {
        switch (p_gap_data->msg_data.opcode)
        {
        case GAPS_WRITE_DEVICE_NAME:
            {
                T_LOCAL_NAME device_name;
                memcpy(device_name.local_name, p_gap_data->msg_data.p_value, p_gap_data->msg_data.len);
                device_name.local_name[p_gap_data->msg_data.len] = 0;
                flash_save_local_name(&device_name);
            }
            break;

        case GAPS_WRITE_APPEARANCE:
            {
                uint16_t appearance_val;
                T_LOCAL_APPEARANCE appearance;

                LE_ARRAY_TO_UINT16(appearance_val, p_gap_data->msg_data.p_value);
                appearance.local_appearance = appearance_val;
                flash_save_local_appearance(&appearance);
            }
            break;

        default:
            break;
        }
    }
    return result;
}
/** @} */
#endif

/** @defgroup  SCATTERNET_CLIIENT_CALLBACK Profile Client Callback Event Handler
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
    if (client_id == CLIENT_PROFILE_GENERAL_ID)
    {
        T_CLIENT_APP_CB_DATA *p_client_app_cb_data = (T_CLIENT_APP_CB_DATA *)p_data;
        switch (p_client_app_cb_data->cb_type)
        {
        case CLIENT_APP_CB_TYPE_DISC_STATE:
            if (p_client_app_cb_data->cb_content.disc_state_data.disc_state == DISC_STATE_SRV_DONE)
            {
                APP_PRINT_INFO0("Discovery All Service Procedure Done.");
            }
            else
            {
                APP_PRINT_INFO0("Discovery state send to application directly.");
            }
            break;
        case CLIENT_APP_CB_TYPE_DISC_RESULT:
            if (p_client_app_cb_data->cb_content.disc_result_data.result_type == DISC_RESULT_ALL_SRV_UUID16)
            {
                APP_PRINT_INFO3("Discovery All Primary Service: UUID16 0x%x, start handle 0x%x, end handle 0x%x.",
                                p_client_app_cb_data->cb_content.disc_result_data.result_data.p_srv_uuid16_disc_data->uuid16,
                                p_client_app_cb_data->cb_content.disc_result_data.result_data.p_srv_uuid16_disc_data->att_handle,
                                p_client_app_cb_data->cb_content.disc_result_data.result_data.p_srv_uuid16_disc_data->end_group_handle);
            }
            else
            {
                APP_PRINT_INFO0("Discovery result send to application directly.");
            }
            break;
        default:
            break;
        }

    }
    else if (client_id == gaps_client_id)
    {
        T_GAPS_CLIENT_CB_DATA *p_gaps_cb_data = (T_GAPS_CLIENT_CB_DATA *)p_data;
        switch (p_gaps_cb_data->cb_type)
        {
        case GAPS_CLIENT_CB_TYPE_DISC_STATE:
            switch (p_gaps_cb_data->cb_content.disc_state)
            {
            case DISC_GAPS_DONE:
                /* Discovery Simple BLE service procedure successfully done. */
                APP_PRINT_INFO0("app_client_callback: discover gaps procedure done.");
                break;
            case DISC_GAPS_FAILED:
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
                APP_PRINT_INFO0("app_client_callback: discover simp procedure done.");
                break;
            case DISC_SIMP_FAILED:
                /* Discovery Request failed. */
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
    if (client_id == bas_client_id)
    {
        T_BAS_CLIENT_CB_DATA *p_bas_cb_data = (T_BAS_CLIENT_CB_DATA *)p_data;
        switch (p_bas_cb_data->cb_type)
        {
        case BAS_CLIENT_CB_TYPE_DISC_STATE:
            switch (p_bas_cb_data->cb_content.disc_state)
            {
            case DISC_BAS_DONE:
                /* Discovery BAS procedure successfully done. */
                APP_PRINT_INFO0("app_client_callback: discover bas procedure done");
                break;
            case DISC_BAS_FAILED:
                /* Discovery Request failed. */
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
/** @} */ /* End of group SCATTERNET_CLIENT_CALLBACK */
/** @defgroup  SCATTERNET_SEVER_CALLBACK Profile Server Callback Event Handler
    * @brief Handle profile server callback event
    * @{
    */
/**
    * @brief    All the BT Profile service callback events are handled in this function
    * @note     Then the event handling function shall be called according to the
    *           service_id
    * @param    service_id  Profile service ID
    * @param    p_data      Pointer to callback data
    * @return   T_APP_RESULT, which indicates the function call is successful or not
    * @retval   APP_RESULT_SUCCESS  Function run successfully
    * @retval   others              Function run failed, and return number indicates the reason
    */
T_APP_RESULT app_profile_callback(T_SERVER_ID service_id, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    if (service_id == SERVICE_PROFILE_GENERAL_ID)
    {
        T_SERVER_APP_CB_DATA *p_param = (T_SERVER_APP_CB_DATA *)p_data;
        switch (p_param->eventId)
        {
        case PROFILE_EVT_SRV_REG_COMPLETE:// srv register result event.
            APP_PRINT_INFO1("PROFILE_EVT_SRV_REG_COMPLETE: result %d",
                            p_param->event_data.service_reg_result);
            break;

        case PROFILE_EVT_SEND_DATA_COMPLETE:
            APP_PRINT_INFO5("PROFILE_EVT_SEND_DATA_COMPLETE: conn_id %d, cause 0x%x, service_id %d, attrib_idx 0x%x, credits %d",
                            p_param->event_data.send_data_result.conn_id,
                            p_param->event_data.send_data_result.cause,
                            p_param->event_data.send_data_result.service_id,
                            p_param->event_data.send_data_result.attrib_idx,
                            p_param->event_data.send_data_result.credits);
            if (p_param->event_data.send_data_result.cause == GAP_SUCCESS)
            {
                APP_PRINT_INFO0("PROFILE_EVT_SEND_DATA_COMPLETE success");
            }
            else
            {
                APP_PRINT_ERROR0("PROFILE_EVT_SEND_DATA_COMPLETE failed");
            }
            break;

        default:
            break;
        }
    }
    else  if (service_id == simp_srv_id)
    {
        TSIMP_CALLBACK_DATA *p_simp_cb_data = (TSIMP_CALLBACK_DATA *)p_data;
        switch (p_simp_cb_data->msg_type)
        {
        case SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION:
            {
                switch (p_simp_cb_data->msg_data.notification_indification_index)
                {
                case SIMP_NOTIFY_INDICATE_V3_ENABLE:
                    {
                        APP_PRINT_INFO0("SIMP_NOTIFY_INDICATE_V3_ENABLE");
                    }
                    break;

                case SIMP_NOTIFY_INDICATE_V3_DISABLE:
                    {
                        APP_PRINT_INFO0("SIMP_NOTIFY_INDICATE_V3_DISABLE");
                    }
                    break;
                case SIMP_NOTIFY_INDICATE_V4_ENABLE:
                    {
                        APP_PRINT_INFO0("SIMP_NOTIFY_INDICATE_V4_ENABLE");
                    }
                    break;
                case SIMP_NOTIFY_INDICATE_V4_DISABLE:
                    {
                        APP_PRINT_INFO0("SIMP_NOTIFY_INDICATE_V4_DISABLE");
                    }
                    break;
                default:
                    break;
                }
            }
            break;

        case SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE:
            {
                if (p_simp_cb_data->msg_data.read_value_index == SIMP_READ_V1)
                {
                    uint8_t value[2] = {0x01, 0x02};
                    APP_PRINT_INFO0("SIMP_READ_V1");
                    simp_ble_service_set_parameter(SIMPLE_BLE_SERVICE_PARAM_V1_READ_CHAR_VAL, 2, &value);
                }
            }
            break;
        case SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE:
            {
                switch (p_simp_cb_data->msg_data.write.opcode)
                {
                case SIMP_WRITE_V2:
                    {
                        APP_PRINT_INFO2("SIMP_WRITE_V2: write type %d, len %d", p_simp_cb_data->msg_data.write.write_type,
                                        p_simp_cb_data->msg_data.write.len);
                    }
                    break;
                default:
                    break;
                }
            }
            break;

        default:
            break;
        }
    }
    else if (service_id == bas_srv_id)
    {
        T_BAS_CALLBACK_DATA *p_bas_cb_data = (T_BAS_CALLBACK_DATA *)p_data;
        switch (p_bas_cb_data->msg_type)
        {
        case SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION:
            {
                switch (p_bas_cb_data->msg_data.notification_indification_index)
                {
                case BAS_NOTIFY_BATTERY_LEVEL_ENABLE:
                    {
                        APP_PRINT_INFO0("BAS_NOTIFY_BATTERY_LEVEL_ENABLE");
                    }
                    break;

                case BAS_NOTIFY_BATTERY_LEVEL_DISABLE:
                    {
                        APP_PRINT_INFO0("BAS_NOTIFY_BATTERY_LEVEL_DISABLE");
                    }
                    break;
                default:
                    break;
                }
            }
            break;

        case SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE:
            {
                if (p_bas_cb_data->msg_data.read_value_index == BAS_READ_BATTERY_LEVEL)
                {
                    uint8_t battery_level = 90;
                    APP_PRINT_INFO1("BAS_READ_BATTERY_LEVEL: battery_level %d", battery_level);
                    bas_set_parameter(BAS_PARAM_BATTERY_LEVEL, 1, &battery_level);
                }
            }
            break;


        default:
            break;
        }
    }

    return app_result;
}
/** @} */ /* End of group SCATTERNET_SEVER_CALLBACK */
/** @} */ /* End of group SCATTERNET_APP */

