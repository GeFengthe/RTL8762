/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      light_app.c
* @brief     Smart mesh demo application
* @details
* @author    bill
* @date      2015-11-12
* @version   v0.1
* *********************************************************************************************************
*/

#include <string.h>
#include <stdio.h>
#include <app_msg.h>
#include <trace.h>
#include <gap_scan.h>
#include <gap.h>
#include <gap_msg.h>
#include <gap_adv.h>
#include <gap_bond_le.h>
#include <profile_server.h>

#include "rtl876x.h"
#include "trace.h"
#include "gap_wrapper.h"
#include "mesh_api.h"
#include "light_app.h"
#include "light_effect_app.h"
#include "sha256.h"
#include "dfu_server.h"
#include "dfu_client.h"
#include "ais.h"
#include "mp_cmd_parse.h"
#include "user_data.h"
#include "otp_config.h"
#include "mem_config.h"
#include "light_swtimer.h"
#include "light_ctl_server_app.h"
#include "ali_ctl_light_app.h"
#include "vendor_timer.h"
#include "system_clock.h"
#include "light_cwrgb_app.h"
#if ALI_AIS_SUPPORT
#include "ais.h"
#include "dis.h"
T_SERVER_ID dis_server_id;
#endif

#define NEW_AUTH_METHOD  1
/**
 * @brief  Application Link control block definition.
 */
typedef struct
{
    T_GAP_CONN_STATE        conn_state;          /**< Connection state. */
    T_GAP_REMOTE_ADDR_TYPE  bd_type;             /**< remote BD type*/
    uint8_t                 bd_addr[GAP_BD_ADDR_LEN]; /**< remote BD */
} T_APP_LINK;

T_GAP_DEV_STATE gap_dev_state = {0, 0, 0, 0};                 /**< GAP device state */
T_APP_LINK app_link_table[APP_MAX_LINKS];

static timer_data_t temp;
timer_data_t *pTimer_adjust = &temp;
uint8_t weekday_list[] = {MON, TUE, WED, THU, FRI, SAT, SUN};

void app_handle_gap_msg(T_IO_MSG *p_gap_msg);

