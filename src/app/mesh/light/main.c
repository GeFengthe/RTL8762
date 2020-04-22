/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      main.c
   * @brief     Source file for BLE scatternet project, mainly used for initialize modules
   * @author    jane
   * @date      2017-06-12
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <stdlib.h>
#include <os_sched.h>
#include <string.h>
#include <app_task.h>
#include <trace.h>
#include <gap.h>
#include <gap_bond_le.h>
#include <gap_scan.h>
#include <gap_msg.h>
#include <profile_client.h>
#include <gaps_client.h>
#include <gap_adv.h>
#include <profile_server.h>
#include <gatt_builtin_services.h>
#include <platform_utils.h>

#if DLPS_EN
#include "dlps.h"
#include "rtl876x_io_dlps.h"
#endif

#include "mesh_api.h"
#include "mesh_sdk.h"
#include "health.h"
#include "ping.h"
#include "ping_app.h"
#include "light_app.h"
#include "light_config.h"
#include "light_cwrgb_app.h"
#include "light_cwrgb_server_app.h"
#include "light_hsl_server_app.h"
#include "light_ctl_server_app.h"
#include "light_ctl_hsl_server_app.h"
#include "light_lightness_server_app.h"
#include "group_light_app.h"
#include "light_controller_app.h"
#include "light_storage_app.h"

#include "ota_server.h"
#include "dfu_server.h"
#include "dfu_client.h"
#include "mem_config.h"

#define NODE_DEFAULT_CONFIG                     0
#if NODE_DEFAULT_CONFIG
#define MATRIX_SIZE_ROW                         10
#define MATRIX_SIZE_COLUMN                      10
extern mesh_model_info_t light_cwrgb_server;
#endif

static mesh_model_info_t health_server_model;

void light_init(uint8_t light_type)
{
    switch (light_type)
    {
    case LIGHT_LIGHTNESS:
        light_lightness_server_models_init(0);
        break;
    case LIGHT_CW:
        light_ctl_server_models_init(0);
        break;
    case LIGHT_RGB:
        light_hsl_server_models_init(0);
        break;
    case LIGHT_CWRGB:
        light_ctl_hsl_server_models_init(0);
        break;
    default:
        break;
    }
    light_cwrgb_server_models_init();
}

/******************************************************************
 * @fn          Initial gap parameters
 * @brief      Initialize peripheral and gap bond manager related parameters
 *
 * @return     void
 */
