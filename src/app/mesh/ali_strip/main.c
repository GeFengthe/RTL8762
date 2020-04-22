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
#include "board.h"
#if LIGHT_DLPS_EN
#include "light_dlps_ctrl.h"
#include "dlps.h"
#include "rtl876x_io_dlps.h"
#include "rtl876x_pinmux.h"
#endif
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

#include "mesh_api.h"
#include "health.h"
#include "main_app.h"
#include "light_controller_app.h"
#include "light_cwrgb_app.h"
#include "light_storage_app.h"
#include "light_config.h"
#include "dfu_updater_app.h"

#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"

#include "light_lightness_server_app.h"
#include "light_ctl_server_app.h"
#include "light_hsl_server_app.h"
#include "light_ctl_hsl_server_app.h"
#include "light_cwrgb_server_app.h"

#include "strip_app.h"

#include "ota_server.h"
#include "dfu_server.h"
#include "dfu_client.h"
#include "mem_config.h"
#include "user_data.h"
#if ALI_AIS_SUPPORT
#include "ais.h"
#include "dis.h"
#endif

static mesh_model_info_t health_server_model;

//void light_init(uint8_t light_type)
//{
//    switch (light_type)
//    {
//    case LIGHT_LIGHTNESS:
//        light_lightness_server_models_init(0);
//        break;
//    case LIGHT_CW:
//        light_ctl_server_models_init(0);
//        break;
//    case LIGHT_RGB:
//        light_hsl_server_models_init(0);
//        break;
//    case LIGHT_CWRGB:
//        light_ctl_hsl_server_models_init(0);
//        break;
//    default:
//        break;
//    }
//    light_cwrgb_server_models_init();
//}

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

    /** mesh stack needs rand seed */
    plt_srand(platform_random(0xffffffff));

    /** set device name and appearance */
    char *dev_name = "Mesh Ali Light";
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    gap_sched_params_set(GAP_SCHED_PARAMS_DEVICE_NAME, dev_name, GAP_DEVICE_NAME_LEN);
    gap_sched_params_set(GAP_SCHED_PARAMS_APPEARANCE, &appearance, sizeof(appearance));

    /** set device uuid */
    uint32_t product_id = ALI_PRODUCT_ID;
    if (user_data_contains_ali_data())
    {
        product_id = user_data_read_ali_product_id();
    }
    ali_uuid_t dev_uuid;
    dev_uuid.cid = 0x01A8; //!< taobao
    dev_uuid.pid.adv_ver = 1;
    dev_uuid.pid.sec = 1;
    dev_uuid.pid.ota = 1;
    dev_uuid.pid.bt_ver = 1;
    dev_uuid.product_id = product_id;
    uint8_t bt_addr[6];
    gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);
    memcpy(dev_uuid.mac_addr, bt_addr, sizeof(bt_addr));
    dev_uuid.feature.silent_adv = 0;
    dev_uuid.feature.uuid_ver = 1;
    memset(dev_uuid.rfu, 0, sizeof(dev_uuid.rfu));
    device_uuid_set((uint8_t *)&dev_uuid);

    /** configure provisioning parameters */
    prov_capabilities_t prov_capabilities =
    {
        .algorithm = PROV_CAP_ALGO_FIPS_P256_ELLIPTIC_CURVE,
        .public_key = 0,
        .static_oob = PROV_CAP_STATIC_OOB,
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
        .fn = 1,
        .lpn = 0,
        .prov = 1,
        .udb = 1,
        .snb = 1,
        .bg_scan = 1,
        .flash = 1,
        .flash_rpl = 0
    };
    mesh_node_cfg_t node_cfg =
    {
        .dev_key_num = 1,
        .net_key_num = 3,
        .app_key_num = 3,
        .vir_addr_num = 3,
        .rpl_num = 100,
        .sub_addr_num = 10,
        .proxy_num = 1,
        .udb_interval = 1,
        .prov_interval = 10,
        .proxy_interval = 10,
        .identity_interval = 20
    };
    mesh_node_cfg(features, &node_cfg);
    mesh_node.net_trans_count = 5;
    mesh_node.relay_retrans_count = 5;
    mesh_node.trans_retrans_count = 7;
    uint8_t udb_retrans_count = 4;
    uint16_t udb_retrans_interval = 10;
    beacon_cfg(BEACON_CFG_TYPE_UDB_RETRANS_COUNT, &udb_retrans_count, sizeof(udb_retrans_count));
    beacon_cfg(BEACON_CFG_TYPE_UDB_RETRANS_INTERVAL, (uint8_t *)&udb_retrans_interval,
               sizeof(udb_retrans_interval));
    uint8_t prov_adv_retrans_count = 4;
    uint16_t prov_adv_retrans_interval = 10;
    mesh_service_adv_cfg(MESH_SERVCIE_ADV_CFG_TYPE_PROV_RETRANS_COUNT, &prov_adv_retrans_count,
                         sizeof(prov_adv_retrans_count));
    mesh_service_adv_cfg(MESH_SERVCIE_ADV_CFG_TYPE_PROV_RETRANS_INTERVAL,
                         (uint8_t *)&prov_adv_retrans_interval, sizeof(prov_adv_retrans_interval));

    /** create elements and register models */
    mesh_element_create(GATT_NS_DESC_UNKNOWN);
    health_server_reg(0, &health_server_model);
    health_server_set_company_id(&health_server_model, COMPANY_ID);
    dfu_updater_models_init();
    /* add elements according to the light type */
    //light_init(LIGHT_TYPE);
    strip_server_models_init(0);
    compo_data_page0_header_t compo_data_page0_header = {COMPANY_ID, PRODUCT_ID, VERSION_ID};
    compo_data_page0_gen(&compo_data_page0_header);

    /** restore light ahead since it may restore the fatory setting */
    light_flash_restore();