void timer_data_init(void)
{
    temp.index = 0xFF;
    temp.unix_time = 0xFFFFFFFF;
    temp.on_off = GENERIC_OFF;
}

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
    switch (msg_type)
    {
    case IO_MSG_TYPE_BT_STATUS:
        app_handle_gap_msg(&io_msg);
        break;
    case DFU_SERVER_TIMEOUT_MSG:
        dfu_server_adv_send();
        break;
    case AIS_SERVER_TIMEOUT_MSG:
        ais_server_timeout_handle(io_msg.u.buf);
        break;
    case IO_MSG_TYPE_UART:
        {
            /* We handle user command informations from Data UART in this branch. */
            uint8_t data = io_msg.subtype;
            mp_cmd_parse(&data, sizeof(data));
        }
        break;
    case IO_MSG_TYPE_RTC:
        switch (io_msg.subtype)
        {
        case IO_MSG_TIMER_ALARM:
            pTimer_adjust = (timer_data_t *)io_msg.u.buf;
            DBG_DIRECT("set timer to %x, current time is %x", pTimer_adjust->unix_time,
                       generic_data_current.unix_time);
            timer_data_init();
            break;

        case IO_MSG_TIMER_RWS:
            // All timers are handled in this branch, which can be replaced by customized codes

            //Local time:
            local_time.minutes++;
            if (local_time.minutes == 60)
            {
                local_time.minutes = 0;
                local_time.hour++;
                if (local_time.hour == 24)
                {
                    local_time.hour = 0;
                    local_time.weekday = (local_time.weekday + 1) % 7;
                }
            }
            generic_data_current.unix_time += 60;
            DBG_DIRECT("local_time: %2d/%2d/%2d day%d %2d : %2d", local_time.year, local_time.month,
                       local_time.day, local_time.weekday + 1,
                       local_time.hour, local_time.minutes);

            //Update system time daily
            if ((generic_data_current.unix_time - sys_clk_update) >= DAY)
            {
                send_update_request();
            }

            //Execute timing switch, publish state and delete finished timer
            if (generic_data_current.unix_time >= timer_list[0].unix_time)
            {
                clear_timer(timer_list[0].index);
                generic_data_current.generic_on_off = timer_list[0].on_off ? GENERIC_ON : GENERIC_OFF;
                T_IO_MSG on_off_msg;
                on_off_msg.type = IO_MSG_TYPE_GPIO;
                on_off_msg.subtype = timer_list[0].on_off ? GENERIC_ON : GENERIC_OFF;
                app_send_msg_to_apptask(&on_off_msg);
                timer_list[0].index = 0xFF;
                timer_list[0].unix_time = 0xFFFFFFFF;
                timer_list[0].on_off = GENERIC_OFF;
                timer_sort();
            }

            //Execute if periodic timer exists
            else if (is_prd_exist())
            {
                if (weekday_list[local_time.weekday] & get_prd_schedule())
                {
                    if ((local_time.minutes + local_time.hour * 60) == get_prd_unix_time())
                    {
                        generic_data_current.generic_on_off = get_prd_state() ? GENERIC_ON : GENERIC_OFF;
                        T_IO_MSG on_off_msg;
                        on_off_msg.type = IO_MSG_TYPE_GPIO;
                        on_off_msg.subtype = generic_data_current.generic_on_off;
                        app_send_msg_to_apptask(&on_off_msg);
                    }
                }
            }
            break;
        default:
            break;
        }
        break;
#if (ROM_WATCH_DOG_ENABLE == 1)
    case IO_MSG_TYPE_RESET_WDG_TIMER:
        {
            //APP_PRINT_INFO0("[WDG] Watch Dog Rset Timer");
            DBG_DIRECT("[WDG] Watch Dog Rset Timer");
            WDG_Restart();
        }
        break;
#endif
    case IO_MSG_TYPE_TIMER:
        light_handle_sw_timer_msg(&io_msg);
        break;
    case IO_MSG_TYPE_GPIO:
        switch (io_msg.subtype)
        {
        case GENERIC_ON:
            //TODO: open power strip here
            light_cw_turn_on();
            light_rgb_turn_on();
            handle_pub_evt(io_msg.subtype);
            break;
        case GENERIC_OFF:
            //TODO: shut down power strip here
            light_cw_turn_off();
            light_rgb_turn_off();
            handle_pub_evt(io_msg.subtype);
            break;
        default:
            break;
        }
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
            APP_PRINT_INFO0("GAP stack ready");
            uint8_t bt_addr[6];
            gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);
            APP_PRINT_INFO6("bt addr 0x%x:%x:%x:%x:%x:%x",
                            bt_addr[5], bt_addr[4], bt_addr[3],
                            bt_addr[2], bt_addr[1], bt_addr[0]);
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
            memset(&app_link_table[conn_id], 0, sizeof(T_APP_LINK));
            dfu_client_disconnect_cb(conn_id);
            dfu_server_disconnect_cb(conn_id);
            if (mesh_node.node_state == UNPROV_DEVICE)
            {
                light_gap_state_disconnect();
            }
        }
        break;
    case GAP_CONN_STATE_CONNECTING:
        {
            dfu_client_handle_connecting(conn_id);
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
#if F_BT_LE_5_0_SET_PHY_SUPPORT
            {
                uint8_t tx_phy;
                uint8_t rx_phy;
                le_get_conn_param(GAP_PARAM_CONN_RX_PHY_TYPE, &rx_phy, conn_id);
                le_get_conn_param(GAP_PARAM_CONN_TX_PHY_TYPE, &tx_phy, conn_id);
                APP_PRINT_INFO2("GAP_CONN_STATE_CONNECTED: tx_phy %d, rx_phy %d", tx_phy, rx_phy);
            }
#endif
            le_set_data_len(conn_id, 251, 2120);
            dfu_client_handle_connect(conn_id);
            if (mesh_node.node_state == UNPROV_DEVICE)
            {
                light_gap_state_connected();
            }
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
                APP_PRINT_INFO0("app_handle_authen_state_evt: GAP_AUTHEN_STATE_COMPLETE pair success");

            }
            else
            {
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

    APP_PRINT_TRACE1("app_handle_gap_msg: sub_type %d", p_gap_msg->subtype);
    mesh_inner_msg_t mesh_inner_msg;
    mesh_inner_msg.type = MESH_BT_STATUS_UPDATE;
    mesh_inner_msg.sub_type = p_gap_msg->subtype;
    mesh_inner_msg.parm = p_gap_msg->u.param;
    gap_sched_handle_bt_status_msg(&mesh_inner_msg);
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
        }
        break;

    case GAP_MSG_LE_BOND_USER_CONFIRMATION:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_user_conf.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO2("GAP_MSG_LE_BOND_USER_CONFIRMATION: conn_id %d, passkey %d",
                            conn_id, display_value);
            //le_bond_user_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_INPUT:
        {
            //uint32_t passkey = 888888;
            conn_id = gap_msg.msg_data.gap_bond_passkey_input.conn_id;
            APP_PRINT_INFO2("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d, key_press %d",
                            conn_id, gap_msg.msg_data.gap_bond_passkey_input.key_press);
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
        APP_PRINT_ERROR1("app_handle_gap_msg: unknown sub_type %d", p_gap_msg->subtype);
        break;
    }
}

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
        gap_sched_handle_adv_report(p_data->p_le_scan_info);