void mesh_stack_init(void)
{
    /** set ble stack log level, disable nonsignificant log */
    log_module_bitmap_trace_set(0xFFFFFFFFFFFFFFFF, LEVEL_TRACE, 0);
    log_module_bitmap_trace_set(0xFFFFFFFFFFFFFFFF, LEVEL_INFO, 0);
    log_module_trace_set(MODULE_LOWERSTACK, LEVEL_ERROR, 0);
    log_module_trace_set(MODULE_SNOOP, LEVEL_ERROR, 0);

    /** set mesh stack log level, default all on, disable the log of level LEVEL_TRACE */
    uint32_t module_bitmap[MESH_LOG_LEVEL_SIZE] = {0};
    diag_level_set(LEVEL_TRACE, module_bitmap);

    /** print the mesh sdk & lib version */
    mesh_sdk_version();

    /** mesh stack needs rand seed */
    plt_srand(platform_random(0xffffffff));

    /** set device name and appearance */
    char *dev_name = "RTK Mesh Light";
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    gap_sched_params_set(GAP_SCHED_PARAMS_DEVICE_NAME, dev_name, GAP_DEVICE_NAME_LEN);
    gap_sched_params_set(GAP_SCHED_PARAMS_APPEARANCE, &appearance, sizeof(appearance));

#if DLPS_EN
    uint16_t scan_interval = 0x320; //!< 500ms
    uint16_t scan_window = 0x30; //!< 30ms
    gap_sched_params_set(GAP_SCHED_PARAMS_SCAN_INTERVAL, &scan_interval, sizeof(scan_interval));
    gap_sched_params_set(GAP_SCHED_PARAMS_SCAN_WINDOW, &scan_window, sizeof(scan_window));
#endif

    /** set device uuid according to bt address */
    uint8_t bt_addr[6];
    uint8_t dev_uuid[16] = MESH_DEVICE_UUID;
    gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);
    memcpy(dev_uuid, bt_addr, sizeof(bt_addr));
    device_uuid_set(dev_uuid);

    /** configure provisioning parameters */
    prov_capabilities_t prov_capabilities =
    {
        .algorithm = PROV_CAP_ALGO_FIPS_P256_ELLIPTIC_CURVE,
        .public_key = 0,
        .static_oob = 0,
        .output_oob_size = 0,
        .output_oob_action = 0,
        .input_oob_size = 0,
        .input_oob_action = 0
    };
    prov_params_set(PROV_PARAMS_CAPABILITIES, &prov_capabilities, sizeof(prov_capabilities_t));
    prov_params_set(PROV_PARAMS_CALLBACK_FUN, prov_cb, sizeof(prov_cb_pf));

    /** config node parameters */
    mesh_node_features_t features =
    {
        .role = MESH_ROLE_DEVICE,
        .relay = 1,
        .proxy = 1,
        .fn = 0,
        .lpn = 0,
        .prov = 1,
        .udb = 1,
        .snb = 1,
        .bg_scan = 1,
        .flash = 1,
        .flash_rpl = 1
    };
    mesh_node_cfg_t node_cfg =
    {
        .dev_key_num = 1,
        .net_key_num = 3,
        .app_key_num = 3,
        .vir_addr_num = 3,
        .rpl_num = 20,
        .sub_addr_num = 10,
        .proxy_num = 1
    };
    mesh_node_cfg(features, &node_cfg);

    /** create elements and register models */
    mesh_element_create(GATT_NS_DESC_UNKNOWN);
    health_server_reg(0, &health_server_model);
    health_server_set_company_id(&health_server_model, COMPANY_ID);
    ping_control_reg(ping_app_ping_cb, NULL);
    trans_ping_pong_init(ping_app_ping_cb, NULL);
    light_init(LIGHT_TYPE);
    compo_data_page0_header_t compo_data_page0_header = {COMPANY_ID, PRODUCT_ID, VERSION_ID};
    compo_data_page0_gen(&compo_data_page0_header);

    /** init mesh stack */
    /** restore light ahead since it may restore the fatory setting */
    if (light_flash_restore())
    {
#if (GROUP_LIGHT_SUPPORTED && GROUP_RECEIVER_PREEMPTIVE_MODE == 0)
        group_receiver_nvm_clear();
#endif
        mesh_init();
    }
    else
    {
#if NODE_DEFAULT_CONFIG
        if (mesh_node_state_restore() != PROV_NODE)
        {
            /** set the mesh parameter of light for test */
            mesh_node.node_state = PROV_NODE;
            mesh_init();
            uint16_t addr = LE_EXTRN2WORD(bt_addr);
            /* three elements per light */
            mesh_node.unicast_addr = (0x64 + 3 * addr) & MESH_UNICAST_ADDR_MASK;
            const uint8_t net_key[] = MESH_NET_KEY;
            const uint8_t app_key[] = MESH_APP_KEY;
            uint16_t net_key_index = net_key_add(0, net_key);
            app_key_add(net_key_index, 0, app_key);
            mesh_model_bind_all_key();
            mesh_model_sub(light_cwrgb_server.pmodel,
                           MESH_GROUP_ADDR_DYNAMIC_START + (addr / MATRIX_SIZE_COLUMN % MATRIX_SIZE_ROW) &
                           MESH_GROUP_ADDR_MASK);
            mesh_model_sub(light_cwrgb_server.pmodel,
                           MESH_GROUP_ADDR_DYNAMIC_START + (MATRIX_SIZE_ROW + addr % MATRIX_SIZE_COLUMN) &
                           MESH_GROUP_ADDR_MASK);
        }
        else
#endif
        {
            mesh_init();
        }
    }
}

/**
  * @brief  Initialize gap related parameters
  * @return void
  */
void app_le_gap_init(void)
{
    /* GAP Bond Manager parameters */
    uint8_t  auth_pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint16_t auth_flags = GAP_AUTHEN_BIT_BONDING_FLAG;
    uint8_t  auth_io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8_t  auth_oob = false;
    uint8_t  auth_use_fix_passkey = false;
    uint32_t auth_fix_passkey = 0;
    uint8_t  auth_sec_req_enable = false;
    uint16_t auth_sec_req_flags = GAP_AUTHEN_BIT_BONDING_FLAG;

    /* Setup the GAP Bond Manager */
    gap_set_param(GAP_PARAM_BOND_PAIRING_MODE, sizeof(auth_pair_mode), &auth_pair_mode);
    gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(auth_flags), &auth_flags);
    gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(auth_io_cap), &auth_io_cap);
    gap_set_param(GAP_PARAM_BOND_OOB_ENABLED, sizeof(auth_oob), &auth_oob);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY, sizeof(auth_fix_passkey), &auth_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY_ENABLE, sizeof(auth_use_fix_passkey),
                      &auth_use_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_ENABLE, sizeof(auth_sec_req_enable), &auth_sec_req_enable);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_REQUIREMENT, sizeof(auth_sec_req_flags),
                      &auth_sec_req_flags);

    /* register gap message callback */
    le_register_app_cb(app_gap_callback);