#if DFU_UPDATER_SUPPORT_POWER_OFF_GO_ON
    dfu_updater_load();
#endif

    /** init mesh stack */
    mesh_init();
    fn_params_t fn_params = {10};
    fn_init(1, &fn_params, fn_cb);

    mesh_model_bind_all_key();
    //light_ctl_server_models_sub();
    strip_server_models_sub();
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
#if ALI_AIS_SUPPORT
    server_init(MESH_GATT_SERVER_COUNT + 4);
    ais_server_add(app_profile_callback);
    dis_server_id = dis_add_service(app_profile_callback);
#else
    server_init(MESH_GATT_SERVER_COUNT + 2);
#endif
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
#if (LIGHT_TYPE == LIGHT_LIGHTNESS) || (MESH_ALI_CERTIFICATION)
    light_controller_init();
#endif

    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
    Pinmux_Config(CTRL_PIN, DWGPIO);
    Pad_Config(CTRL_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE, PAD_OUT_LOW);
}

/**
 * @brief    Contains the initialization of peripherals
 * @note     Both new architecture driver and legacy driver initialization method can be used
 * @return   void
 */
void driver_init(void)
{
    GPIO_InitTypeDef GPIO_Struct;
    GPIO_StructInit(&GPIO_Struct);

    GPIO_Struct.GPIO_DebounceTime = 20;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Struct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_HIGH;
    GPIO_Struct.GPIO_ITTrigger = GPIO_INT_Trigger_EDGE;
    GPIO_Struct.GPIO_Pin = GPIO_CTRL_PIN;
    GPIO_Struct.GPIO_ITCmd = ENABLE;
    GPIO_Init(&GPIO_Struct);
    GPIO_MaskINTConfig(GPIO_CTRL_PIN, DISABLE);
    GPIO_INTConfig(GPIO_CTRL_PIN, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GPIO_CTRL_PIN_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}




#if LIGHT_DLPS_EN
/**
 * @brief this function will be called before enter DLPS
 *
 *  set PAD and wakeup pin config for enterring DLPS
 *
 * @param none
 * @return none
 * @retval void
*/
void app_enter_dlps_config(void)
{
    DBG_DIRECT("En DLPS");
    light_cwrgb_enter_dlps();
}

/**
 * @brief this function will be called after exit DLPS
 *
 *  set PAD and wakeup pin config for enterring DLPS
 *
 * @param none
 * @return none
 * @retval void
*/
void app_exit_dlps_config(void)
{

}

/**
 * @brief app_dlps_check_cb() contains the setting about app dlps callback.
*/
bool app_dlps_check_cb(void)
{
    return light_check_dlps();
}
#endif

/**
 * @brief    Contains the power mode settings
 * @return   void
 */
void pwr_mgr_init(void)
{
#if LIGHT_DLPS_EN
    if (false == dlps_check_cb_reg(app_dlps_check_cb))
    {
        APP_PRINT_ERROR0("Error: dlps_check_cb_reg(app_dlps_check_cb) failed!");
    }
    DLPS_IORegUserDlpsEnterCb(app_enter_dlps_config);
    DLPS_IORegUserDlpsExitCb(app_exit_dlps_config);
    DLPS_IORegister();
    lps_mode_set(LPM_DLPS_MODE);
    light_dlps_ctrl_init();
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
    board_init();
    driver_init();
    le_gap_init(APP_MAX_LINKS);
    gap_lib_init();
    app_le_gap_init();
    app_le_profile_init();
    mesh_stack_init();
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