#if DFU_AUTO_BETWEEN_DEVICES
        dfu_client_handle_adv_pkt(p_data->p_le_scan_info);
#endif
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
        gap_sched_adv_params_set_done();
        break;
    case GAP_MSG_LE_VENDOR_ONE_SHOT_ADV:
        APP_PRINT_INFO1("GAP_MSG_LE_VENDOR_ONE_SHOT_ADV: cause 0x%x",
                        p_data->le_cause.cause);
        gap_sched_adv_done(GAP_SCHED_ADV_END_TYPE_SUCCESS);
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
    else if (client_id == dfu_client_id)
    {
        dfu_cb_msg_t *pmsg = (dfu_cb_msg_t *)p_data;
        switch (pmsg->type)
        {
        case DFU_CB_START:
            light_dfu_client_start();
            break;
        case DFU_CB_FAIL:
            light_dfu_client_fail();
            break;
        case DFU_CB_END:
            light_dfu_client_end();
            break;
        default:
            break;
        }
    }
    return result;
}

#if ALI_AIS_SUPPORT
/******************************************************************
 * @fn          app_dis_srv_cb
 * @brief      DIS service callbacks are handled in this function.
 * @param    p_data  - pointer to callback data
 * @return     T_APP_RESULT
 */
static T_APP_RESULT app_dis_srv_cb(T_DIS_CALLBACK_DATA *p_data)
{
    T_APP_RESULT cb_result = APP_RESULT_SUCCESS;

    switch (p_data->msg_type)
    {
    case SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE:
        {
            if (p_data->msg_data.read_value_index == DIS_READ_MANU_NAME_INDEX)
            {
                const uint8_t DISManufacturerName[] = "Realtek BT";
                dis_set_parameter(DIS_PARAM_MANUFACTURER_NAME,
                                  sizeof(DISManufacturerName) - 1,
                                  (void *)DISManufacturerName);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_MODEL_NUM_INDEX)
            {
                const uint8_t DISModelNumber[] = "Model Nbr 0.9";
                dis_set_parameter(DIS_PARAM_MODEL_NUMBER,
                                  sizeof(DISModelNumber) - 1,
                                  (void *)DISModelNumber);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_SERIAL_NUM_INDEX)
            {
                const uint8_t DISSerialNumber[] = "RTKBeeSerialNum";
                dis_set_parameter(DIS_PARAM_SERIAL_NUMBER,
                                  sizeof(DISSerialNumber) - 1,
                                  (void *)DISSerialNumber);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_HARDWARE_REV_INDEX)
            {
                const uint8_t DISHardwareRev[] = "RTKBeeHardwareRev";
                dis_set_parameter(DIS_PARAM_HARDWARE_REVISION,
                                  sizeof(DISHardwareRev) - 1,
                                  (void *)DISHardwareRev);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_FIRMWARE_REV_INDEX)
            {
                const uint8_t DISFirmwareRev[] = "RTKBeeFirmwareRev";
                dis_set_parameter(DIS_PARAM_FIRMWARE_REVISION,
                                  sizeof(DISFirmwareRev) - 1,
                                  (void *)DISFirmwareRev);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_SOFTWARE_REV_INDEX)
            {
                const uint32_t DISSoftwareRev = ALI_VERSION_ID;
                dis_set_parameter(DIS_PARAM_SOFTWARE_REVISION,
                                  sizeof(DISSoftwareRev),
                                  (void *)&DISSoftwareRev);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_SYSTEM_ID_INDEX)
            {
                const uint8_t DISSystemID[DIS_SYSTEM_ID_LENGTH] = {0, 1, 2, 0, 0, 3, 4, 5};
                dis_set_parameter(DIS_PARAM_SYSTEM_ID,
                                  sizeof(DISSystemID),
                                  (void *)DISSystemID);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_IEEE_CERT_STR_INDEX)
            {
                const uint8_t DISIEEEDataList[] = "RTKBeeIEEEDatalist";
                dis_set_parameter(DIS_PARAM_IEEE_DATA_LIST,
                                  sizeof(DISIEEEDataList) - 1,
                                  (void *)DISIEEEDataList);
            }
            else if (p_data->msg_data.read_value_index == DIS_READ_PNP_ID_INDEX)
            {
                /* TODO: need to conform this info with Androind team */
                uint16_t version = 0x03; //VERSION_BUILD;

                uint8_t DISPnpID[DIS_PNP_ID_LENGTH] = {0x01, 0x5D, 0x00, 0x01, 0x00, (uint8_t)version, (uint8_t)(version >> 8)}; //VID_005D&PID_0001?

                dis_set_parameter(DIS_PARAM_PNP_ID,
                                  sizeof(DISPnpID),
                                  DISPnpID);
            }
        }
        break;
    default:
        break;
    }

    return cb_result;
}
#endif

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
    else if (service_id == dfu_server_id)
    {
        dfu_cb_msg_t *pmsg = (dfu_cb_msg_t *)p_data;
        switch (pmsg->type)
        {
        case DFU_CB_START:
            light_dfu_server_start();
            break;
        case DFU_CB_FAIL:
            light_dfu_server_fail();
            break;
        case DFU_CB_END:
            light_dfu_server_end();
            break;
        default:
            break;
        }
    }