#if F_BT_LE_5_0_SET_PHY_SUPPORT
    uint8_t  phys_prefer = GAP_PHYS_PREFER_ALL;
    uint8_t  tx_phys_prefer = GAP_PHYS_PREFER_1M_BIT | GAP_PHYS_PREFER_2M_BIT |
                              GAP_PHYS_PREFER_CODED_BIT;
    uint8_t  rx_phys_prefer = GAP_PHYS_PREFER_1M_BIT | GAP_PHYS_PREFER_2M_BIT |
                              GAP_PHYS_PREFER_CODED_BIT;
    le_set_gap_param(GAP_PARAM_DEFAULT_PHYS_PREFER, sizeof(phys_prefer), &phys_prefer);
    le_set_gap_param(GAP_PARAM_DEFAULT_TX_PHYS_PREFER, sizeof(tx_phys_prefer), &tx_phys_prefer);
    le_set_gap_param(GAP_PARAM_DEFAULT_RX_PHYS_PREFER, sizeof(rx_phys_prefer), &rx_phys_prefer);
#endif
}

/**
 * @brief  Add GATT services, clients and register callbacks
 * @return void
 */
void app_le_profile_init(void)
{
    server_init(MESH_GATT_SERVER_COUNT + 2);
    /* Add Server Module */
    ota_server_add(NULL);
    dfu_server_add(app_profile_callback);

    /* Register Server Callback */
    server_register_app_cb(app_profile_callback);

    client_init(1);
    /* Add Client Module */
#if DFU_AUTO_BETWEEN_DEVICES
    dfu_client_add(app_client_callback);
#endif

    /* Register Client Callback--App_ClientCallback to handle events from Profile Client layer. */
    client_register_general_client_cb(app_client_callback);
}

/**
 * @brief    Contains the initialization of pinmux settings and pad settings
 * @note     All the pinmux settings and pad settings shall be initiated in this function,
 *           but if legacy driver is used, the initialization of pinmux setting and pad setting
 *           should be peformed with the IO initializing.
 * @return   void
 */
void board_init(void)
{
    light_driver_init();
    light_cwrgb_driver_init();
#if LIGHT_TYPE == LIGHT_LIGHTNESS
    light_controller_init();
#endif
}

/**
 * @brief    Contains the initialization of peripherals
 * @note     Both new architecture driver and legacy driver initialization method can be used
 * @return   void
 */
void driver_init(void)
{

}
void app_enter_dlps_config(void)
{


}
void app_exit_dlps_config(void)
{

}
/**
 * @brief    Contains the power mode settings
 * @return   void
 */
void pwr_mgr_init(void)
{
#if DLPS_EN
    DLPS_IORegUserDlpsEnterCb(app_enter_dlps_config);
    DLPS_IORegUserDlpsExitCb(app_exit_dlps_config);
    DLPS_IORegister();
    lps_mode_set(LPM_DLPS_MODE);
#endif
}

/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Scatternet APP, thus only one APP task is init here
 * @return   void
 */
void task_init(void)
{
    app_task_init();
}

/**
 * @brief    Entry of APP code
 * @return   int (To avoid compile warning)
 */
int main(void)
{
    extern uint32_t random_seed_value;
    srand(random_seed_value);
    board_init();
    driver_init();
    le_gap_init(APP_MAX_LINKS);
    gap_lib_init();
    app_le_gap_init();
    app_le_profile_init();
    mesh_stack_init();
#if GROUP_LIGHT_SUPPORTED
    light_controller_init();
    group_light_init();
#endif
    pwr_mgr_init();
    task_init();
    os_sched_start();

    return 0;
}

#include "otp_config.h"
#ifdef BT_STACK_CONFIG_ENABLE
#include "app_section.h"
#include "gap_config.h"
APP_FLASH_TEXT_SECTION void bt_stack_config_init(void)
{
    gap_config_bt_report_buf_num(8);
}
#endif

