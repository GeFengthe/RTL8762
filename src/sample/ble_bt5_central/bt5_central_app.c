/**
*****************************************************************************************
*     Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      central_app.c
   * @brief     This file handles BLE BT5 central application routines.
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
#include <string.h>
#include "trace.h"
#include "gap.h"
#include "gap_bond_le.h"
#include "gap_msg.h"
#include "gap_vendor.h"
#include "gap_ext_scan.h"
#include "user_cmd.h"
#include "user_cmd_parse.h"
#include "app_msg.h"
#include "link_mgr.h"
#include "bt5_central_app.h"

/** @defgroup  BT5_CENTRAL_APP BT5 Central Application
    * @brief This file handles BLE BT5 central application routines.
    * @{
    */
/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @defgroup  BT5_CENTRAL_GAP_MSG GAP Message Handler
    * @brief Handle GAP Message
    * @{
    */
T_GAP_DEV_STATE gap_dev_state = {0, 0, 0, 0};                 /**< GAP device state */
T_GAP_CONN_STATE gap_conn_state = GAP_CONN_STATE_DISCONNECTED; /**< GAP connection state */
#if APP_RECOMBINE_ADV_DATA
uint8_t *p_temp_data = NULL;     /**< The location to save ext adv data */
#endif

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
#if LE_CODED_PHY_S8
            le_ae_coding_scheme(GAP_AE_CODING_SCHEME_S8);
#endif
        }
    }

    if (gap_dev_state.gap_scan_state != new_state.gap_scan_state)
    {
        if (new_state.gap_scan_state == GAP_SCAN_STATE_IDLE)
        {
            APP_PRINT_INFO0("GAP scan stop");
            data_uart_print("GAP scan stop\r\n");

            /* Reset flags of recombining advertising data when stop scanning */
#if APP_RECOMBINE_ADV_DATA
            ext_adv_data->flag = false;
            ext_adv_data->data_len = 0;
            memset(fail_bd_addr, 0, GAP_BD_ADDR_LEN);
#endif
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
    APP_PRINT_INFO4("app_handle_conn_state_evt: conn_id %d, conn_state(%d -> %d), disc_cause 0x%x",
                    conn_id, gap_conn_state, new_state, disc_cause);

    switch (new_state)
    {
    case GAP_CONN_STATE_DISCONNECTED:
        {
            if ((disc_cause != (HCI_ERR | HCI_ERR_REMOTE_USER_TERMINATE))
                && (disc_cause != (HCI_ERR | HCI_ERR_LOCAL_HOST_TERMINATE)))
            {
                APP_PRINT_ERROR1("app_handle_conn_state_evt: connection lost cause 0x%x", disc_cause);
            }
        }
        data_uart_print("Disconnect conn_id %d, dis_cause 0x%x\r\n", conn_id, disc_cause);
        break;

    case GAP_CONN_STATE_CONNECTED:
        {
            uint8_t tx_phy;
            uint8_t rx_phy;
            uint16_t conn_interval;
            uint16_t conn_latency;
            uint16_t conn_supervision_timeout;
            uint8_t  remote_bd[6];
            T_GAP_REMOTE_ADDR_TYPE remote_bd_type;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            le_get_conn_addr(conn_id, remote_bd, (uint8_t *)&remote_bd_type);
            APP_PRINT_INFO5("GAP_CONN_STATE_CONNECTED:remote_bd %s, remote_addr_type %d, conn_interval 0x%x, conn_latency 0x%x, conn_supervision_timeout 0x%x",
                            TRACE_BDADDR(remote_bd), remote_bd_type,
                            conn_interval, conn_latency, conn_supervision_timeout);

            le_get_conn_param(GAP_PARAM_CONN_TX_PHY_TYPE, &tx_phy, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_RX_PHY_TYPE, &rx_phy, conn_id);
            data_uart_print("Connected success conn_id %d, tx_phy %d, rx_phy %d\r\n", conn_id, tx_phy, rx_phy);
        }
        break;

    default:
        break;
    }
    gap_conn_state = new_state;
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
 *           sub_type of T_IO_MSG
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
            le_bond_user_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_INPUT:
        {
            uint32_t passkey = 888888;
            conn_id = gap_msg.msg_data.gap_bond_passkey_input.conn_id;
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d", conn_id);
            data_uart_print("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d\r\n", conn_id);
            le_bond_passkey_input_confirm(conn_id, passkey, GAP_CFM_CAUSE_ACCEPT);
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
/** @} */ /* End of group BT5_CENTRAL_GAP_MSG */

/** @defgroup  BT5_CENTRAL_GAP_CALLBACK GAP Callback Event Handler
    * @brief Handle GAP callback event
    * @{
    */
#if APP_RECOMBINE_ADV_DATA
/**
  * @brief Check the length of received advertising data.
  * @param[in] data_len      Length of recieved advertising data.
  * @retval true:   Lenth of received advertising data is less than or equal to APP_MAX_EXT_ADV_TOTAL_LEN.
            false:  Lenth of received advertising data is greater than APP_MAX_EXT_ADV_TOTAL_LEN.
  */
bool app_check_adv_data_len(void)
{
    if (ext_adv_data->data_len > APP_MAX_EXT_ADV_TOTAL_LEN)
    {
        APP_PRINT_ERROR2("app_check_adv_data_len: The length of received advertising data is %d, exceeds APP_MAX_EXT_ADV_TOTAL_LEN %d",
                         ext_adv_data->data_len, APP_MAX_EXT_ADV_TOTAL_LEN);
        /* Update failed recombination parameters for next recombination. */
        fail_event_type = ext_adv_data->event_type;
        memcpy(fail_bd_addr, ext_adv_data->bd_addr, GAP_BD_ADDR_LEN);
        /* Reset recombination parameters. */
        ext_adv_data->data_len = 0;
        ext_adv_data->flag = false;
        return false;
    }
    else
    {
        return true;
    }
}

/**
  * @brief Handle callback GAP_MSG_LE_EXT_ADV_REPORT_INFO to recombine advertising data.
  * @param[in] event_type    Advertisement event type.
  * @param[in] data_status   Data status @ref T_GAP_EXT_ADV_EVT_DATA_STATUS.
  * @param[in] bd_addr       Peer device address.
  * @param[in] data_len      Length of data.
  * @param[in] p_data        Advertising data.
  * @retval void
  */
void app_handle_ext_adv_report(uint16_t event_type, T_GAP_EXT_ADV_EVT_DATA_STATUS data_status,
                               uint8_t *bd_addr, uint8_t data_len, uint8_t *p_data)
{
    APP_PRINT_INFO2("app_handle_ext_adv_report: Old ext_adv_data->flag is %d, data status is 0x%x",
                    ext_adv_data->flag, data_status);

    /* Recombine advertising data from one device. */
    switch (data_status)
    {
    case GAP_EXT_ADV_EVT_DATA_STATUS_COMPLETE:
        /* Advertising data is complete. */
        if (ext_adv_data->flag)
        {
            if ((memcmp(ext_adv_data->bd_addr, bd_addr, GAP_BD_ADDR_LEN) == 0) &&
                (ext_adv_data->event_type == event_type))
            {
                /* The advertising report is the expected report. */
                if ((memcmp(fail_bd_addr, bd_addr, GAP_BD_ADDR_LEN) == 0) && (fail_event_type == event_type))
                {
                    APP_PRINT_ERROR2("app_handle_ext_adv_report: The advertising data is destroyed by last failed recombination, last failed bd_addr %s, last failed event type 0x%x",
                                     TRACE_BDADDR(fail_bd_addr), fail_event_type);
                }
                else
                {
                    /* Update length of advertising data, and check whether the length exceeds APP_MAX_EXT_ADV_TOTAL_LEN. */
                    ext_adv_data->data_len += data_len;
                    if (!(app_check_adv_data_len()))
                    {
                        return;
                    }
                    memcpy(p_temp_data, p_data, data_len);
                    STREAM_SKIP_LEN(p_temp_data, data_len);
                    APP_PRINT_INFO3("app_handle_ext_adv_report: Data from bd_addr %s is complete, event type is 0x%x, total data length is %d",
                                    TRACE_BDADDR(ext_adv_data->bd_addr), ext_adv_data->event_type, ext_adv_data->data_len);
                    if (ext_adv_data->data_len > 5)
                    {
                        APP_PRINT_INFO5("app_handle_ext_adv_report: First five datas are 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
                                        ext_adv_data->p_data[0], ext_adv_data->p_data[1], ext_adv_data->p_data[2], ext_adv_data->p_data[3],
                                        ext_adv_data->p_data[4]);
                        APP_PRINT_INFO5("app_handle_ext_adv_report: Last five datas are 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
                                        ext_adv_data->p_data[ext_adv_data->data_len - 5], ext_adv_data->p_data[ext_adv_data->data_len - 4],
                                        ext_adv_data->p_data[ext_adv_data->data_len - 3],
                                        ext_adv_data->p_data[ext_adv_data->data_len - 2], ext_adv_data->p_data[ext_adv_data->data_len - 1]);
                    }
                }
                /*  Reset recombination parameters. */
                memset(fail_bd_addr, 0, GAP_BD_ADDR_LEN);
                ext_adv_data->data_len = 0;
                ext_adv_data->flag = false;
            }
            else
            {
                APP_PRINT_INFO2("app_handle_ext_adv_report: New data which is not saved from bd_addr %s is complete, total data length is %d",
                                TRACE_BDADDR(bd_addr), data_len);
            }
        }
        else
        {
            if ((memcmp(fail_bd_addr, bd_addr, GAP_BD_ADDR_LEN) == 0) && (fail_event_type == event_type))
            {
                APP_PRINT_ERROR2("app_handle_ext_adv_report: The advertising data is destroyed by last failed recombination, last failed bd_addr %s, last failed event type 0x%x",
                                 TRACE_BDADDR(fail_bd_addr), fail_event_type);
            }
            else
            {
                /* Update recombination parameters, and check whether the length exceeds APP_MAX_EXT_ADV_TOTAL_LEN. */
                ext_adv_data->flag = true;
                ext_adv_data->data_len = data_len;
                memcpy(ext_adv_data->bd_addr, bd_addr, GAP_BD_ADDR_LEN);
                ext_adv_data->event_type = event_type;
                if (!(app_check_adv_data_len()))
                {
                    return;
                }
                p_temp_data = ext_adv_data->p_data;
                memcpy(p_temp_data, p_data, data_len);
                STREAM_SKIP_LEN(p_temp_data, data_len);
                APP_PRINT_INFO2("app_handle_ext_adv_report: Data from bd_addr %s is complete, total data length is %d",
                                TRACE_BDADDR(ext_adv_data->bd_addr), data_len);
                ext_adv_data->data_len = 0;
                ext_adv_data->flag = false;
            }
            memset(fail_bd_addr, 0, GAP_BD_ADDR_LEN);
        }
        break;

    case GAP_EXT_ADV_EVT_DATA_STATUS_MORE:
        /* Advertising data is incomplete, more data to come. */
        if (ext_adv_data->flag)
        {
            if ((memcmp(ext_adv_data->bd_addr, bd_addr, GAP_BD_ADDR_LEN) == 0) &&
                (ext_adv_data->event_type == event_type))
            {
                /* The advertising report is the expected report. */
                ext_adv_data->data_len += data_len;
                if (!(app_check_adv_data_len()))
                {
                    return;
                }
                memcpy(p_temp_data, p_data, data_len);
                STREAM_SKIP_LEN(p_temp_data, data_len);
                APP_PRINT_INFO2("app_handle_ext_adv_report: Continuation data from bd_addr %s is incomplete, data length is %d, and waiting more data",
                                TRACE_BDADDR(ext_adv_data->bd_addr), ext_adv_data->data_len);
            }
            else
            {
                APP_PRINT_INFO2("app_handle_ext_adv_report: New data which is not saved from bd_addr %s is incomplete, data length is %d",
                                TRACE_BDADDR(bd_addr), data_len);
            }
        }
        else
        {
            /* Update recombination parameters for first fragment, and check whether the length exceeds APP_MAX_EXT_ADV_TOTAL_LEN. */
            ext_adv_data->flag = true;
            ext_adv_data->data_len = data_len;
            memcpy(ext_adv_data->bd_addr, bd_addr, GAP_BD_ADDR_LEN);
            ext_adv_data->event_type = event_type;
            if (!(app_check_adv_data_len()))
            {
                return;
            }
            p_temp_data = ext_adv_data->p_data;
            memcpy(p_temp_data, p_data, data_len);
            STREAM_SKIP_LEN(p_temp_data, data_len);
            APP_PRINT_INFO2("app_handle_ext_adv_report:First Data from bd_addr %s, data length is %d, and waiting more data",
                            TRACE_BDADDR(ext_adv_data->bd_addr), ext_adv_data->data_len);
        }
        break;

    case GAP_EXT_ADV_EVT_DATA_STATUS_TRUNCATED:
        /* Advertising data is incomplete, data truncated, no more to come. */
        if (ext_adv_data->flag && (memcmp(ext_adv_data->bd_addr, bd_addr, GAP_BD_ADDR_LEN) == 0) &&
            (ext_adv_data->event_type == event_type))
        {
            /* If data is truncated, reset recombination parameters. */
            ext_adv_data->data_len += data_len;
            if (!(app_check_adv_data_len()))
            {
                return;
            }
            memcpy(p_temp_data, p_data, data_len);
            STREAM_SKIP_LEN(p_temp_data, data_len);
            APP_PRINT_INFO3("app_handle_ext_adv_report: Continuation data from bd_addr %s is truncated, event type is 0x%x, data length is %d, and no more data to come",
                            TRACE_BDADDR(ext_adv_data->bd_addr), ext_adv_data->event_type, ext_adv_data->data_len);
            memset(fail_bd_addr, 0, GAP_BD_ADDR_LEN);
            ext_adv_data->data_len = 0;
            ext_adv_data->flag = false;
        }
        break;

    case GAP_EXT_ADV_EVT_DATA_STATUS_RFU:
        /* Reserved for future use. */
        break;

    default:
        APP_PRINT_ERROR1("app_handle_ext_adv_report: unhandled data_status 0x%x", data_status);
        break;
    }
    APP_PRINT_INFO1("app_handle_ext_adv_report: New ext_adv_data->flag is %d", ext_adv_data->flag);
}
#endif

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

    APP_PRINT_TRACE1("app_gap_callback: cb_type = 0x%x", cb_type);

    switch (cb_type)
    {
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

    case GAP_MSG_LE_EXT_ADV_REPORT_INFO:
        APP_PRINT_INFO6("GAP_MSG_LE_EXT_ADV_REPORT_INFO:connectable %d, scannable %d, direct %d, scan response %d, legacy %d, data status 0x%x",
                        p_data->p_le_ext_adv_report_info->event_type & GAP_EXT_ADV_REPORT_BIT_CONNECTABLE_ADV,
                        p_data->p_le_ext_adv_report_info->event_type & GAP_EXT_ADV_REPORT_BIT_SCANNABLE_ADV,
                        p_data->p_le_ext_adv_report_info->event_type & GAP_EXT_ADV_REPORT_BIT_DIRECTED_ADV,
                        p_data->p_le_ext_adv_report_info->event_type & GAP_EXT_ADV_REPORT_BIT_SCAN_RESPONSE,
                        p_data->p_le_ext_adv_report_info->event_type & GAP_EXT_ADV_REPORT_BIT_USE_LEGACY_ADV,
                        p_data->p_le_ext_adv_report_info->data_status);
        APP_PRINT_INFO5("GAP_MSG_LE_EXT_ADV_REPORT_INFO:event_type 0x%x, bd_addr %s, addr_type %d, rssi %d, data_len %d",
                        p_data->p_le_ext_adv_report_info->event_type,
                        TRACE_BDADDR(p_data->p_le_ext_adv_report_info->bd_addr),
                        p_data->p_le_ext_adv_report_info->addr_type,
                        p_data->p_le_ext_adv_report_info->rssi,
                        p_data->p_le_ext_adv_report_info->data_len);
        APP_PRINT_INFO5("GAP_MSG_LE_EXT_ADV_REPORT_INFO:primary_phy %d, secondary_phy %d, adv_sid %d, tx_power %d, peri_adv_interval %d",
                        p_data->p_le_ext_adv_report_info->primary_phy,
                        p_data->p_le_ext_adv_report_info->secondary_phy,
                        p_data->p_le_ext_adv_report_info->adv_sid,
                        p_data->p_le_ext_adv_report_info->tx_power,
                        p_data->p_le_ext_adv_report_info->peri_adv_interval);
        APP_PRINT_INFO2("GAP_MSG_LE_EXT_ADV_REPORT_INFO:direct_addr_type 0x%x, direct_addr %s",
                        p_data->p_le_ext_adv_report_info->direct_addr_type,
                        TRACE_BDADDR(p_data->p_le_ext_adv_report_info->direct_addr));

        link_mgr_add_device(p_data->p_le_ext_adv_report_info->bd_addr,
                            p_data->p_le_ext_adv_report_info->addr_type);

#if APP_RECOMBINE_ADV_DATA
        if (!(p_data->p_le_ext_adv_report_info->event_type & GAP_EXT_ADV_REPORT_BIT_USE_LEGACY_ADV))
        {
            /* If the advertisement uses extended advertising PDUs, recombine advertising data. */
            app_handle_ext_adv_report(p_data->p_le_ext_adv_report_info->event_type,
                                      p_data->p_le_ext_adv_report_info->data_status, p_data->p_le_ext_adv_report_info->bd_addr,
                                      p_data->p_le_ext_adv_report_info->data_len, p_data->p_le_ext_adv_report_info->p_data);
        }
#endif
        break;

    case GAP_MSG_LE_AE_CODING_SCHEME:
        APP_PRINT_INFO1("GAP_MSG_LE_AE_CODING_SCHEME: cause 0x%x",
                        p_data->le_cause.cause);
        break;

    default:
        APP_PRINT_ERROR1("app_gap_callback: unhandled cb_type 0x%x", cb_type);
        break;
    }
    return result;
}
/** @} */ /* End of group BT5_CENTRAL_GAP_CALLBACK */
/** @} */ /* End of group BT5_CENTRAL_APP */