#if ALI_AIS_SUPPORT
    else if (service_id == dis_server_id)
    {
        app_result = app_dis_srv_cb((T_DIS_CALLBACK_DATA *)p_data);
    }
#endif
    return app_result;
}
void prov_cap_oob_calc(uint32_t pid, char *secret, uint8_t *random, uint8_t *auth)
{
    char data[8 + 1 + 12 + 1  + 32 + 1 + 32];
    uint8_t auth_data[SHA256_DIGEST_LENGTH]; // Only use 16 bytes
    sprintf(data, "%08x", pid);
    data[8] = ',';
    uint8_t bt_addr[6];
    gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);

    sprintf(data + 8 + 1, "%02x%02x%02x%02x%02x%02x", bt_addr[5], bt_addr[4], bt_addr[3], bt_addr[2],
            bt_addr[1], bt_addr[0]);
    data[21] = ',';
    memcpy(data + 22, secret, 32);
    data[54] = ',';
    for (int i = 0; i < 16; i++)
    {
        sprintf(data + 55 + i * 2, "%02x", random[i]);
    }

    printi("sha256 input: %s", TRACE_STRING(data));
    SHA256_CTX sha_ctx;
    SHA256_Init(&sha_ctx);
    SHA256_Update(&sha_ctx, data, sizeof(data));
    SHA256_Final(&sha_ctx, auth_data);
    printi("sha256 output: %b", TRACE_BINARY(SHA256_DIGEST_LENGTH, auth_data));
    memcpy(auth, auth_data, SHA256_DIGEST_LENGTH);
}
/******************************************************************
 * @fn      prov_cb
 * @brief   Provisioning callbacks are handled in this function.
 *
 * @param   cb_data  -  @ref TProvisioningCbData
 * @return  the operation result
 */
