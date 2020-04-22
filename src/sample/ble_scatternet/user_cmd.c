/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      user_cmd.c
   * @brief     User defined test commands.
   * @details  User command interfaces.
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
#include <trace.h>
#include <profile_client.h>
#include <gap_bond_le.h>
#include <gap_scan.h>
#include <user_cmd.h>
#include <gap.h>
#include <gap_conn_le.h>
#include <link_mgr.h>
#include <simple_ble_client.h>
#include <gaps_client.h>
#include <bas_client.h>
#include <gap_adv.h>
#include <scatternet_app.h>
#include <simple_ble_service.h>
#include <bas.h>
#include <gatt_builtin_services.h>


/** @defgroup  SCATTERNET_CMD Scatternet User Command
    * @brief This file handles Scatternet User Command.
    * @{
    */
/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @brief User command interface data, used to parse the commands from Data UART. */
T_USER_CMD_IF    user_cmd_if;

/*============================================================================*
 *                              Functions
 *============================================================================*/
/**
 * @brief Read the RSSI value
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "rssiread",
        "rssiread [conn_id]\n\r",
        "Read the RSSI value\n\r",
        cmd_rssiread
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_rssiread(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    T_GAP_CAUSE cause;
    cause = le_read_rssi(conn_id);

    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Show all devices connecting status
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "showcon",
        "showcon\n\r",
        "Show all devices connecting status\n\r",
        cmd_showcon
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_showcon(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id;
    T_GAP_CONN_INFO conn_info;
    for (conn_id = 0; conn_id < APP_MAX_LINKS; conn_id++)
    {
        if (le_get_conn_info(conn_id, &conn_info))
        {
            data_uart_print("ShowCon conn_id %d state 0x%x role %d\r\n", conn_id,
                            conn_info.conn_state, conn_info.role);
            data_uart_print("RemoteBd = [%02x:%02x:%02x:%02x:%02x:%02x] type = %d\r\n",
                            conn_info.remote_bd[5], conn_info.remote_bd[4],
                            conn_info.remote_bd[3], conn_info.remote_bd[2],
                            conn_info.remote_bd[1], conn_info.remote_bd[0],
                            conn_info.remote_bd_type);
        }
    }
    data_uart_print("active link num %d,  idle link num %d\r\n",
                    le_get_active_link_num(), le_get_idle_link_num());
    return (RESULT_SUCESS);
}

/**
 * @brief LE connection param update request
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "conupdreq",
        "conupdreq [conn_id] [interval_min] [interval_max] [latency] [supervision_timeout]\n\r",
        "LE connection param update request\r\n\
        sample: conupdreq 0 0x30 0x40 0 500\n\r",
        cmd_conupdreq
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_conupdreq(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    uint8_t  conn_id = p_parse_value->dw_param[0];
    uint16_t conn_interval_min = p_parse_value->dw_param[1];
    uint16_t conn_interval_max = p_parse_value->dw_param[2];
    uint16_t conn_latency = p_parse_value->dw_param[3];
    uint16_t supervision_timeout = p_parse_value->dw_param[4];


    cause = le_update_conn_param(conn_id,
                                 conn_interval_min,
                                 conn_interval_max,
                                 conn_latency,
                                 supervision_timeout,
                                 2 * (conn_interval_min - 1),
                                 2 * (conn_interval_max - 1)
                                );
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Disconnect to remote device
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "disc",
        "disc [conn_id]\n\r",
        "Disconnect to remote device\n\r",
        cmd_disc
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_disc(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    T_GAP_CAUSE cause;
    cause = le_disconnect(conn_id);
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Using connected device address to modify whitelist
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "wl",
        "wl [op] [conn_id]\n\r",
        "Using connected device address to modify whitelist \r\n\
        [op]: 0-(clear), 1-(add), 2-(remove)\r\n\
        sample: wl 0 1\n\r",
        cmd_wl
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_wl(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    T_GAP_WHITE_LIST_OP op = (T_GAP_WHITE_LIST_OP)p_parse_value->dw_param[0];
    uint8_t conn_id = p_parse_value->dw_param[1];
    T_GAP_CONN_INFO conn_info;

    if (op == GAP_WHITE_LIST_OP_CLEAR)
    {
        cause = le_modify_white_list(GAP_WHITE_LIST_OP_CLEAR, NULL, GAP_REMOTE_ADDR_LE_PUBLIC);
        return (T_USER_CMD_PARSE_RESULT)cause;
    }

    if (le_get_conn_info(conn_id, &conn_info))
    {
        cause = le_modify_white_list(op, conn_info.remote_bd,
                                     (T_GAP_REMOTE_ADDR_TYPE)conn_info.remote_bd_type);
        return (T_USER_CMD_PARSE_RESULT)cause;
    }
    else
    {
        return RESULT_CMD_ERR_PARAM;
    }
}
/**
 * @brief Config authentication mode
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "authmode",
        "authmode [auth_flags] [io_cap] [sec_enable] [oob_enable]\n\r",
        "Config authentication mode\r\n\
        [auth_flags]:authentication req bit field: bit0-(bonding), bit2-(MITM), bit3-(SC)\r\n\
        [io_cap]:set io Capabilities: 0-(display only), 1-(display yes/no), 2-(keyboard noly), 3-(no IO), 4-(keyboard display)\r\n\
        [sec_enable]:Start smp pairing procedure when connected: 0-(disable), 1-(enable)\r\n\
        [oob_enable]:Enable oob flag: 0-(disable), 1-(enable)\r\n\
        sample: authmode 0x5 2 1 0\n\r",
        cmd_authmode
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_authmode(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    uint8_t  auth_pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint16_t auth_flags = GAP_AUTHEN_BIT_BONDING_FLAG;
    uint8_t  auth_io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8_t  oob_enable = false;
    uint8_t  auth_sec_req_enable = false;
    uint16_t auth_sec_req_flags = GAP_AUTHEN_BIT_BONDING_FLAG;

    if (p_parse_value->param_count > 0)
    {
        auth_flags = p_parse_value->dw_param[0];
        auth_sec_req_flags = p_parse_value->dw_param[0];
    }
    if (p_parse_value->param_count > 1)
    {
        auth_io_cap = p_parse_value->dw_param[1];
    }
    if (p_parse_value->param_count > 2)
    {
        auth_sec_req_enable = p_parse_value->dw_param[2];
    }
    if (p_parse_value->param_count > 3)
    {
        oob_enable = p_parse_value->dw_param[3];
    }
    gap_set_param(GAP_PARAM_BOND_PAIRING_MODE, sizeof(auth_pair_mode), &auth_pair_mode);
    gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(auth_flags), &auth_flags);
    gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(auth_io_cap), &auth_io_cap);
    gap_set_param(GAP_PARAM_BOND_OOB_ENABLED, sizeof(uint8_t), &oob_enable);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_ENABLE, sizeof(auth_sec_req_enable), &auth_sec_req_enable);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_REQUIREMENT, sizeof(auth_sec_req_flags),
                      &auth_sec_req_flags);
    cause = gap_set_pairable_mode();
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Send authentication request
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "sauth",
        "sauth [conn_id]\n\r",
        "Send authentication request\n\r",
        cmd_sauth
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_sauth(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    T_GAP_CAUSE cause;
    cause = le_bond_pair(conn_id);
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Send user confirmation when show GAP_MSG_LE_BOND_USER_CONFIRMATION
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "userconf",
        "userconf [conn_id] [conf]\n\r",
        "Send user confirmation when show GAP_MSG_LE_BOND_USER_CONFIRMATION\r\n\
        [conf]: 0-(Reject), 1-(Accept)\r\n\
        sample: userconf 0 1\n\r",
        cmd_userconf
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_userconf(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    T_GAP_CFM_CAUSE confirm = GAP_CFM_CAUSE_ACCEPT;
    T_GAP_CAUSE cause;
    if (p_parse_value->dw_param[1] == 0)
    {
        confirm = GAP_CFM_CAUSE_REJECT;
    }
    cause = le_bond_user_confirm(conn_id, confirm);
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Input passkey when show GAP_MSG_LE_BOND_PASSKEY_INPUT
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "authkey",
        "authkey [conn_id] [passkey]\n\r",
        "Input passkey when show GAP_MSG_LE_BOND_PASSKEY_INPUT\r\n\
        [passkey]: 0 - 999999\r\n\
        sample: authkey 0 123456\n\r",
        cmd_authkey
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_authkey(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    uint32_t passcode = p_parse_value->dw_param[1];
    T_GAP_CAUSE cause;
    T_GAP_CFM_CAUSE confirm = GAP_CFM_CAUSE_ACCEPT;
    if (passcode > GAP_PASSCODE_MAX)
    {
        confirm = GAP_CFM_CAUSE_REJECT;
    }
    cause = le_bond_passkey_input_confirm(conn_id, passcode,
                                          confirm);
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Clear all bonded devices information
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "bondclear",
        "bondclear\n\r",
        "Clear all bonded devices information\n\r",
        cmd_bondclear
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_bondclear(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    le_bond_clear_all_keys();
    return (RESULT_SUCESS);
}

/**
 * @brief Get all Bonded devices information
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "bondinfo",
        "bondinfo\n\r",
        "Get all Bonded devices information\n\r",
        cmd_bondinfo
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_bondinfo(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t i;
    T_LE_KEY_ENTRY *p_entry;
    for (i = 0; i < bond_storage_num; i++)
    {
        p_entry = le_find_key_entry_by_idx(i);
        if (p_entry != NULL)
        {
            data_uart_print("bond_dev[%d]: bd 0x%02x%02x%02x%02x%02x%02x, addr_type %d, flags 0x%x\r\n",
                            p_entry->idx,
                            p_entry->remote_bd.addr[5],
                            p_entry->remote_bd.addr[4],
                            p_entry->remote_bd.addr[3],
                            p_entry->remote_bd.addr[2],
                            p_entry->remote_bd.addr[1],
                            p_entry->remote_bd.addr[0],
                            p_entry->remote_bd.remote_bd_type,
                            p_entry->flags);
        }
    }

    return (RESULT_SUCESS);
}

#if F_BT_AIRPLANE_MODE_SUPPORT
/**
 * @brief Write airplane mode
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "wairplane",
        "wairplane [mode]\n\r",
        "Write airplane mode\r\n\
        [mode]: 0-(normal), 1-(airplane)\n\r",
        cmd_wairplane
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_wairplane(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    uint8_t mode = p_parse_value->dw_param[0];
    cause = gap_write_airplan_mode(mode);
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Read airplane mode
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "rairplane",
        "rairplane\n\r",
        "Read airplane mode\n\r",
        cmd_rairplane
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_rairplane(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    cause = gap_read_airplan_mode();
    return (T_USER_CMD_PARSE_RESULT)cause;
}
#endif
/************************** Central only *************************************/
/**
 * @brief Start scan
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "scan",
        "scan [filter_policy] [filter_duplicate]\n\r",
        "Start scan\r\n\
        [filter_policy]: 0-(any), 1-(whitelist), 2-(any RPA), 3-(whitelist RPA) \r\n\
        [filter_duplicate]: 0-(disable), 1-(enable) \n\r",
        cmd_scan
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_scan(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    uint8_t scan_filter_policy = GAP_SCAN_FILTER_ANY;
    uint8_t scan_filter_duplicate = GAP_SCAN_FILTER_DUPLICATE_ENABLE;

    if (p_parse_value->param_count > 0)
    {
        scan_filter_policy = p_parse_value->dw_param[0];
    }
    if (p_parse_value->param_count > 1)
    {
        scan_filter_duplicate = p_parse_value->dw_param[1];
    }

    link_mgr_clear_device_list();
    le_scan_set_param(GAP_PARAM_SCAN_FILTER_POLICY, sizeof(scan_filter_policy),
                      &scan_filter_policy);
    le_scan_set_param(GAP_PARAM_SCAN_FILTER_DUPLICATES, sizeof(scan_filter_duplicate),
                      &scan_filter_duplicate);
    cause = le_scan_start();
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Stop scan
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "stopscan",
        "stopscan\n\r",
        "Stop scan\n\r",
        cmd_stopscan
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_stopscan(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    cause = le_scan_stop();
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Show scan dev list: filter simple ble service
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "showdev",
        "showdev\n\r",
        "Show scan dev list: filter simple ble service\n\r",
        cmd_showdev
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_showdev(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t i;
    data_uart_print("Advertising and Scan response: filter uuid = 0xA00A dev list\r\n");
    for (i = 0; i < dev_list_count; i++)
    {
        data_uart_print("RemoteBd[%d] = [%02x:%02x:%02x:%02x:%02x:%02x] type = %d\r\n",
                        i,
                        dev_list[i].bd_addr[5], dev_list[i].bd_addr[4],
                        dev_list[i].bd_addr[3], dev_list[i].bd_addr[2],
                        dev_list[i].bd_addr[1], dev_list[i].bd_addr[0],
                        dev_list[i].bd_type);
        if (dev_list[i].bd_type == GAP_REMOTE_ADDR_LE_RANDOM)
        {
            uint8_t addr = dev_list[i].bd_addr[5] & RANDOM_ADDR_MASK;
            if (addr == RANDOM_ADDR_MASK_STATIC)
            {
                data_uart_print("Static Random Addr\r\n");
            }
            else if (addr == RANDOM_ADDR_MASK_RESOLVABLE)
            {
                data_uart_print("Resolv Random Addr\r\n");
            }
            else if (addr == RANDOM_ADDR_MASK_NON_RESOLVABLE)
            {
                data_uart_print("Non-resolv Random Addr\r\n");
            }
            else
            {
                data_uart_print("Unknown Random Addr\r\n");
            }
        }
    }

    return (RESULT_SUCESS);
}

/**
 * @brief Connect to remote device: use address
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "con",
        "con [BD0] [BD1] [BD2] [BD3] [BD4] [BD5] [addr_type]\n\r",
        "Connect to remote device: use address\r\n\
        [BD0] [BD1] [BD2] [BD3] [BD4] [BD5]: remote device address\r\n\
        [addr_type]: 0-(public), 1-(random)\r\n\
        sample: con x11 x22 x33 x44 x55 x66 0 \n\r",
        cmd_con
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_con(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    uint8_t addr[6] = {0};
    uint8_t addr_len;
    uint8_t addr_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    T_GAP_LE_CONN_REQ_PARAM conn_req_param;
#if F_BT_LE_USE_STATIC_RANDOM_ADDR
    T_GAP_LOCAL_ADDR_TYPE local_addr_type = GAP_LOCAL_ADDR_LE_RANDOM;
#else
    T_GAP_LOCAL_ADDR_TYPE local_addr_type = GAP_LOCAL_ADDR_LE_PUBLIC;
#endif
    conn_req_param.scan_interval = 0x10;
    conn_req_param.scan_window = 0x10;
    conn_req_param.conn_interval_min = 80;
    conn_req_param.conn_interval_max = 80;
    conn_req_param.conn_latency = 0;
    conn_req_param.supv_tout = 1000;
    conn_req_param.ce_len_min = 2 * (conn_req_param.conn_interval_min - 1);
    conn_req_param.ce_len_max = 2 * (conn_req_param.conn_interval_max - 1);
    le_set_conn_param(GAP_CONN_PARAM_1M, &conn_req_param);

    for (addr_len = 0; addr_len < GAP_BD_ADDR_LEN; addr_len++)
    {
        addr[addr_len] = p_parse_value->dw_param[GAP_BD_ADDR_LEN - addr_len - 1];
    }
    if (p_parse_value->param_count >= 7)
    {
        addr_type = p_parse_value->dw_param[6];
    }

    cause = le_connect(GAP_PHYS_CONN_INIT_1M_BIT, addr, (T_GAP_REMOTE_ADDR_TYPE)addr_type,
                       local_addr_type,
                       1000);

    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Connect to remote device: use showdev to show idx
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "condev",
        "condev [idx]\n\r",
        "Connect to remote device: use showdev to show idx\r\n\
        [idx]: use cmd showdev to show idx before use this cmd\r\n\
        sample: condev 0\n\r",
        cmd_condev
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_condev(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t dev_idx = p_parse_value->dw_param[0];
    if (dev_idx < dev_list_count)
    {
        T_GAP_CAUSE cause;
        T_GAP_LE_CONN_REQ_PARAM conn_req_param;
#if F_BT_LE_USE_STATIC_RANDOM_ADDR
        T_GAP_LOCAL_ADDR_TYPE local_addr_type = GAP_LOCAL_ADDR_LE_RANDOM;
#else
        T_GAP_LOCAL_ADDR_TYPE local_addr_type = GAP_LOCAL_ADDR_LE_PUBLIC;
#endif

        conn_req_param.scan_interval = 0x10;
        conn_req_param.scan_window = 0x10;
        conn_req_param.conn_interval_min = 80;
        conn_req_param.conn_interval_max = 80;
        conn_req_param.conn_latency = 0;
        conn_req_param.supv_tout = 1000;
        conn_req_param.ce_len_min = 2 * (conn_req_param.conn_interval_min - 1);
        conn_req_param.ce_len_max = 2 * (conn_req_param.conn_interval_max - 1);
        le_set_conn_param(GAP_CONN_PARAM_1M, &conn_req_param);
        cause = le_connect(GAP_PHYS_CONN_INIT_1M_BIT,
                           dev_list[dev_idx].bd_addr,
                           (T_GAP_REMOTE_ADDR_TYPE)dev_list[dev_idx].bd_type,
                           local_addr_type,
                           1000);
        return (T_USER_CMD_PARSE_RESULT)cause;
    }
    else
    {
        return RESULT_ERR;
    }
}

/**
 * @brief Set Host Channel Classification
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "chanclassset",
        "chanclassset [idx0] [idx1] [idx2] [idx3] [idx4]\n\r",
        "Set Host Channel Classification\r\n\
        [idx0] [idx1] [idx2] [idx3] [idx4]: channel bit map\r\n\
        sample: chanclassset xff xff x3f xff x00\n\r",
        cmd_chanclassset
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_chanclassset(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    uint8_t chan_map[5] = {0};
    uint8_t i;
    for (i = 0; i < 5; i++)
    {
        chan_map[i] = (uint8_t)p_parse_value->dw_param[i];
    }

    chan_map[4] = chan_map[4] & 0x1F;

    cause = le_set_host_chann_classif(chan_map);

    return (T_USER_CMD_PARSE_RESULT)cause;
}
/************************** GATT client *************************************/
/**
 * @brief Service discovery, discover all primary services
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "srvdis",
        "srvdis [conn_id]\n\r",
        "Service discovery, discover all primary services\n\r",
        cmd_srvdis
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_srvdis(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    T_GAP_CAUSE cause;

    cause = client_all_primary_srv_discovery(conn_id, CLIENT_PROFILE_GENERAL_ID);
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Start discovery gap service
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "gapdis",
        "gapdis [conn_id]\n\r",
        "Start discovery gap service\n\r",
        cmd_gapdis
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_gapdis(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    bool ret = gaps_start_discovery(conn_id);
    if (ret)
    {
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}
/**
 * @brief Read GAP service characteristic value
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "gapread",
        "gapread [conn_id] [type]\n\r",
        "Read GAP service characteristic value\r\n\
        [type]: 0-(read device name), 1-(read appearance)\r\n\
        simple: gapread 0 0\n\r",
        cmd_gapread
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_gapread(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    T_GAPS_READ_TYPE read_type = (T_GAPS_READ_TYPE)p_parse_value->dw_param[1];
    bool ret = gaps_read(conn_id, read_type);
    if (ret)
    {
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}

/**
 * @brief List GAP service handle cache
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "gaphdl",
        "gaphdl [conn_id]\n\r",
        "List GAP service handle cache\n\r",
        cmd_gaphdl
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_gaphdl(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    uint16_t hdl_cache[HDL_GAPS_CACHE_LEN];
    uint8_t hdl_idx;
    bool ret = gaps_get_hdl_cache(conn_id, hdl_cache,
                                  sizeof(uint16_t) * HDL_GAPS_CACHE_LEN);

    if (ret)
    {
        for (hdl_idx = HDL_GAPS_SRV_START; hdl_idx < HDL_GAPS_CACHE_LEN; hdl_idx++)
        {
            data_uart_print("-->Index %d -- Handle 0x%x\r\n", hdl_idx, hdl_cache[hdl_idx]);
        }
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}

/**
 * @brief Start discovery simple ble service
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "simpdis",
        "simpdis [conn_id]\n\r",
        "Start discovery simple ble service\n\r",
        cmd_simpdis
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_simpdis(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    bool ret = simp_ble_client_start_discovery(conn_id);
    if (ret)
    {
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}
/**
 * @brief Read simple ble service characteristic and descriptor value
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "simpread",
        "simpread [conn_id] [type] [pattern]\n\r",
        "Read simple ble service characteristic and descriptor value\r\n\
        [type]: 0-(read v1), 1-(v3 cccd), 2-(v4 cccd)\r\n\
        [pattern]: 0-(read by handle), 1-(read by uuid)\r\n\
        sample: simpread 0 1 0 \n\r",
        cmd_simpread
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_simpread(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    /* Indicate which char to be read. */
    uint8_t conn_id = p_parse_value->dw_param[0];
    bool ret;
    T_SIMP_READ_TYPE read_type = (T_SIMP_READ_TYPE)p_parse_value->dw_param[1];
    /* Read by handle or UUID, 1--by UUID, 0--by handle. */
    uint8_t read_pattern = (uint8_t)p_parse_value->dw_param[2];

    if (read_pattern)
    {
        ret = simp_ble_client_read_by_uuid(conn_id, read_type);
    }
    else
    {
        ret = simp_ble_client_read_by_handle(conn_id, read_type);
    }

    if (ret)
    {
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}

