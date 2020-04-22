/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     provisioner_cmd.c
  * @brief    Source file for provisioner cmd.
  * @details  User command interfaces.
  * @author   bill
  * @date     2017-3-31
  * @version  v1.0
  * *************************************************************************************
  */

#include <string.h>
#include "trace.h"
#include "gap_wrapper.h"
#include "provisioner_cmd.h"
#include "provisioner_app.h"
#include "provision_client.h"
#include "provision_provisioner.h"
#include "mesh_api.h"
#include "mesh_cmd.h"
#include "test_cmd.h"
#include "client_cmd.h"
#include "generic_client_app.h"
#include "light_client_app.h"
#include "dfu_distributor_app.h"
#include "datatrans_model.h"
#include "datatrans_client_app.h"
#include "datatrans_client.h"

static plt_timer_t light_cwrgb_timer;
static uint8_t light_cwrgb_counter;
static uint8_t light_cwrgb_num;
static uint8_t light_cwrgb_mode;
static uint16_t light_cwrgb_dst;
static uint8_t light_cwrgb_app_key_index;
static uint8_t light_cwrgb_ack;
static uint8_t light_cwrgb_state[5];

static user_cmd_parse_result_t user_cmd_prov_discover(user_cmd_parse_value_t *pparse_value)
{
    data_uart_debug("Prov Start Discover\r\n");
    prov_client_conn_id = pparse_value->dw_parameter[0];
    prov_client_start_discovery(prov_client_conn_id);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_prov_read_char(user_cmd_parse_value_t *pparse_value)
{
    /* Indicate which char to be read. */
    prov_read_type_t read_char_type = (prov_read_type_t)pparse_value->dw_parameter[0];
    /* Read by handle or UUID, 1--by UUID, 0--by handle. */
    uint8_t read_pattern = (uint8_t)pparse_value->dw_parameter[1];
    data_uart_debug("Pro Read Char\r\n");
    if (read_pattern)
    {
        prov_client_read_by_uuid(prov_client_conn_id, read_char_type);
    }
    else
    {
        prov_client_read_by_handle(prov_client_conn_id, read_char_type);
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_prov_cccd_operate(user_cmd_parse_value_t *pparse_value)
{
    /* Indicate which char CCCD command. */
    uint8_t cmd_type = (uint8_t)pparse_value->dw_parameter[0];
    /* Enable or disable, 1--enable, 0--disable. */
    bool cmd_data = (bool)pparse_value->dw_parameter[1];
    data_uart_debug("Prov Cccd Operate\r\n");
    switch (cmd_type)
    {
    case 0:/* V3 Notify char notif enable/disable. */
        {
            prov_client_data_out_cccd_set(prov_client_conn_id, cmd_data);
            proxy_ctx_set_link(prov_proxy_ctx_id, prov_client_conn_id);
        }
        break;
    default:
        return USER_CMD_RESULT_WRONG_PARAMETER;
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_prov_list(user_cmd_parse_value_t *pparse_value)
{
    data_uart_debug("Prov Server Handle List:\r\nidx\thandle\r\n");
    for (prov_handle_type_t hdl_idx = HDL_PROV_SRV_START; hdl_idx < HDL_PROV_CACHE_LEN; hdl_idx++)
    {
        data_uart_debug("%d\t0x%x\r\n", hdl_idx, prov_client_handle_get(prov_client_conn_id, hdl_idx));
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_pb_adv_con(user_cmd_parse_value_t *pparse_value)
{
    uint8_t dev_uuid[16];
    plt_hex_to_bin(dev_uuid, (uint8_t *)pparse_value->pparameter[0], sizeof(dev_uuid));
    if (!pb_adv_link_open(0, dev_uuid))
    {
        data_uart_debug("PB_ADV: Link Busy!\r\n");
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_pb_adv_disc(user_cmd_parse_value_t *pparse_value)
{
    if (pb_adv_link_close(0, PB_ADV_LINK_CLOSE_SUCCESS))
    {
        data_uart_debug("PB_ADV: Link Closed!\r\n");
    }
    else
    {
        data_uart_debug("PB_ADV: Link Closed Already!\r\n");
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_prov(user_cmd_parse_value_t *pparse_value)
{
    data_uart_debug("provision...\r\n");
    uint32_t attn_dur = pparse_value->dw_parameter[0];
    prov_manual = pparse_value->dw_parameter[1];
    prov_start_time = plt_time_read_ms();
    return prov_invite(attn_dur) ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
}

static user_cmd_parse_result_t user_cmd_prov_stop(user_cmd_parse_value_t *pparse_value)
{
    return prov_reject() ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
}

static user_cmd_parse_result_t user_cmd_prov_auth_path(user_cmd_parse_value_t *pparse_value)
{
    prov_start_algorithm_t algo = (prov_start_algorithm_t)pparse_value->dw_parameter[0];
    prov_start_public_key_t public_key = (prov_start_public_key_t)pparse_value->dw_parameter[1];
    prov_auth_method_t auth_method = (prov_auth_method_t)pparse_value->dw_parameter[2];
    uint8_t oob_action = pparse_value->dw_parameter[3];
    uint8_t oob_size = pparse_value->dw_parameter[4];

    prov_start_t prov_start;
    prov_start.algorithm = algo;
    prov_start.public_key = public_key;
    prov_start.auth_method = auth_method;
    prov_start.auth_action.oob_action = oob_action;
    prov_start.auth_size.oob_size = oob_size;
    return prov_path_choose(&prov_start) == true ? USER_CMD_RESULT_OK : USER_CMD_RESULT_WRONG_PARAMETER;
}

static user_cmd_parse_result_t user_cmd_unprov(user_cmd_parse_value_t *pparse_value)
{
    data_uart_debug("Unprovision...\r\n");
#if MESH_UNPROVISIONING_SUPPORT
    prov_unprovisioning();
#endif
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_cfg_client_key_set(user_cmd_parse_value_t
                                                           *pparse_value)
{
    uint8_t key_index = pparse_value->dw_parameter[0];
    bool use_app_key = pparse_value->dw_parameter[1] ? TRUE : FALSE;
    mesh_node.features.cfg_model_use_app_key = use_app_key;
    return cfg_client_key_set(key_index) ? USER_CMD_RESULT_OK :
           USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
}

static user_cmd_parse_result_t user_cmd_compo_data_get(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    uint8_t page = pparse_value->dw_parameter[1];
    if (MESH_IS_UNASSIGNED_ADDR(dst))
    {
        data_uart_debug("CDP0 len=%d, data=", mesh_node.compo_data_size[0]);
        data_uart_dump(mesh_node.compo_data[0], mesh_node.compo_data_size[0]);
    }
    else
    {
        cfg_compo_data_get(dst, page);
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_node_reset(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    bool clear = pparse_value->dw_parameter[1];
    if (MESH_IS_UNASSIGNED_ADDR(dst))
    {
        mesh_node_reset();
    }
    else
    {
        cfg_node_reset(dst);
        if (clear)
        {
            int dev_key_index = dev_key_find(dst);
            if (dev_key_index >= 0)
            {
                dev_key_delete(dev_key_index);
                mesh_flash_store(MESH_FLASH_PARAMS_DEV_KEY, &dev_key_index);
            }
        }
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_net_key_add(user_cmd_parse_value_t *pparse_value)
{
    if (pparse_value->para_count != 2)
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    uint16_t dst = pparse_value->dw_parameter[0];
    uint16_t net_key_index = pparse_value->dw_parameter[1];
    if (net_key_index >= mesh_node.net_key_num ||
        mesh_node.net_key_list[net_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    cfg_net_key_add(dst, mesh_node.net_key_list[net_key_index].net_key_index_g,
                    mesh_node.net_key_list[net_key_index].pnet_key[key_state_to_new_loop(
                                                                       mesh_node.net_key_list[net_key_index].key_state)]->net_key);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_net_key_update(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    uint16_t net_key_index = pparse_value->dw_parameter[1];
    if (net_key_index >= mesh_node.net_key_num ||
        mesh_node.net_key_list[net_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    if (MESH_IS_UNASSIGNED_ADDR(dst))
    {
        if (pparse_value->para_count != 3)
        {
            return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
        }
        uint8_t net_key[MESH_COMMON_KEY_SIZE];
        plt_hex_to_bin(net_key, (uint8_t *)pparse_value->pparameter[2], MESH_COMMON_KEY_SIZE);
        return net_key_update(net_key_index, mesh_node.net_key_list[net_key_index].net_key_index_g,
                              net_key) ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
    }
    else
    {
        cfg_net_key_update(dst, mesh_node.net_key_list[net_key_index].net_key_index_g,
                           mesh_node.net_key_list[net_key_index].pnet_key[key_state_to_new_loop(
                                                                              mesh_node.net_key_list[net_key_index].key_state)]->net_key);
        return USER_CMD_RESULT_OK;
    }
}

static user_cmd_parse_result_t user_cmd_app_key_add(user_cmd_parse_value_t *pparse_value)
{
    if (pparse_value->para_count != 3)
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    uint16_t dst = pparse_value->dw_parameter[0];
    uint16_t net_key_index = pparse_value->dw_parameter[1];
    uint16_t app_key_index = pparse_value->dw_parameter[2];
    if (net_key_index >= mesh_node.net_key_num ||
        mesh_node.net_key_list[net_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    if (app_key_index >= mesh_node.app_key_num ||
        mesh_node.app_key_list[app_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    cfg_app_key_add(dst, mesh_node.net_key_list[net_key_index].net_key_index_g,
                    mesh_node.app_key_list[app_key_index].app_key_index_g,
                    mesh_node.app_key_list[app_key_index].papp_key[key_state_to_new_loop(
                                                                       mesh_node.app_key_list[app_key_index].key_state)]->app_key);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_app_key_update(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    uint16_t net_key_index = pparse_value->dw_parameter[1];
    uint16_t app_key_index = pparse_value->dw_parameter[2];
    if (net_key_index >= mesh_node.net_key_num ||
        mesh_node.net_key_list[net_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    if (app_key_index >= mesh_node.app_key_num ||
        mesh_node.app_key_list[app_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    if (MESH_IS_UNASSIGNED_ADDR(dst))
    {
        if (pparse_value->para_count != 4)
        {
            return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
        }
        uint8_t app_key[MESH_COMMON_KEY_SIZE];
        plt_hex_to_bin(app_key, (uint8_t *)pparse_value->pparameter[3], MESH_COMMON_KEY_SIZE);
        return app_key_update(app_key_index, net_key_index,
                              mesh_node.app_key_list[app_key_index].app_key_index_g,
                              app_key) ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
    }
    else
    {
        cfg_app_key_update(dst, mesh_node.net_key_list[net_key_index].net_key_index_g,
                           mesh_node.app_key_list[app_key_index].app_key_index_g,
                           mesh_node.app_key_list[app_key_index].papp_key[key_state_to_new_loop(
                                                                              mesh_node.app_key_list[app_key_index].key_state)]->app_key);
        return USER_CMD_RESULT_OK;
    }
}

static user_cmd_parse_result_t user_cmd_model_app_bind(user_cmd_parse_value_t *pparse_value)
{
    if (pparse_value->para_count != 4)
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    uint16_t dst = pparse_value->dw_parameter[0];
    uint8_t element_index = pparse_value->dw_parameter[1];
    uint32_t model_id = pparse_value->dw_parameter[2];
    uint16_t app_key_index = pparse_value->dw_parameter[3];
    if (app_key_index >= mesh_node.app_key_num ||
        mesh_node.app_key_list[app_key_index].key_state == MESH_KEY_STATE_INVALID)
    {
        return USER_CMD_RESULT_VALUE_OUT_OF_RANGE;
    }
    cfg_model_app_bind(dst, dst + element_index, mesh_node.app_key_list[app_key_index].app_key_index_g,
                       model_id);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_model_sub_add(user_cmd_parse_value_t *pparse_value)
{
    if (pparse_value->para_count != 4)
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    uint16_t dst = pparse_value->dw_parameter[0];
    uint8_t element_index = pparse_value->dw_parameter[1];
    uint32_t model_id = pparse_value->dw_parameter[2];
    uint16_t group_addr = pparse_value->dw_parameter[3];
    if (MESH_NOT_GROUP_ADDR(group_addr))
    {
        return USER_CMD_RESULT_WRONG_PARAMETER;
    }
    cfg_model_sub_add(dst, dst + element_index, false, (uint8_t *)&group_addr, model_id);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_model_sub_delete(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    uint8_t element_index = pparse_value->dw_parameter[1];
    uint32_t model_id = pparse_value->dw_parameter[2];
    uint16_t group_addr = pparse_value->dw_parameter[3];
    if (pparse_value->para_count == 4)
    {
        if (MESH_NOT_GROUP_ADDR(group_addr))
        {
            return USER_CMD_RESULT_WRONG_PARAMETER;
        }
        cfg_model_sub_delete(dst, dst + element_index, false, (uint8_t *)&group_addr, model_id);
    }
    else if (pparse_value->para_count == 3)
    {
        cfg_model_sub_delete_all(dst, dst + element_index, model_id);
    }
    else
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_key_refresh_phase_get(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    uint16_t net_key_index = pparse_value->dw_parameter[1];
    cfg_key_refresh_phase_get(dst, net_key_index_to_global(net_key_index));
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_key_refresh_phase_set(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst = pparse_value->dw_parameter[0];
    uint16_t net_key_index = pparse_value->dw_parameter[1];
    uint8_t phase = pparse_value->dw_parameter[2];
    cfg_key_refresh_phase_set(dst, net_key_index_to_global(net_key_index), phase);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_light_cwrgb_get(user_cmd_parse_value_t *pparse_value)
{
    uint16_t dst;
    uint16_t app_key_index;
    dst = pparse_value->dw_parameter[0];
    app_key_index = pparse_value->dw_parameter[1];
    light_cwrgb_get(&light_cwrgb_client, dst, app_key_index);
    return USER_CMD_RESULT_OK;
}

void light_cwrgb_process(void)
{
    uint8_t switch_state;
    uint16_t light_addr;
    uint8_t light_state[5];
    memset(light_state, 0, sizeof(light_state));
    switch (light_cwrgb_mode)
    {
    case 0:
        {
            switch_state = light_cwrgb_counter % 6;
            if (switch_state < sizeof(light_state))
            {
                light_state[switch_state] = 255;
            }
            light_addr = light_cwrgb_dst;
            break;
        }
    case 1:
    case 2:
        {
            switch_state = light_cwrgb_counter % 32;
            if (switch_state >= 16)
            {
                switch_state -= 16;
                memcpy(light_state, light_cwrgb_state, sizeof(light_state));
            }
            light_addr = (light_cwrgb_mode == 1 ? 0xc010 : 0xc000) + switch_state;
            break;
        }
    case 3:
        {
            uint8_t round = (light_cwrgb_counter / 32) % 3;
            switch (round)
            {
            case 0:
            case 1:
                switch_state = light_cwrgb_counter % 32;
                if (switch_state >= 16)
                {
                    switch_state -= 16;
                    memcpy(light_state, light_cwrgb_state, sizeof(light_state));
                }
                light_addr = (round == 0 ? 0xc010 : 0xc000) + switch_state;
                break;
            case 2:
                switch_state = (light_cwrgb_counter % 32 + 5) % 6;
                if (switch_state < sizeof(light_state))
                {
                    light_state[switch_state] = 255;
                }
                light_addr = light_cwrgb_dst;
                break;
            default:
                break;
            }
            break;
        }
    case 4:
        if (light_cwrgb_counter < light_cwrgb_num - 1)
        {
            memcpy(light_state, light_cwrgb_state, sizeof(light_state));
            light_addr = light_cwrgb_dst;
        }
        else
        {
            if (light_cwrgb_timer != NULL)
            {
                plt_timer_delete(light_cwrgb_timer, 0);
                light_cwrgb_timer = NULL;
            }
            data_uart_debug("done\r\n>");
        }
        break;
    default:
        break;
    }

    light_cwrgb_set(&light_cwrgb_client, light_addr, light_cwrgb_app_key_index, light_state,
                    light_cwrgb_ack);
    light_cwrgb_counter++;
}

extern void *evt_queue_handle; //!< Event queue handle
extern void *io_queue_handle; //!< IO queue handle
static void light_cwrgb_timeout_cb(void *ptimer)
{
    uint8_t event = EVENT_IO_TO_APP;
    T_IO_MSG msg;
    msg.type = LIGHT_CWRGB_TIMEOUT_MSG;
    if (os_msg_send(io_queue_handle, &msg, 0) == false)
    {
    }
    else if (os_msg_send(evt_queue_handle, &event, 0) == false)
    {
    }
}

static user_cmd_parse_result_t user_cmd_light_cwrgb_set(user_cmd_parse_value_t *pparse_value)
{
    light_cwrgb_dst = pparse_value->dw_parameter[0];
    plt_hex_to_bin(light_cwrgb_state, (uint8_t *)pparse_value->pparameter[1],
                   sizeof(light_cwrgb_state));
    light_cwrgb_app_key_index = pparse_value->dw_parameter[2];
    light_cwrgb_ack = pparse_value->dw_parameter[3];
    light_cwrgb_set(&light_cwrgb_client, light_cwrgb_dst, light_cwrgb_app_key_index, light_cwrgb_state,
                    light_cwrgb_ack);
    uint32_t period = pparse_value->dw_parameter[4];
    if (period == 0)
    {
        if (light_cwrgb_timer != NULL)
        {
            plt_timer_delete(light_cwrgb_timer, 0);
            light_cwrgb_timer = NULL;
        }
    }
    else
    {
        if (period < 10)
        {
            period = 10;
        }
        light_cwrgb_mode = pparse_value->dw_parameter[5];
        light_cwrgb_num = pparse_value->dw_parameter[6];
        light_cwrgb_counter = 0;
        if (light_cwrgb_timer == NULL)
        {
            light_cwrgb_timer = plt_timer_create("cwrgb", period, true, 0,
                                                 light_cwrgb_timeout_cb);
            if (light_cwrgb_timer != NULL)
            {
                plt_timer_start(light_cwrgb_timer, 0);
            }
        }
        else
        {
            plt_timer_change_period(light_cwrgb_timer, period, 0);
        }
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_dfu(user_cmd_parse_value_t *pparse_value)
{
    uint16_t company_id;
    uint8_t firmware_id[FW_UPDATE_FW_ID_LEN];
    uint8_t object_id[8];
    uint16_t dst;
    if (pparse_value->para_count < 5)
    {
        return USER_CMD_RESULT_WRONG_NUM_OF_PARAMETERS;
    }
    company_id = pparse_value->dw_parameter[0];
    plt_hex_to_bin(firmware_id, (uint8_t *)pparse_value->pparameter[1], FW_UPDATE_FW_ID_LEN);
    plt_swap1(firmware_id, FW_UPDATE_FW_ID_LEN);
    plt_hex_to_bin(object_id, (uint8_t *)pparse_value->pparameter[2], 8);
    dst = pparse_value->dw_parameter[3];
    uint16_t *node_list = plt_malloc((pparse_value->para_count - 4) << 1, RAM_TYPE_DATA_OFF);
    for (uint8_t loop = 0; loop < pparse_value->para_count - 4; loop++)
    {
        node_list[loop] = pparse_value->dw_parameter[loop + 4];
    }
    bool ret = dfu_dist_start(company_id, firmware_id, object_id, dst, node_list,
                              pparse_value->para_count - 4);
    plt_free(node_list, RAM_TYPE_DATA_OFF);
    return ret ? USER_CMD_RESULT_OK : USER_CMD_RESULT_ERROR;
}

static user_cmd_parse_result_t user_cmd_datatrans_write(user_cmd_parse_value_t *pparse_value)
{
    uint8_t para_count = pparse_value->para_count;
    uint8_t data[18];

    for (uint8_t i = 0; i < para_count - 3; ++i)
    {
        data[i] = pparse_value->dw_parameter[i + 1];
    }
    datatrans_write(&datatrans_client, pparse_value->dw_parameter[0],
                    pparse_value->dw_parameter[para_count - 2], para_count - 3, data,
                    pparse_value->dw_parameter[para_count - 1]);

    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_datatrans_read(user_cmd_parse_value_t *pparse_value)
{
    datatrans_read(&datatrans_client, pparse_value->dw_parameter[0],
                   pparse_value->dw_parameter[2], pparse_value->dw_parameter[1]);

    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_datatrans_discover(user_cmd_parse_value_t *pparse_value)
{
    data_uart_debug("datatrans start discover\r\n");
    datatrans_client_start_discovery(0);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_datatrans_read_char(user_cmd_parse_value_t *pparse_value)
{
    /** indicate which char to be read. */
    datatrans_read_type_t read_char_type = (datatrans_read_type_t)pparse_value->dw_parameter[0];
    /** read by handle or UUID, 1--by UUID, 0--by handle. */
    uint8_t read_pattern = (uint8_t)pparse_value->dw_parameter[1];
    data_uart_debug("datatrans read char\r\n");
    if (read_pattern)
    {
        datatrans_client_read_by_uuid(0, read_char_type);
    }
    else
    {
        datatrans_client_read_by_handle(0, read_char_type);
    }
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_datatrans_cccd_operate(user_cmd_parse_value_t *pparse_value)
{
    /** enable or disable, 1--enable, 0--disable. */
    bool cmd_data = (bool)pparse_value->dw_parameter[1];
    data_uart_debug("datatrans cccd operate\r\n");
    datatrans_client_data_out_cccd_set(0, cmd_data);
    return USER_CMD_RESULT_OK;
}

static user_cmd_parse_result_t user_cmd_datatrans_list(user_cmd_parse_value_t *pparse_value)
{
    data_uart_debug("datatrans server handle list:\r\nidx\thandle\r\n");
    for (datatrans_handle_type_t hdl_idx = HDL_DATATRANS_SRV_START; hdl_idx < HDL_DATATRANS_CACHE_LEN;
         hdl_idx++)
    {
        data_uart_debug("%d\t0x%x\r\n", hdl_idx, datatrans_client_handle_get(hdl_idx));
    }
    return USER_CMD_RESULT_OK;
}

/*----------------------------------------------------
 * command table
 * --------------------------------------------------*/
const user_cmd_table_entry_t provisioner_cmd_table[] =
{
    // mesh common cmd
    MESH_COMMON_CMD,
    CLIENT_CMD,
    TEST_CMD,
    // provisioner cmd
    // pb-adv
    {
        "pbadvcon",
        "pbadvcon [dev uuid]\n\r",
        "create a pb-adv link with the device uuid\n\r",
        user_cmd_pb_adv_con
    },
    {
        "pbadvdisc",
        "pbadvdisc\n\r",
        "disconnect the pb-adv link\n\r",
        user_cmd_pb_adv_disc
    },
    // pb-gatt
    {
        "provdis",
        "provdis [conn id]\n\r",
        "Start discovery provisioning service\n\r",
        user_cmd_prov_discover
    },
    {
        "provread",
        "provread [char] [pattern: handle/UUID16]\n\r",
        "Read all related chars by user input\n\r",
        user_cmd_prov_read_char
    },
    {
        "provcmd",
        "provcmd [char CCCD] [command: enable/disable]\n\r",
        "Provisioning notify/ind switch command\n\r",
        user_cmd_prov_cccd_operate
    },
    {
        "provls",
        "provls\n\r",
        "Provision server handle list\n\r",
        user_cmd_prov_list
    },
    // provisioner
    {
        "prov",
        "prov [attn_dur] [manual]\n\r",
        "provision a new mesh device\n\r",
        user_cmd_prov
    },
    {
        "provs",
        "provs\n\r",
        "provision stop\n\r",
        user_cmd_prov_stop
    },
    {
        "pap",
        "pap [algorithm] [pubkey] [method: nsoi] [action] [size]\n\r",
        "provision authentication path\n\r",
        user_cmd_prov_auth_path
    },
    {
        "unprov",
        "unprov\n\r",
        "unprovision the mesh device\n\r",
        user_cmd_unprov
    },
    // cfg client key set
    {
        "ccks",
        "ccks [key_index] [use_app_key]\n\r",
        "cfg client key set\n\r",
        user_cmd_cfg_client_key_set
    },
    // cfg client or local setting
    {
        "cdg",
        "cdg [dst]\n\r",
        "compo data get\n\r",
        user_cmd_compo_data_get
    },
    {
        "nr",
        "nr [dst] [clear]\n\r",
        "node reset\n\r",
        user_cmd_node_reset
    },
    {
        "nka",
        "nka [dst] [net_key_index]\n\r",
        "net key add\n\r",
        user_cmd_net_key_add
    },
    {
        "nku",
        "nku [dst] [net_key_index] [net key]\n\r",
        "net key update\n\r",
        user_cmd_net_key_update
    },
    {
        "aka",
        "aka [dst] [net_key_index] [app_key_index]\n\r",
        "app key add\n\r",
        user_cmd_app_key_add
    },
    {
        "aku",
        "aku [dst] [net_key_index] [app_key_index] [app key]\n\r",
        "app key update\n\r",
        user_cmd_app_key_update
    },
    {
        "mab",
        "mab [dst] [element index] [model_id] [app_key_index]\n\r",
        "model app bind\n\r",
        user_cmd_model_app_bind
    },
    {
        "msa",
        "msa [dst] [element index] [model_id] [group addr]\n\r",
        "model subsribe add\n\r",
        user_cmd_model_sub_add
    },
    {
        "msd",
        "msd [dst] [element index] [model_id] <group addr>\n\r",
        "model subsribe delete\n\r",
        user_cmd_model_sub_delete
    },
    {
        "krpg",
        "krpg [dst] [net key index]\n\r",
        "key refresh phase get\n\r",
        user_cmd_key_refresh_phase_get
    },
    {
        "krps",
        "krps [dst] [net key index] [phase]\n\r",
        "key refresh phase set\n\r",
        user_cmd_key_refresh_phase_set
    },
    // light models
    {
        "lrg",
        "lrg [dst] [app_key_index]\n\r",
        "light cwrgb get\n\r",
        user_cmd_light_cwrgb_get
    },
    {
        "lrs",
        "lrs [dst] [cwrgb] [app_key_index] [ack] [period(ms)] [mode] [times]\n\r",
        "light cwrgb set\n\r",
        user_cmd_light_cwrgb_set
    },
    {
        "dfu",
        "dfu [company_id] [fw_id] [obj_id] [dst] [node_addr...]\n\r",
        "dfu firmware update + object transfer\n\r",
        user_cmd_dfu
    },
    {
        "dtw",
        "dtw [dst] [data...] [app_key_index] [ack] \n\r",
        "data transmission write data\n\r",
        user_cmd_datatrans_write
    },
    {
        "dtr",
        "dtr [dst] [len] [app_key_index]\n\r",
        "data transmission read data\n\r",
        user_cmd_datatrans_read
    },
    {
        "dtdis",
        "dtdis\n\r",
        "start discovery data transmission service\n\r",
        user_cmd_datatrans_discover
    },
    {
        "dtread",
        "dt [char] [pattern: handle/UUID16]\n\r",
        "read all related characteristic by user input\n\r",
        user_cmd_datatrans_read_char
    },
    {
        "dtcmd",
        "dtcmd [char CCCD] [command: enable/disable]\n\r",
        "data transmission notify/ind switch command\n\r",
        user_cmd_datatrans_cccd_operate
    },
    {
        "dtls",
        "dtls\n\r",
        "data transmission server handle list\n\r",
        user_cmd_datatrans_list
    },
    /* MUST be at the end: */
    {
        0,
        0,
        0,
        0
    }
};
