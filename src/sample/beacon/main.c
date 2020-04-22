/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      main.c
   * @brief     Source file for BLE peripheral project, mainly used for initialize modules
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
#include <trace.h>
#include <board.h>
#include <gap.h>
#include <gap_adv.h>
#include <gap_bond_le.h>
#include <profile_server.h>
#include <gap_msg.h>
#include "beacon_sample_service.h"
#include <bas.h>
#include <app_task.h>
#include "beacon_app.h"
#include <rtl876x_gpio.h>
#include "rtl876x_pinmux.h"
#include "rtl876x_nvic.h"
#include <gpio_handle.h>
#if F_BT_DLPS_EN
#include <dlps.h>
#include <rtl876x_io_dlps.h>
#endif


/** @defgroup  PERIPH_DEMO_MAIN Peripheral Main
    * @brief Main file to initialize hardware and BT stack and start task scheduling
    * @{
    */

/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @brief  Default minimum advertising interval when device is discoverable (units of 625us, 160=100ms) */
#define DEFAULT_ADVERTISING_INTERVAL_MIN            320
/** @brief  Default maximum advertising interval */
#define DEFAULT_ADVERTISING_INTERVAL_MAX            320


/*============================================================================*
 *                              Variables
 *============================================================================*/
bool allowedEnterDlps = true;
/** @brief  GAP - scan response data (max size = 31 bytes) */
static const uint8_t scan_rsp_data[] =
{
    0x03,                             /* length */
    GAP_ADTYPE_APPEARANCE,            /* type="Appearance" */
    LO_WORD(GAP_GATT_APPEARANCE_UNKNOWN),
    HI_WORD(GAP_GATT_APPEARANCE_UNKNOWN),
};

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */
#if defined(DEVICE_CONNECTABLE)
uint8_t adv_data[] =
{
    /* Flags */
    0x02,             /* length */
    GAP_ADTYPE_FLAGS, /* type="Flags" */
    GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    /* Service */
    0x1A,               /* 4:length     */
    GAP_ADTYPE_MANUFACTURER_SPECIFIC,          /* 5:type="GAP_ADTYPE_MANUFACTURER_SPECIFIC" */
    0x5D, 0x00,         /*realtek company Id*/
//    0x4C, 0x00,         /* 6-7: Apple company Id*/
    0x02, 0x15,         /* 8-9:For all proximity beacon,specify data type & remaining data length*/

    //Wechat test uuid:FDA50693-A4E2-4FB1-AFCF-C6EB07647825, Major: 10, Minor: 7
    0xFD, 0xA5, 0x06, 0x93,
    0xA4, 0xE2, 0x4F, 0xB1,
    0xAF, 0xCF, 0xC6, 0xEB,
    0x07, 0x64, 0x78, 0x25,
    0x00, 0x0A,         /* 26-27:major id*/
    0x00, 0x07,         /* 28-29:minor id*/
    0xC3
};
#else
static const uint8_t adv_data[] =
{
    /* Flags */
    0x02,             /* length */
    GAP_ADTYPE_FLAGS, /* type="Flags" */
    GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    /* Service */
    0x1A,               /* 4:length     */
    GAP_ADTYPE_MANUFACTURER_SPECIFIC,          /* 5:type="GAP_ADTYPE_MANUFACTURER_SPECIFIC" */
    0x5D, 0x00,         /*realtek company Id*/
    0x02, 0x15,         /* 8-9:For all proximity beacon,specify data type & remaining data length*/

    //Wechat test uuid:FDA50693-A4E2-4FB1-AFCF-C6EB07647825, Major: 10, Minor: 7
    0xFD, 0xA5, 0x06, 0x93,
    0xA4, 0xE2, 0x4F, 0xB1,
    0xAF, 0xCF, 0xC6, 0xEB,
    0x07, 0x64, 0x78, 0x25,
    0x00, 0x0A,         /* 26-27:major id*/
    0x00, 0x07,         /* 28-29:minor id*/
    0xC3
};
#endif
/*============================================================================*
 *                              Functions
 *============================================================================*/
/**
  * @brief  Initialize peripheral and gap bond manager related parameters
  * @return void
  */