void light_start_pub_stat(void);
bool prov_cb(prov_cb_type_t cb_type, prov_cb_data_t cb_data)
{
    APP_PRINT_INFO1("prov_cb: type = %d", cb_type);

    switch (cb_type)
    {
    case PROV_CB_TYPE_PB_ADV_LINK_STATE:
        switch (cb_data.pb_generic_cb_type)
        {
        case PB_GENERIC_CB_LINK_OPENED:
            if (mesh_node.node_state == UNPROV_DEVICE)
            {
                light_prov_link_open();
            }
            break;
        case PB_GENERIC_CB_LINK_OPEN_FAILED:
            break;
        case PB_GENERIC_CB_LINK_CLOSED:
            if (mesh_node.node_state == UNPROV_DEVICE)
            {
                light_prov_link_closed();
            }
            break;
        default:
            break;
        }
        break;
    case PROV_CB_TYPE_UNPROV:
        unprov_timer_init();
        light_prov_unprov();
        break;
    case PROV_CB_TYPE_START:
        light_prov_start();
        break;
    case PROV_CB_TYPE_PUBLIC_KEY:
        {
            uint8_t public_key[64] = {0xf4, 0x65, 0xe4, 0x3f, 0xf2, 0x3d, 0x3f, 0x1b, 0x9d, 0xc7, 0xdf, 0xc0, 0x4d, 0xa8, 0x75, 0x81, 0x84, 0xdb, 0xc9, 0x66, 0x20, 0x47, 0x96, 0xec, 0xcf, 0x0d, 0x6c, 0xf5, 0xe1, 0x65, 0x00, 0xcc, 0x02, 0x01, 0xd0, 0x48, 0xbc, 0xbb, 0xd8, 0x99, 0xee, 0xef, 0xc4, 0x24, 0x16, 0x4e, 0x33, 0xc2, 0x01, 0xc2, 0xb0, 0x10, 0xca, 0x6b, 0x4d, 0x43, 0xa8, 0xa1, 0x55, 0xca, 0xd8, 0xec, 0xb2, 0x79};
            uint8_t private_key[32] = {0x52, 0x9a, 0xa0, 0x67, 0x0d, 0x72, 0xcd, 0x64, 0x97, 0x50, 0x2e, 0xd4, 0x73, 0x50, 0x2b, 0x03, 0x7e, 0x88, 0x03, 0xb5, 0xc6, 0x08, 0x29, 0xa5, 0xa3, 0xca, 0xa2, 0x19, 0x50, 0x55, 0x30, 0xba};
            prov_params_set(PROV_PARAMS_PUBLIC_KEY, public_key, sizeof(public_key));
            prov_params_set(PROV_PARAMS_PRIVATE_KEY, private_key, sizeof(private_key));
            APP_PRINT_INFO0("prov_cb: Please show the public key to the provisioner");
        }
        break;
    case PROV_CB_TYPE_AUTH_DATA:
        {
#if NEW_AUTH_METHOD
            uint8_t random[16];
            plt_rand(random, sizeof(random));
            prov_auth_random_set(random);
            char data[8 + 1 + 12 + 1  + 32 + 1 + 32];
#else
            char data[8 + 1 + 12 + 1 + 32 + 1];
#endif
            prov_start_p pprov_start = cb_data.pprov_start;
            char secret[32];
            uint32_t product_id;
            if (user_data_contains_ali_data())
            {
                user_data_read_ali_secret_key((uint8_t *)secret);
                product_id = user_data_read_ali_product_id();
            }
            else
            {
                memcpy(secret, ALI_SECRET_KEY, 32);
                product_id = ALI_PRODUCT_ID;
            }

            uint8_t auth_data[SHA256_DIGEST_LENGTH]; // Only use 16 bytes
            sprintf(data, "%08x", product_id);
            data[8] = ',';
            uint8_t bt_addr[6];
            gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);
            sprintf(data + 8 + 1, "%02x%02x%02x%02x%02x%02x", bt_addr[5], bt_addr[4], bt_addr[3], bt_addr[2],
                    bt_addr[1], bt_addr[0]);
            data[21] = ',';
            memcpy(data + 22, secret, 32);
#if NEW_AUTH_METHOD
            data[54] = ',';
            for (int i = 0; i < 16; i++)
            {
                sprintf(data + 55 + i * 2, "%02x", random[i]);
            }
#else
            data[54] = 0; // for log print debug
#endif
            /*
             * sample: product id = 0x293e2, bt addr = AB:CD:F0:F1:F2:F3, secret = "atFY1tGDCo4MQSVCGVDqtti3PvBI5WXb"
             * input: "000293e2,abcdf0f1f2f3,atFY1tGDCo4MQSVCGVDqtti3PvBI5WXb"
             * ouput: 8e-e2-17-bc-02-a5-ab-66-6d-d2-ce-39-5d-f7-20-55-85-4a-f2-7e-c5-c0-45-d9-2a-48-48-99-74-3a-dc-9f
             * authvalue: 8e-e2-17-bc-02-a5-ab-66-6d-d2-ce-39-5d-f7-20-55
             */
            printi("sha256 input: %s", TRACE_STRING(data));
            SHA256_CTX sha_ctx;
            SHA256_Init(&sha_ctx);
#if NEW_AUTH_METHOD
            SHA256_Update(&sha_ctx, data, sizeof(data));
#else
            SHA256_Update(&sha_ctx, data, sizeof(data) - 1);
#endif
            SHA256_Final(&sha_ctx, auth_data);
            printi("sha256 output: %b", TRACE_BINARY(SHA256_DIGEST_LENGTH, auth_data));
            switch (pprov_start->auth_method)
            {
            case PROV_AUTH_METHOD_STATIC_OOB:
                prov_auth_value_set(auth_data, 16);
                APP_PRINT_ERROR1("prov_cb: Please exchange the oob data(%b) with the provisioner", TRACE_BINARY(16,
                                 auth_data));
                break;
            case PROV_AUTH_METHOD_OUTPUT_OOB:
                //prov_auth_value_set(auth_data, pprov_start->auth_size.output_oob_size);
                APP_PRINT_INFO2("prov_cb: Please output the oob data to the provisioner, output size = %d, action = %d",
                                pprov_start->auth_size.output_oob_size, pprov_start->auth_action.output_oob_action);
                break;
            case PROV_AUTH_METHOD_INPUT_OOB:
                //prov_auth_value_set(auth_data, pprov_start->auth_size.input_oob_size);
                APP_PRINT_INFO2("prov_cb: Please input the oob data provided by the provisioner, input size = %d, action = %d",
                                pprov_start->auth_size.input_oob_size, pprov_start->auth_action.input_oob_action);
                break;
            default:
                break;
            }
        }
        break;
    case PROV_CB_TYPE_RANDOM:
        {
#if NEW_AUTH_METHOD
            uint8_t random[16];
            memcpy(random, cb_data.pprov_random->rand, 16);
            char secret[32];
            uint32_t product_id;
            if (user_data_contains_ali_data())
            {
                user_data_read_ali_secret_key((uint8_t *)secret);
                product_id = user_data_read_ali_product_id();
            }
            else
            {
                memcpy(secret, ALI_SECRET_KEY, 32);
                product_id = ALI_PRODUCT_ID;
            }

            char data[8 + 1 + 12 + 1  + 32 + 1 + 32];
            uint8_t auth_data[SHA256_DIGEST_LENGTH]; // Only use 16 bytes
            sprintf(data, "%08x", product_id);
            data[8] = ',';
            uint8_t bt_addr[6];
            gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);

            sprintf(data + 8 + 1, "%02x%02x%02x%02x%02x%02x", bt_addr[5], bt_addr[4], bt_addr[3], bt_addr[2],
                    bt_addr[1], bt_addr[0]);
            data[21] = ',';
            memcpy(data + 22, secret, 32);
            data[54] = ',';
            for (int i = 0; i < 16; i++)
            {
                sprintf(data + 55 + i * 2, "%02x", random[i]);
            }

            printi("sha256 input: %s", TRACE_STRING(data));
            SHA256_CTX sha_ctx;
            SHA256_Init(&sha_ctx);
            SHA256_Update(&sha_ctx, data, sizeof(data));
            SHA256_Final(&sha_ctx, auth_data);
            printi("sha256 output: %b", TRACE_BINARY(SHA256_DIGEST_LENGTH, auth_data));
            prov_auth_value_change(auth_data, 16);
#endif
        }
        break;
    case PROV_CB_TYPE_COMPLETE:
        {
#include "dfu_updater_app.h"
#if DFU_UPDATER_SUPPORT_POWER_OFF_GO_ON
            dfu_updater_clear();
#endif
            light_prov_complete();
            mesh_model_bind_all_key();
            light_ctl_server_models_sub();
            light_ctl_server_models_pub();
            light_ctl_server_models_pub_start();
            power_on_message();
        }
        break;
    case PROV_CB_TYPE_FAIL:
        //light_lighten_green();
        break;
    case PROV_CB_TYPE_PROV:
        power_on_message();
        /* stack ready */
        break;
    default:
        break;
    }
    return true;
}

void fn_cb(uint8_t frnd_index, fn_cb_type_t type, uint16_t lpn_addr)
{
    char *string[] = {"establishing with lpn 0x%04x\r\n", "no poll from 0x%04x\r\n", "established with lpn 0x%04x\r\n", "lpn 0x%04x lost\r\n"};
    data_uart_debug(string[type], lpn_addr);
}
