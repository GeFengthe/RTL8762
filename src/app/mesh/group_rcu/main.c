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
#include "board.h"
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
#include "gap_vendor.h"
#if DLPS_EN
#include "dlps.h"
#include "rtl876x_io_dlps.h"
#endif
#include "mesh_api.h"
#include "mesh_sdk.h"
#include "health.h"
#include "ping.h"
#include "ping_app.h"
#include "rcu_app.h"
#include "dimmable_light.h"
#include "light_cwrgb_server_app.h"
#include "light_hsl_server_app.h"

#include "ota_server.h"
#include "dfu_server.h"
#include "mem_config.h"
#include "rtl876x_keyscan.h"
#include "key_handle.h"
#include "rcu_swtimer.h"
#include "keyscan_driver.h"
#ifdef BAT_EN
#include "battery_driver.h"
#endif
#if LED_EN
#include "led_driver.h"
#endif

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
    char *dev_name = "Group RCU";
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    gap_sched_params_set(GAP_SCHED_PARAMS_DEVICE_NAME, dev_name, GAP_DEVICE_NAME_LEN);
    gap_sched_params_set(GAP_SCHED_PARAMS_APPEARANCE, &appearance, sizeof(appearance));

    /** init mesh stack */
    mesh_init();
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
    //dfu_client_add(NULL);

    /* Register Client Callback--App_ClientCallback to handle events from Profile Client layer. */
    client_register_general_client_cb(app_client_callback);
}

/**
* @brief  global_data_init() contains the initialization of global data.
*/
void global_data_init(void)
{
    keyscan_init_data();
    key_handle_init_data();
}

/**
 * @brief app_pinmux_config() contains the initialization of app pinmux config.
 * @return void
 */
void app_pinmux_config(void)
{
    keyscan_pinmux_config();
}

/**
 * @brief app_pad_config() contains the initialization of app pad config.
 * @return void
 */
void app_pad_config(void)
{
    keyscan_init_pad_config();
}

/**
 * @brief app_nvic_config() contains the initialization of app NVIC config.
 * @return void
 */
void app_nvic_config(void)
{
    keyscan_nvic_config();
#ifdef BAT_EN
    rcu_bat_nvic_config();
#endif
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
    app_pad_config();
    app_pinmux_config();
}

/**
 * @brief    Contains the initialization of peripherals
 * @note     Both new architecture driver and legacy driver initialization method can be used
 * @return   void
 */
void driver_init(void)
{
    keyscan_init_driver(KeyScan_Debounce_Enable);
    /*rcu battery module init*/
    rcu_bat_init();
#if LED_EN
    led_module_init();
#endif
}

#if DLPS_EN
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
    keyscan_enter_dlps_config();
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
    keyscan_exit_dlps_config();
}

/**
 * @brief app_dlps_check_cb() contains the setting about app dlps callback.
*/
bool app_dlps_check_cb(void)
{
    return (
               keyscan_global_data.is_allowed_to_enter_dlps
#ifdef BAT_EN
               &&  bat_allow_enter_dlps()
#endif
           );
}
#endif

void System_Handler(void)
{
    APP_PRINT_INFO0("[app_system_on_interrupt_handler] sytem on interrupt");

    uint8_t tmpVal;
    NVIC_DisableIRQ(System_IRQn);
    tmpVal = btaon_fast_read_safe(0x2b);
    if (tmpVal & BIT7)
    {
        /* pad signal wake up event */
        btaon_fast_write_safe(0x2b, (tmpVal | BIT7));
        keyscan_init_driver(KeyScan_Debounce_Disable);
        keyscan_global_data.is_allowed_to_enter_dlps =
            false;  /* not allow to enter DLPS before interrupt or timeout */
        /* Set flag and start timer to handle unexpection condition:
           1. Not Keyscan row PAD wake up system;
           2. PAD wake up signal disappear before Keyscan works; */
        keyscan_global_data.is_key_pressed = true;
        if (!os_timer_restart(&keyscan_timer, KEYSCAN_SW_INTERVAL))
        {
            APP_PRINT_ERROR0("[System_Handler] keyscan_timer restart failed!");
            keyscan_global_data.is_allowed_to_enter_dlps = true;
        }

    }
    NVIC_ClearPendingIRQ(System_IRQn);
}

/**
 * @brief    Contains the power mode settings
 * @return   void
 */
void pwr_mgr_init(void)
{
#if DLPS_EN
    if (false == dlps_check_cb_reg(app_dlps_check_cb))
    {
        APP_PRINT_ERROR0("Error: dlps_check_cb_reg(app_dlps_check_cb) failed!");
    }
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
    global_data_init();
    board_init();
    le_gap_init(APP_MAX_LINKS);
    gap_lib_init();
    app_le_gap_init();
    app_le_profile_init();
    mesh_stack_init();
    pwr_mgr_init();
    task_init();
    sw_timer_init();
    os_sched_start();

    return 0;
}