void app_le_gap_init(void)
{
    /* Device name and device appearance */
    uint8_t  device_name[GAP_DEVICE_NAME_LEN] = "BLE_BEACON";
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    uint8_t  slave_init_mtu_req = false;


    /* Advertising parameters */
#if defined(DEVICE_CONNECTABLE)
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_NONCONN_IND;
    //uint8_t  adv_evt_type = GAP_ADTYPE_ADV_IND;
#else
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_NONCONN_IND;
#endif
    uint8_t  adv_direct_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  adv_direct_addr[GAP_BD_ADDR_LEN] = {0};
    uint8_t  adv_chann_map = GAP_ADVCHAN_ALL;
    uint8_t  adv_filter_policy = GAP_ADV_FILTER_ANY;
    uint16_t adv_int_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint16_t adv_int_max = DEFAULT_ADVERTISING_INTERVAL_MAX;

    /* GAP Bond Manager parameters */
    uint8_t  auth_pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint16_t auth_flags = GAP_AUTHEN_BIT_BONDING_FLAG;
    uint8_t  auth_io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8_t  auth_oob = false;
    uint8_t  auth_use_fix_passkey = false;
    uint32_t auth_fix_passkey = 0;
#if F_BT_ANCS_CLIENT_SUPPORT
    uint8_t  auth_sec_req_enable = true;
#else
    uint8_t  auth_sec_req_enable = false;
#endif
    uint16_t auth_sec_req_flags = GAP_AUTHEN_BIT_BONDING_FLAG;

    /* Set device name and device appearance */
    le_set_gap_param(GAP_PARAM_DEVICE_NAME, GAP_DEVICE_NAME_LEN, device_name);
    le_set_gap_param(GAP_PARAM_APPEARANCE, sizeof(appearance), &appearance);
    le_set_gap_param(GAP_PARAM_SLAVE_INIT_GATT_MTU_REQ, sizeof(slave_init_mtu_req),
                     &slave_init_mtu_req);

    /* Set advertising parameters */
    le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_evt_type), &adv_evt_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR_TYPE, sizeof(adv_direct_type), &adv_direct_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR, sizeof(adv_direct_addr), adv_direct_addr);
    le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_chann_map), &adv_chann_map);
    le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_min), &adv_int_min);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_max), &adv_int_max);
    le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data), (void *)adv_data);
    le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sizeof(scan_rsp_data), (void *)scan_rsp_data);

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
}

/**
 * @brief  Add GATT services and register callbacks
 * @return void
 */
void app_le_profile_init(void)
{
#if defined(DEVICE_CONNECTABLE)
    server_init(2);
    beacon_srv_id = beacon_sample_add_service(app_profile_callback);
    bas_srv_id  = bas_add_service(app_profile_callback);
    server_register_app_cb(app_profile_callback);
#else
    server_init(1);
    bas_srv_id  = bas_add_service(app_profile_callback);
    server_register_app_cb(app_profile_callback);
#endif
}


/**
 * @brief    rcc configuration
 * @return   void
 */
void RCC_Configuration(void)
{
    /* Enable GPIO clock */

    return;
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
 * @brief    Configuration of PADs when enter dlps
 * @note     enter dlps to set pad&wake up pin
 * @return   void
 */
void enterDlpsSet(void)
{

    Pad_Config(Int_Pin, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(DATA_UART_TX_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(DATA_UART_RX_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    System_WakeUpDebounceTime(0x8);
    if (GPIO_ReadInputDataBit(GPIO_Int_Pin))
    {
        System_WakeUpPinEnable(Int_Pin, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_ENABLE);
    }
    else
    {
        System_WakeUpPinEnable(Int_Pin, PAD_WAKEUP_POL_HIGH, PAD_WK_DEBOUNCE_ENABLE);
    }
}
/**
 * @brief    Configuration of PADs when exit dlps
 * @note     Exit dlps to configure pad
 * @return   void
 */
void exitDlpsInit(void)
{
    Pad_Config(Int_Pin, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(DATA_UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(DATA_UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
}

/**
 * @brief    DLPS_PxpCheck
 * @note     check app enter dlps flg.
 * @return   bool
 */
bool DLPS_Check(void)
{
    return allowedEnterDlps;
}

/**
 * @brief    Contains the power mode settings and dlps register
 * @return   void
 */
void pwr_mgr_init(void)
{
    if (false == dlps_check_cb_reg(DLPS_Check))
    {
        DBG_DIRECT("Error: dlps_check_cb_reg(DLPS_RcuCheck) failed!\n");
    }
    DLPS_IORegUserDlpsEnterCb(enterDlpsSet);
    DLPS_IORegUserDlpsExitCb(exitDlpsInit);
    DLPS_IORegister();
    lps_mode_set(LPM_DLPS_MODE);
}

void System_Handler(void)
{
    uint8_t tmpVal;
    APP_PRINT_INFO0("System_Handler");
    NVIC_DisableIRQ(System_IRQn);
    if (GPIO_ReadInputDataBit(GPIO_Int_Pin) == 0)
    {
        GPIO_Int_Handler();
    }
    // need clear debounce bit here.
    tmpVal = btaon_fast_read_safe(0x2b);
    btaon_fast_write_safe(0x2b, (tmpVal | BIT7));
    NVIC_ClearPendingIRQ(System_IRQn);
}

/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Peripheral APP, thus only one APP task is init here
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
    le_gap_init(APP_MAX_LINKS);
    gap_lib_init();
    app_le_gap_init();
    app_le_profile_init();
    pwr_mgr_init();
    task_init();
    os_sched_start();

    return 0;
}
/** @} */ /* End of group PERIPH_DEMO_MAIN */