/**
 * @brief Config simple ble service client characteristic configuration descriptor value
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "simpcccd",
        "simpcccd [conn_id] [type] [enable]\n\r",
        "Config simple ble service client characteristic configuration descriptor value\r\n\
        [type]: 0-(v3 notify), 1-(v4 indication)\r\n\
        [enable]: 0-(disable), 1-(enable) \r\n\
        sample: simpcccd 0 1 1\n\r",
        cmd_simpcccd
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_simpcccd(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    /* Indicate which char CCCD command. */
    bool ret = false;
    uint8_t conn_id = p_parse_value->dw_param[0];
    uint8_t type = p_parse_value->dw_param[1];
    /* Enable or disable, 1--enable, 0--disable. */
    bool cccd_data = (bool)p_parse_value->dw_param[2];

    switch (type)
    {
    case 0:/* V3 Notify char notif enable/disable. */
        ret = simp_ble_client_set_v3_notify(conn_id, cccd_data);
        break;
    case 1:/* V4 Indicate char indicate enable/disable. */
        ret = simp_ble_client_set_v4_ind(conn_id, cccd_data);
        break;
    default:
        break;
    }

    if (ret)
    {
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}

/**
 * @brief Write simple ble service V2 characteristic value
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "simpwritev2",
        "simpwritev2 [conn_id] [type] [len]\n\r",
        "Write simple ble service V2 characteristic value\r\n\
        [type]: 1-(write request), 2-(write command) \r\n\
        [len]: type=1 len range:0-270, type=2 len range: 0-(mtu-3)\r\n\
        sample: simpwritev2 0 1 10\n\r",
        cmd_simpwritev2
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_simpwritev2(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    T_GATT_WRITE_TYPE write_type = (T_GATT_WRITE_TYPE)p_parse_value->dw_param[1];
    uint8_t data[270];
    uint16_t length = 270;
    uint16_t i;
    for (i = 0; i < 270; i++)
    {
        data[i] = i;
    }
    if (write_type > GATT_WRITE_TYPE_CMD)
    {
        return (RESULT_ERR);
    }
    if (p_parse_value->param_count > 2)
    {
        length = p_parse_value->dw_param[2];
        if (length > 270)
        {
            length = 270;
        }
    }
    if (simp_ble_client_write_v2_char(conn_id, length, data, write_type))
    {
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}

/**
 * @brief List simple ble service handle cache
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "simphdl",
        "simphdl [conn_id]\n\r",
        "List simple ble service handle cache\n\r",
        cmd_simphdl
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_simphdl(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    uint16_t hdl_cache[HDL_SIMBLE_CACHE_LEN];
    uint8_t hdl_idx;
    bool ret = simp_ble_client_get_hdl_cache(conn_id, hdl_cache,
                                             sizeof(uint16_t) * HDL_SIMBLE_CACHE_LEN);

    if (ret)
    {
        for (hdl_idx = HDL_SIMBLE_SRV_START; hdl_idx < HDL_SIMBLE_CACHE_LEN; hdl_idx++)
        {
            data_uart_print("-->Index %d -- Handle 0x%x\r\n", hdl_idx, hdl_cache[hdl_idx]);
        }
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}
/**
 * @brief Start discovery battery service
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "basdis",
        "basdis [conn_id]\n\r",
        "Start discovery battery service\n\r",
        cmd_basdis
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_basdis(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    bool ret = bas_start_discovery(conn_id);
    if (ret)
    {
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}
/**
 * @brief Read battery service characteristic and descriptor value
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "basread",
        "basread [conn_id] [type]\n\r",
        "Read battery service characteristic and descriptor value\r\n\
        [type]: 0-(battery level value), 1-(battery cccd)\r\n\
        sample: basread 0 1 \n\r",
        cmd_basread
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_basread(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    bool ret = false;
    if (p_parse_value->dw_param[1] == 0)
    {
        ret = bas_read_battery_level(conn_id);
    }
    else if (p_parse_value->dw_param[1] == 1)
    {
        ret = bas_read_notify(conn_id);
    }

    if (ret)
    {
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}

/**
 * @brief Config battery service client characteristic configuration descriptor value
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "bascccd",
        "bascccd [conn_id] [notify]\n\r",
        "Config battery service client characteristic configuration descriptor value\r\n\
        [notify]: 0-(disable), 1-(enable)\n\r",
        cmd_bascccd
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_bascccd(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    bool notify = p_parse_value->dw_param[1];
    bool ret;
    ret = bas_set_notify(conn_id, notify);

    if (ret)
    {
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}

/**
 * @brief List battery service handle cache
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "bashdl",
        "bashdl [conn_id]\n\r",
        "List battery service handle cache\n\r",
        cmd_bashdl
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_bashdl(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    uint16_t hdl_cache[HDL_BAS_CACHE_LEN];
    uint8_t hdl_idx;
    bool ret = bas_get_hdl_cache(conn_id, hdl_cache,
                                 sizeof(uint16_t) * HDL_BAS_CACHE_LEN);

    if (ret)
    {
        for (hdl_idx = HDL_BAS_SRV_START; hdl_idx < HDL_BAS_CACHE_LEN; hdl_idx++)
        {
            data_uart_print("-->Index %d -- Handle 0x%x\r\n", hdl_idx, hdl_cache[hdl_idx]);
        }
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}

/************************** Peripheral Only *************************************/
/**
 * @brief Start advertising without setting advertising parameters
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "startadv",
        "startadv\n\r",
        "start advertising without setting advertising parameters\n\r",
        cmd_startadv
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_startadv(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    //advertising parameters
    T_GAP_CAUSE cause;
    cause = le_adv_start();
    return (T_USER_CMD_PARSE_RESULT)cause;
}
/**
 * @brief Stop advertising
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "stopadv",
        "stopadv\n\r",
        "Stop advertising\n\r",
        cmd_stopadv
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_stopadv(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    cause = le_adv_stop();
    return (T_USER_CMD_PARSE_RESULT)cause;
}
/**
 * @brief Start Undirected advertising
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "adv",
        "adv [adv_interval] [filter_policy]\n\r",
        "Start Undirected advertising\r\n\
        [adv_interval]: 0x0020 - 0x4000 (20ms - 10240ms, 0.625ms/step)\r\n\
        [filter_policy]: 0-(all), 1-(whitelist scan), 2-(whitelist conn), 3-(whitelist all)\r\n\
        sample: adv x40 0 \n\r",
        cmd_adv
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_adv(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_IND;
    uint8_t  adv_chann_map = GAP_ADVCHAN_ALL;
    uint8_t  adv_filter_policy = GAP_ADV_FILTER_ANY;
    uint16_t adv_int_min = 80;
    uint16_t adv_int_max = 80;

    if (p_parse_value->param_count > 0)
    {
        adv_int_min = p_parse_value->dw_param[0];
        adv_int_max = p_parse_value->dw_param[0];
    }
    if (p_parse_value->param_count > 1)
    {
        adv_filter_policy = p_parse_value->dw_param[1];
    }

    le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_evt_type), &adv_evt_type);
    le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_chann_map), &adv_chann_map);
    le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_min), &adv_int_min);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_max), &adv_int_max);

    cause = le_adv_start();
    return (T_USER_CMD_PARSE_RESULT)cause;
}
/**
 * @brief Start lower duty directed advertising
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "advld",
        "advld [BD0] [BD1] [BD2] [BD3] [BD4] [BD5]\n\r",
        "Start lower duty directed advertising\r\n\
        [BD0] [BD1] [BD2] [BD3] [BD4] [BD5]: peer address\r\n\
        sample: advld x11 x22 x33 x44 x55 x66\n\r",
        cmd_advld
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_advld(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_LDC_DIRECT_IND;
    uint8_t  adv_chann_map = GAP_ADVCHAN_ALL;
    uint16_t adv_int_min = 300;
    uint16_t adv_int_max = 320;
    uint8_t  adv_direct_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  adv_direct_addr[GAP_BD_ADDR_LEN] = {0};
    uint8_t  addr_len;

    for (addr_len = 0; addr_len < GAP_BD_ADDR_LEN; addr_len++)
    {
        adv_direct_addr[addr_len] = p_parse_value->dw_param[GAP_BD_ADDR_LEN - addr_len - 1];
    }

    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR_TYPE, sizeof(adv_direct_type), &adv_direct_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR, sizeof(adv_direct_addr), adv_direct_addr);
    le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_evt_type), &adv_evt_type);
    le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_chann_map), &adv_chann_map);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_min), &adv_int_min);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_max), &adv_int_max);

    cause = le_adv_start();

    return (T_USER_CMD_PARSE_RESULT)cause;
}
/**
 * @brief Start high duty directed advertising
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "advhd",
        "advhd [BD0] [BD1] [BD2] [BD3] [BD4] [BD5]\n\r",
        "Start high duty directed advertising\r\n\
        [BD0] [BD1] [BD2] [BD3] [BD4] [BD5]: peer address\r\n\
        sample: advhd x11 x22 x33 x44 x55 x66\n\r",
        cmd_advhd
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_advhd(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_HDC_DIRECT_IND;
    uint8_t  adv_chann_map = GAP_ADVCHAN_ALL;
    uint8_t  adv_direct_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  adv_direct_addr[GAP_BD_ADDR_LEN] = {0};
    uint8_t  addr_len;

    for (addr_len = 0; addr_len < GAP_BD_ADDR_LEN; addr_len++)
    {
        adv_direct_addr[addr_len] = p_parse_value->dw_param[GAP_BD_ADDR_LEN - addr_len - 1];
    }

    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR_TYPE, sizeof(adv_direct_type), &adv_direct_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR, sizeof(adv_direct_addr), adv_direct_addr);
    le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_evt_type), &adv_evt_type);
    le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_chann_map), &adv_chann_map);

    cause = le_adv_start();

    return (T_USER_CMD_PARSE_RESULT)cause;
}
/**
 * @brief Start scannable undirected advertising
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "advscan",
        "advscan\n\r",
        "Start scannable undirected advertising\n\r",
        cmd_advscan
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_advscan(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_SCAN_IND;
    uint8_t  adv_chann_map = GAP_ADVCHAN_ALL;
    uint8_t  adv_filter_policy = GAP_ADV_FILTER_ANY;
    uint16_t adv_int_min = 300;
    uint16_t adv_int_max = 320;

    le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_evt_type), &adv_evt_type);
    le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_chann_map), &adv_chann_map);
    le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_min), &adv_int_min);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_max), &adv_int_max);

    cause = le_adv_start();

    return (T_USER_CMD_PARSE_RESULT)cause;
}
/**
 * @brief Start non_connectable undirected advertising
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "advnonconn",
        "advnonconn\n\r",
        "Start non_connectable undirected advertising\n\r",
        cmd_advnonconn
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_advnonconn(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    T_GAP_CAUSE cause;
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_NONCONN_IND;
    uint8_t  adv_chann_map = GAP_ADVCHAN_ALL;
    uint8_t  adv_filter_policy = GAP_ADV_FILTER_ANY;
    uint16_t adv_int_min = 300;
    uint16_t adv_int_max = 320;

    le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_evt_type), &adv_evt_type);
    le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_chann_map), &adv_chann_map);
    le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_min), &adv_int_min);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_max), &adv_int_max);

    cause = le_adv_start();
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief On off slave latency
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "latency",
        "latency [conn_id] [on_off]\n\r",
        "On off slave latency\r\n\
        [on_off]: 0-(turn on the latency), 1-(turn off the latency)\r\n\
        sample: latency 0 1\n\r",
        cmd_latency
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_latency(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    bool disable = p_parse_value->dw_param[1];
    T_GAP_CAUSE cause;

    cause = le_disable_slave_latency(conn_id, disable);
    return (T_USER_CMD_PARSE_RESULT)cause;
}
/**
 * @brief Update instant passed channel map
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "epassed",
        "epassed [enable]\n\r",
        "Update instant passed channel map\r\n\
        [enable]: 0 - (disable), 1-(enable)\r\n\
        sample: epassed 1\n\r",
        cmd_epassed
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_epassed(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    /* Indicate which char to be written. */
    bool enable = p_parse_value->dw_param[0];
    T_GAP_CAUSE cause;

    cause = le_update_passed_chann_map(enable);
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/************************** GATT services *************************************/
/**
 * @brief Set GAP service Peripheral Preferred Connection Parameters Char value
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "gapprefconn",
        "gapprefconn [conn_interval_min] [conn_interval_max] [latency] [supervision_timeout]\n\r",
        "Set GAP service Peripheral Preferred Connection Parameters Char value\r\n\
        [conn_interval_min]: 0x0006 to 0x0C80(1.25ms/step)\r\n\
        [conn_interval_max]: 0x0006 to 0x0C80(1.25ms/step)\r\n\
        [latency]: 0x0000 to 0x01F3\r\n\
        [supervision_timeout]: 0x000A to 0x0C80(10ms/step)\r\n\
        sample: gapprefconn x80 x90 0 500 \n\r",
        cmd_gapprefconn
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_gapprefconn(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint16_t conn_interval_min   = p_parse_value->dw_param[0];
    uint16_t conn_interval_max   = p_parse_value->dw_param[1];
    uint16_t slave_latency       = p_parse_value->dw_param[2];
    uint16_t supervision_timeout = p_parse_value->dw_param[3];
    gaps_set_peripheral_preferred_conn_param(conn_interval_min, conn_interval_max,
                                             slave_latency, supervision_timeout);
    return (RESULT_SUCESS);
}
/**
 * @brief Send V3 notification
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "simpv3notify",
        "simpv3notify [conn_id] [len] \n\r",
        "Send V3 notification\r\n\
        [len]: 0 - (mtu-3)\n\r",
        cmd_simpv3notify
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_simpv3notify(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    uint8_t notif_val[244];
    uint16_t length = p_parse_value->dw_param[1];
    uint16_t i;

    if (length > 244)
    {
        return RESULT_ERR;
    }
    for (i = 0; i < length; i++)
    {
        notif_val[i] = i;
    }

    if (simp_ble_service_send_v3_notify(conn_id, simp_srv_id,
                                        &notif_val,
                                        length))
    {
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}
/**
 * @brief Send V4 indication
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "simpv4ind",
        "simpv4ind [conn_id] [len]\n\r",
        "Send V4 indication\r\n\
        [len]: 0 - (mtu-3)\n\r",
        cmd_simpv4ind
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_simpv4ind(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    uint8_t ind_val[244];
    uint16_t length = p_parse_value->dw_param[1];
    uint16_t i;

    if (length > 244)
    {
        return RESULT_ERR;
    }
    for (i = 0; i < length; i++)
    {
        ind_val[i] = i;
    }

    if (simp_ble_service_send_v4_indicate(conn_id, simp_srv_id,
                                          &ind_val,
                                          length))
    {
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}

/**
 * @brief Send battery level notification
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "basnotify",
        "basnotify [conn_id] [battery_level] \n\r",
        "Send battery level notification\r\n\
        [battery_level]: 0 - 100\r\n\
        sample: basnotify 0 80\n\r",
        cmd_basnotify
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_basnotify(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    uint8_t battery_level = p_parse_value->dw_param[1];

    if (bas_battery_level_value_notify(conn_id, bas_srv_id, battery_level))
    {
        return (RESULT_SUCESS);
    }
    else
    {
        return (RESULT_ERR);
    }
}
#if F_BT_LE_5_0_SET_PHY_SUPPORT
/**
 * @brief Set the PHY preferences for the connection
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "setphy",
        "setphy [conn_id] [type]\n\r",
        "Set the PHY preferences for the connection\r\n\
        [type]: 0-(1M), 1-(2M), 2-(CODED-S8), 3-(CODED-S2), 4-(tx 2M, rx 1M) \n\r",
        cmd_setphy
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_setphy(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    uint8_t all_phys;
    uint8_t tx_phys;
    uint8_t rx_phys;
    T_GAP_PHYS_OPTIONS phy_options = GAP_PHYS_OPTIONS_CODED_PREFER_S8;
    T_GAP_CAUSE cause;

    if (p_parse_value->dw_param[1] == 0)
    {
        all_phys = GAP_PHYS_PREFER_ALL;
        tx_phys = GAP_PHYS_PREFER_1M_BIT;
        rx_phys = GAP_PHYS_PREFER_1M_BIT;
    }
    else if (p_parse_value->dw_param[1] == 1)
    {
        all_phys = GAP_PHYS_PREFER_ALL;
        tx_phys = GAP_PHYS_PREFER_2M_BIT;
        rx_phys = GAP_PHYS_PREFER_2M_BIT;
    }
    else if (p_parse_value->dw_param[1] == 2)
    {
        all_phys = GAP_PHYS_PREFER_ALL;
        tx_phys = GAP_PHYS_PREFER_CODED_BIT;
        rx_phys = GAP_PHYS_PREFER_CODED_BIT;
        phy_options = GAP_PHYS_OPTIONS_CODED_PREFER_S8;
    }
    else if (p_parse_value->dw_param[1] == 3)
    {
        all_phys = GAP_PHYS_PREFER_ALL;
        tx_phys = GAP_PHYS_PREFER_CODED_BIT;
        rx_phys = GAP_PHYS_PREFER_CODED_BIT;
        phy_options = GAP_PHYS_OPTIONS_CODED_PREFER_S2;
    }
    else
    {
        all_phys = GAP_PHYS_NO_PREFER_TX_BIT;
        tx_phys = GAP_PHYS_PREFER_2M_BIT;
        rx_phys = GAP_PHYS_PREFER_1M_BIT;
    }
    cause = le_set_phy(conn_id, all_phys, tx_phys, rx_phys, phy_options);

    return (T_USER_CMD_PARSE_RESULT)cause;
}
#endif

/*----------------------------------------------------
 * command table
 * --------------------------------------------------*/
