/**
*****************************************************************************************
*     Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      main.c
   * @brief     Source file for BLE BT5 peripheral project, mainly used for initialize modules
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
#include "os_sched.h"
#if F_BT_DLPS_EN
#include "dlps.h"
#include "rtl876x_io_dlps.h"
#endif
#include "trace.h"
#include "gap.h"
#include "gap_bond_le.h"
#include "gap_config.h"
#include "gap_ext_adv.h"
#include "app_task.h"
#include "bt5_peripheral_app.h"
#include "bt5_peripheral_stack_api.h"
#include "otp_config.h"

/** @defgroup  BT5_PERIPH_DEMO_MAIN BT5 Peripheral Main
    * @brief Main file to initialize hardware and BT stack and start task scheduling
    * @{
    */
/*============================================================================*
 *                              Functions
 *============================================================================*/
/**
  * @brief  Config bt stack related feature
  * @return void
  */
#ifdef BT_STACK_CONFIG_ENABLE
#include "app_section.h"

APP_FLASH_TEXT_SECTION void bt_stack_config_init(void)
{
    gap_config_ae_parameter();
}
#endif

/**
  * @brief  Initialize peripheral, gap bond manager and LE Advertising Extensions
            related parameters
  * @return void
  */
void app_le_gap_init(void)
{
    /* Device name and device appearance */
    uint8_t  device_name[GAP_DEVICE_NAME_LEN] = "BLE_BT5_Peripheral";
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    uint8_t  slave_init_mtu_req = false;

    uint16_t max_data_len_tx_oct = 251;
#if LE_CODED_PHY_S8
    uint16_t max_data_len_tx_time = 17040;
#else
    uint16_t max_data_len_tx_time = 4542;
#endif

    /* GAP Bond Manager parameters */
    uint8_t  auth_pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint16_t auth_flags = GAP_AUTHEN_BIT_BONDING_FLAG | GAP_AUTHEN_BIT_SC_FLAG;
    T_GAP_IO_CAP  auth_io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8_t  auth_oob = false;
    uint8_t  auth_use_fix_passkey = false;
    uint32_t auth_fix_passkey = 0;
    uint8_t  auth_sec_req_enable = false;
    uint16_t auth_sec_req_flags = GAP_AUTHEN_BIT_BONDING_FLAG | GAP_AUTHEN_BIT_SC_FLAG;

    /* LE Advertising Extensions parameters */
    bool use_extended = true;

    /* Set device name and device appearance */
    le_set_gap_param(GAP_PARAM_DEVICE_NAME, GAP_DEVICE_NAME_LEN, device_name);
    le_set_gap_param(GAP_PARAM_APPEARANCE, sizeof(appearance), &appearance);
    le_set_gap_param(GAP_PARAM_SLAVE_INIT_GATT_MTU_REQ, sizeof(slave_init_mtu_req),
                     &slave_init_mtu_req);

    le_set_gap_param(GAP_PARAM_DEFAULT_DATA_LEN_MAX_TX_OCTETS, sizeof(max_data_len_tx_oct),
                     &max_data_len_tx_oct);
    le_set_gap_param(GAP_PARAM_DEFAULT_DATA_LEN_MAX_TX_TIME, sizeof(max_data_len_tx_time),
                     &max_data_len_tx_time);

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

    /* Use LE Advertising Extensions */
    le_set_gap_param(GAP_PARAM_USE_EXTENDED_ADV, sizeof(use_extended), &use_extended);

    /* Initialize extended advertising related parameters */
    le_init_ext_adv_params_ext_conn();

    /* Register gap message callback */
    le_register_app_cb(app_gap_callback);
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

}

/**
 * @brief    Contains the initialization of peripherals
 * @note     Both new architecture driver and legacy driver initialization method can be used
 * @return   void
 */
void driver_init(void)
{

}

/**
 * @brief    Contains the power mode settings
 * @return   void
 */
void io_dlps_enter_cb(void)
{

}

void io_dlps_exit_cb(void)
{

}

void pwr_mgr_init(void)
{
#if F_BT_DLPS_EN
    DLPS_IORegUserDlpsEnterCb(io_dlps_enter_cb);
    DLPS_IORegUserDlpsExitCb(io_dlps_exit_cb);
    DLPS_IORegister();
    lps_mode_set(LPM_DLPS_MODE);
#endif
}

/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BT5 BLE Peripheral APP, thus only one APP task is init here
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
    le_gap_init(APP_MAX_LINKS);
    gap_lib_init();
    app_le_gap_init();
    pwr_mgr_init();
    task_init();
    os_sched_start();

    return 0;
}
/** @} */ /* End of group BT5_PERIPH_DEMO_MAIN */