const T_USER_CMD_TABLE_ENTRY user_cmd_table[] =
{
    /************************** Common cmd *************************************/
    {
        "rssiread",
        "rssiread [conn_id]\n\r",
        "Read the RSSI value\n\r",
        cmd_rssiread
    },
    {
        "conupdreq",
        "conupdreq [conn_id] [interval_min] [interval_max] [latency] [supervision_timeout]\n\r",
        "LE connection param update request\r\n\
        sample: conupdreq 0 0x30 0x40 0 500\n\r",
        cmd_conupdreq
    },
    {
        "showcon",
        "showcon\n\r",
        "Show all devices connecting status\n\r",
        cmd_showcon
    },
    {
        "disc",
        "disc [conn_id]\n\r",
        "Disconnect to remote device\n\r",
        cmd_disc
    },
    {
        "wl",
        "wl [op] [conn_id]\n\r",
        "Using connected device address to modify whitelist \r\n\
        [op]: 0-(clear), 1-(add), 2-(remove)\r\n\
        sample: wl 0 1\n\r",
        cmd_wl
    },
    {
        "authmode",
        "authmode [auth_flags] [io_cap] [sec_enable] [oob_enable]\n\r",
        "Config authentication mode\r\n\
        [auth_flags]:authentication req bit field: bit0-(bonding), bit2-(MITM), bit3-(SC)\r\n\
        [io_cap]:set io Capabilities: 0-(display only), 1-(display yes/no), 2-(keyboard noly), 3-(no IO), 4-(keyboard display)\r\n\
        [sec_enable]:Start smp pairing procedure when connected: 0-(disable), 1-(enable)\r\n\
        [oob_enable]:Enable oob flag: 0-(disable), 1-(enable)\r\n\
        sample: authmode 0x5 2 1 0\n\r",
        cmd_authmode
    },
    {
        "sauth",
        "sauth [conn_id]\n\r",
        "Send authentication request\n\r",
        cmd_sauth
    },
    {
        "userconf",
        "userconf [conn_id] [conf]\n\r",
        "Send user confirmation when show GAP_MSG_LE_BOND_USER_CONFIRMATION\r\n\
        [conf]: 0-(Reject), 1-(Accept)\r\n\
        sample: userconf 0 1\n\r",
        cmd_userconf
    },
    {
        "authkey",
        "authkey [conn_id] [passkey]\n\r",
        "Input passkey when show GAP_MSG_LE_BOND_PASSKEY_INPUT\r\n\
        [passkey]: 0 - 999999\r\n\
        sample: authkey 0 123456\n\r",
        cmd_authkey
    },
    {
        "bondinfo",
        "bondinfo\n\r",
        "Get all Bonded devices information\n\r",
        cmd_bondinfo
    },
    {
        "bondclear",
        "bondclear\n\r",
        "Clear all bonded devices information\n\r",
        cmd_bondclear
    },
#if F_BT_AIRPLANE_MODE_SUPPORT
    {
        "wairplane",
        "wairplane [mode]\n\r",
        "Write airplane mode\r\n\
        [mode]: 0-(normal), 1-(airplane)\n\r",
        cmd_wairplane
    },
    {
        "rairplane",
        "rairplane\n\r",
        "Read airplane mode\n\r",
        cmd_rairplane
    },
#endif
    /************************** Central only *************************************/
    {
        "scan",
        "scan [filter_policy] [filter_duplicate]\n\r",
        "Start scan\r\n\
        [filter_policy]: 0-(any), 1-(whitelist), 2-(any RPA), 3-(whitelist RPA) \r\n\
        [filter_duplicate]: 0-(disable), 1-(enable) \n\r",
        cmd_scan
    },
    {
        "stopscan",
        "stopscan\n\r",
        "Stop scan\n\r",
        cmd_stopscan
    },
    {
        "showdev",
        "showdev\n\r",
        "Show scan dev list: filter simple ble service\n\r",
        cmd_showdev
    },
    {
        "con",
        "con [BD0] [BD1] [BD2] [BD3] [BD4] [BD5] [addr_type]\n\r",
        "Connect to remote device: use address\r\n\
        [BD0] [BD1] [BD2] [BD3] [BD4] [BD5]: remote device address\r\n\
        [addr_type]: 0-(public), 1-(random)\r\n\
        sample: con x11 x22 x33 x44 x55 x66 0 \n\r",
        cmd_con
    },
    {
        "condev",
        "condev [idx]\n\r",
        "Connect to remote device: use showdev to show idx\r\n\
        [idx]: use cmd showdev to show idx before use this cmd\r\n\
        sample: condev 0\n\r",
        cmd_condev
    },
    {
        "chanclassset",
        "chanclassset [idx0] [idx1] [idx2] [idx3] [idx4]\n\r",
        "Set Host Channel Classification\r\n\
        [idx0] [idx1] [idx2] [idx3] [idx4]: channel bit map\r\n\
        sample: chanclassset xff xff x3f xff x00\n\r",
        cmd_chanclassset
    },
    /************************** GATT client *************************************/
    {
        "srvdis",
        "srvdis [conn_id]\n\r",
        "Service discovery, discover all primary services\n\r",
        cmd_srvdis
    },
    /*GAPS client*/
    {
        "gapdis",
        "gapdis [conn_id]\n\r",
        "Start discovery gap service\n\r",
        cmd_gapdis
    },
    {
        "gapread",
        "gapread [conn_id] [type]\n\r",
        "Read GAP service characteristic value\r\n\
        [type]: 0-(read device name), 1-(read appearance)\r\n\
        simple: gapread 0 0\n\r",
        cmd_gapread
    },
    {
        "gaphdl",
        "gaphdl [conn_id]\n\r",
        "List GAP service handle cache\n\r",
        cmd_gaphdl
    },
    /*Simple ble client*/
    {
        "simpdis",
        "simpdis [conn_id]\n\r",
        "Start discovery simple ble service\n\r",
        cmd_simpdis
    },
    {
        "simpread",
        "simpread [conn_id] [type] [pattern]\n\r",
        "Read simple ble service characteristic and descriptor value\r\n\
        [type]: 0-(read v1), 1-(v3 cccd), 2-(v4 cccd)\r\n\
        [pattern]: 0-(read by handle), 1-(read by uuid)\r\n\
        sample: simpread 0 1 0 \n\r",
        cmd_simpread
    },
    {
        "simpcccd",
        "simpcccd [conn_id] [type] [enable]\n\r",
        "Config simple ble service client characteristic configuration descriptor value\r\n\
        [type]: 0-(v3 notify), 1-(v4 indication)\r\n\
        [enable]: 0-(disable), 1-(enable) \r\n\
        sample: simpcccd 0 1 1\n\r",
        cmd_simpcccd
    },
    {
        "simpwritev2",
        "simpwritev2 [conn_id] [type] [len]\n\r",
        "Write simple ble service V2 characteristic value\r\n\
        [type]: 1-(write request), 2-(write command) \r\n\
        [len]: type=1 len range:0-270, type=2 len range: 0-(mtu-3)\r\n\
        sample: simpwritev2 0 1 10\n\r",
        cmd_simpwritev2
    },
    {
        "simphdl",
        "simphdl [conn_id]\n\r",
        "List simple ble service handle cache\n\r",
        cmd_simphdl
    },
    /*BAS client*/
    {
        "basdis",
        "basdis [conn_id]\n\r",
        "Start discovery battery service\n\r",
        cmd_basdis
    },
    {
        "basread",
        "basread [conn_id] [type]\n\r",
        "Read battery service characteristic and descriptor value\r\n\
        [type]: 0-(battery level value), 1-(battery cccd)\r\n\
        sample: basread 0 1 \n\r",
        cmd_basread
    },
    {
        "bascccd",
        "bascccd [conn_id] [notify]\n\r",
        "Config battery service client characteristic configuration descriptor value\r\n\
        [notify]: 0-(disable), 1-(enable)\n\r",
        cmd_bascccd
    },
    {
        "bashdl",
        "bashdl [conn_id]\n\r",
        "List battery service handle cache\n\r",
        cmd_bashdl
    },
    /************************** Peripheral Only *************************************/
    {
        "startadv",
        "startadv\n\r",
        "start advertising without setting advertising parameters\n\r",
        cmd_startadv
    },
    {
        "stopadv",
        "stopadv\n\r",
        "Stop advertising\n\r",
        cmd_stopadv
    },
    {
        "adv",
        "adv [adv_interval] [filter_policy]\n\r",
        "Start Undirected advertising\r\n\
        [adv_interval]: 0x0020 - 0x4000 (20ms - 10240ms, 0.625ms/step)\r\n\
        [filter_policy]: 0-(all), 1-(whitelist scan), 2-(whitelist conn), 3-(whitelist all)\r\n\
        sample: adv x40 0 \n\r",
        cmd_adv
    },
    {
        "advld",
        "advld [BD0] [BD1] [BD2] [BD3] [BD4] [BD5]\n\r",
        "Start lower duty directed advertising\r\n\
        [BD0] [BD1] [BD2] [BD3] [BD4] [BD5]: peer address\r\n\
        sample: advld x11 x22 x33 x44 x55 x66\n\r",
        cmd_advld
    },
    {
        "advhd",
        "advhd [BD0] [BD1] [BD2] [BD3] [BD4] [BD5]\n\r",
        "Start high duty directed advertising\r\n\
        [BD0] [BD1] [BD2] [BD3] [BD4] [BD5]: peer address\r\n\
        sample: advhd x11 x22 x33 x44 x55 x66\n\r",
        cmd_advhd
    },
    {
        "advscan",
        "advscan\n\r",
        "Start scannable undirected advertising\n\r",
        cmd_advscan
    },
    {
        "advnonconn",
        "advnonconn\n\r",
        "Start non_connectable undirected advertising\n\r",
        cmd_advnonconn
    },
    {
        "latency",
        "latency [conn_id] [on_off]\n\r",
        "On off slave latency\r\n\
        [on_off]: 0-(turn on the latency), 1-(turn off the latency)\r\n\
        sample: latency 0 1\n\r",
        cmd_latency
    },
    {
        "epassed",
        "epassed [enable]\n\r",
        "Update instant passed channel map\r\n\
        [enable]: 0 - (disable), 1-(enable)\r\n\
        sample: epassed 1\n\r",
        cmd_epassed
    },
    /************************** GATT services *************************************/
    {
        "gapprefconn",
        "gapprefconn [conn_interval_min] [conn_interval_max] [latency] [supervision_timeout]\n\r",
        "Set GAP service Peripheral Preferred Connection Parameters Char value\r\n\
        [conn_interval_min]: 0x0006 to 0x0C80(1.25ms/step)\r\n\
        [conn_interval_max]: 0x0006 to 0x0C80(1.25ms/step)\r\n\
        [latency]: 0x0000 to 0x01F3\r\n\
        [supervision_timeout]: 0x000A to 0x0C80(10ms/step)\r\n\
        sample: gapprefconn x80 x90 0 500 \n\r",
        cmd_gapprefconn
    },
    {
        "simpv3notify",
        "simpv3notify [conn_id] [len] \n\r",
        "Send V3 notification\r\n\
        [len]: 0 - (mtu-3)\n\r",
        cmd_simpv3notify
    },
    {
        "simpv4ind",
        "simpv4ind [conn_id] [len]\n\r",
        "Send V4 indication\r\n\
        [len]: 0 - (mtu-3)\n\r",
        cmd_simpv4ind
    },
    {
        "basnotify",
        "basnotify [conn_id] [battery_level] \n\r",
        "Send battery level notification\r\n\
        [battery_level]: 0 - 100\r\n\
        sample: basnotify 0 80\n\r",
        cmd_basnotify
    },
#if F_BT_LE_5_0_SET_PHY_SUPPORT
    {
        "setphy",
        "setphy [conn_id] [type]\n\r",
        "Set the PHY preferences for the connection\r\n\
        [type]: 0-(1M), 1-(2M), 2-(CODED-S8), 3-(CODED-S2), 4-(tx 2M, rx 1M) \n\r",
        cmd_setphy
    },
#endif
    /* MUST be at the end: */
    {
        0,
        0,
        0,
        0
    }
};
/** @} */ /* End of group SCATTERNET_CMD */


