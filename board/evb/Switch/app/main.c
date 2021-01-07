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

#include "mesh_api.h"
#include "mesh_sdk.h"
#include "mesh_cmd.h"
#include "mem_config.h"
#include "device_app.h"
#include "health.h"
#include "ping.h"
#include "ping_app.h"
#include "tp.h"
#include "ota_server.h"
#include "dfu_server.h"
#include "dfu_client.h"
#include "health.h"
#include "datatrans_server_app.h"

#include "dlps.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_io_dlps.h"
#include "rtl876x_gpio.h"

#include "app_hwtmr.h"
#include "soft_wdt.h"
#include "app_skyiot_server.h"
#include "app_skyiot_dlps.h"

mesh_model_info_t health_server_model;

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
//    log_module_bitmap_trace_set(0xFFFFFFFFFFFFFFFF, LEVEL_WARN, 0);
//    log_module_bitmap_trace_set(0xFFFFFFFFFFFFFFFF, LEVEL_ERROR, 0);
    log_module_trace_set(MODULE_LOWERSTACK, LEVEL_ERROR, 0);
    log_module_trace_set(MODULE_SNOOP, LEVEL_ERROR, 0);
	
	
//    log_module_trace_set(MODULE_APP, LEVEL_WARN, 1);
    log_module_trace_set(MODULE_DFU, LEVEL_TRACE, 1);
	
    /** set mesh stack log level, default all on, disable the log of level LEVEL_TRACE */
    uint32_t module_bitmap[MESH_LOG_LEVEL_SIZE] = {0};
    diag_level_set(LEVEL_TRACE, module_bitmap);
	
    /** print the mesh sdk & lib version */
    mesh_sdk_version();

    /** mesh stack needs rand seed */
    plt_srand(platform_random(0xffffffff));

    /** set device name and appearance */
    char *dev_name = "Mesh Device";
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    gap_sched_params_set(GAP_SCHED_PARAMS_DEVICE_NAME, dev_name, GAP_DEVICE_NAME_LEN);
    gap_sched_params_set(GAP_SCHED_PARAMS_APPEARANCE, &appearance, sizeof(appearance));
#if GAP_SCHED_BT5_AE
    bool ae = true;
    gap_sched_params_set(GAP_SCHED_PARAMS_BT5_AE, &ae, sizeof(ae));
#endif

    /** set device uuid according to bt address */
    // uint8_t bt_addr[6];
    uint8_t dev_uuid[16] = MESH_DEVICE_UUID;
    // gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);
    // memcpy(dev_uuid, bt_addr, sizeof(bt_addr));	

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
        .proxy = 0,
        .fn = 0,
        .lpn = 0,
        .prov = 0,
        .udb = 1,
        .snb = 1,
        .bg_scan = 1,
        .flash = 1,
        .flash_rpl = 0
    };
    mesh_node_cfg_t node_cfg =
    {
        .dev_key_num = 1,
        .net_key_num = 1,
        .app_key_num = 1,
        .vir_addr_num = 1,
        .rpl_num = 20,
        .sub_addr_num = 10,
        .proxy_num = 1,
		
			  .udb_interval = 10,//  default MESH_UDB_PERIOD
			  .snb_interval = 100,//  default MESH_SNB_PERIOD
//			  .prov_interval =1,// mesh GATT service
//			  .proxy_interval =5,// mesh GATT service
    };
    mesh_node_cfg(features, &node_cfg);
    mesh_node.pb_adv_retrans_count = 6;
    mesh_node.pb_adv_retrans_steps = 1;
	
    mesh_node.net_trans_count = 6;
    mesh_node.relay_retrans_count = 4;
    mesh_node.trans_retrans_count = 7;

    /** create elements and register models */
    mesh_element_create(GATT_NS_DESC_UNKNOWN);
    health_server_reg(0, &health_server_model);
    health_server_set_company_id(&health_server_model, COMPANY_ID);
	
	// SkyBleMesh_Vendormodel_init(0);

    compo_data_page0_header_t compo_data_page0_header = {COMPANY_ID, PRODUCT_ID, VERSION_ID};
    compo_data_page0_gen(&compo_data_page0_header);
	
	// SkyBleMesh_App_Init();
	// SkyBleMesh_Get_UUID(dev_uuid, MESH_DEV_UUID_LEN);	// qlj 整理
    // device_uuid_set(dev_uuid);
	
    /** init mesh stack */
    mesh_init();	
	// mesh_model_bind_all_key();
	
    /** register proxy adv callback */
    device_info_cb_reg(device_info_cb);
    // hb_init(hb_cb);
	
	
	// 为了降低功耗
	#if 1
	gap_sched_scan(false); 
	#else
	gap_sched_scan(false); 
    uint16_t scan_interval = 0x1C0;  //!< 280ms
	uint16_t scan_window   = 0x30; //!< 30ms
	gap_sched_params_set(GAP_SCHED_PARAMS_SCAN_INTERVAL, &scan_interval, sizeof(scan_interval));
	gap_sched_params_set(GAP_SCHED_PARAMS_SCAN_WINDOW, &scan_window, sizeof(scan_window));
	gap_sched_scan(true); 
	#endif
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
    uint16_t scan_interval = 0x1C0;  //!< 280ms     500ms
    uint16_t scan_window   = 0x30; //!< 30 30ms     08 20ms
    gap_sched_params_set(GAP_SCHED_PARAMS_SCAN_INTERVAL, &scan_interval, sizeof(scan_interval));
    gap_sched_params_set(GAP_SCHED_PARAMS_SCAN_WINDOW, &scan_window, sizeof(scan_window));
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
	
	#if SKYMESH_FACTORYTEST_EABLE==1
	Regist_ProductFactoryCheck_cb(SkyMesh_ProductFactoryCheck_cb);
	#endif

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
    server_init(MESH_GATT_SERVER_COUNT + 3);
    /* Add Server Module */
    ota_server_add(NULL);
    dfu_server_add(app_profile_callback);

    /* Register Server Callback */
    server_register_app_cb(app_profile_callback);

    client_init(MESH_GATT_CLIENT_COUNT + 1);
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
#include "skydisplay.h"
void board_init(void)
{
	// RTL8762_IIC_Init();
}

/**
 * @brief    Contains the initialization of peripherals
 * @note     Both new architecture driver and legacy driver initialization method can be used
 * @return   void
 */
void driver_init(void)
{	
	#if USE_SOFT_WATCHDOG
	Hal_Timer_init();
	OS_WDTInit();
	#endif
//	 gap_sched_scan(true); 
//     beacon_start();
//	 SkyBleMesh_Batterval_Lightsense(true);
	 uint8_t batt_station = SkyBleMesh_Batt_Station();
	// 要在 mesh_stack_init后获取，后面整理下
    mesh_node_state_t node_state = mesh_node_state_restore();
    if (node_state == UNPROV_DEVICE){
        if(batt_station == BATT_NORMAL){
        beacon_start();
		gap_sched_scan(true); 
        DBG_DIRECT("----------UNPORV--gap_sched_params_set\r\n------\r\n");
		if(SkyBleMesh_Device_Active_Sate()==true){
			SkyBleMesh_Unprov_timer();
		}
        }else{
            SkyBleMesh_unBind_complete();
            DBG_DIRECT("NO Network");
        }
    }else{
        SkyBleMesh_ChangeScan_timer(1);
    }
		 
}

#if ENABLE_DLPS

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
	 DBG_DIRECT("En DLPS \r\n");
		
	SkyBleMesh_EnterDlps_cfg();
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
	 DBG_DIRECT("Exit DLPS\r\n");
//     DBG_DIRECT("wakeup =%d----\r\n",sky_findPin());
	
	SkyBleMesh_ExitDlps_cfg(true);
	
}

/**
 * @brief app_dlps_check_cb() contains the setting about app dlps callback.
*/
bool app_dlps_check_cb(void)
{
    return switch_check_dlps_statu();
}
#if 0
void System_Handler(void)
{
	DBG_DIRECT("Exit System_Handler1 \r\n");
	
	DBG_DIRECT("System_Handler %d\r\n",GPIO_ReadInputDataBit(GPIO_GetPin(LPN_BUTTON)));	
    if (System_WakeUpInterruptValue(P2_4) == SET)
    {
		DBG_DIRECT("Exit System_Handler2 \r\n");
        Pad_ClearWakeupINTPendingBit(P2_4);
        System_WakeUpPinDisable(P2_4);  // exit中加了就不会进中断
        
    }
}
#endif

#endif
/**
 * @brief    Contains the power mode settings
 * @return   void
 */
void pwr_mgr_init(void)
{
#if ENABLE_DLPS
    if (false == dlps_check_cb_reg(app_dlps_check_cb))
    {
        APP_PRINT_ERROR0("Error: dlps_check_cb_reg(app_dlps_check_cb) failed!");
    }
    DLPS_IORegUserDlpsExitCb(app_exit_dlps_config);
    DLPS_IORegUserDlpsEnterCb(app_enter_dlps_config);
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
void app_normal_power_on_seq(void)
{
    extern uint32_t random_seed_value;
    srand(random_seed_value);
    board_init();
    le_gap_init(APP_MAX_LINKS);
    gap_lib_init();
    app_le_gap_init();
    app_le_profile_init();
    mesh_stack_init();
    driver_init();
	
    pwr_mgr_init();
    task_init();
//    os_sched_start();

//    return 0;
}

// module test
#if MP_TEST_MODE_SUPPORT_DATA_UART_TEST
#include "test_mode.h"
#include "single_tone.h"
#endif
T_TEST_MODE   test_mode_value;

int main(void)
{
	test_mode_value = get_test_mode(); 
	reset_test_mode();
	switch (test_mode_value) {
		case NOT_TEST_MODE:{
			app_normal_power_on_seq(); 
		}
		break;
		#if MP_TEST_MODE_SUPPORT_DATA_UART_TEST
		case SINGLE_TONE_MODE: {
			single_tone_init();
		}
		break;
		case DATA_UART_TEST_MODE:{
		#if MP_TEST_MODE_SUPPORT_AUTO_K_RF
			printi("DATA_UART_TEST_MODE");
			app_normal_power_on_seq();
		#endif 
		}
		break;
		#endif
		
		default:
		break;
	}
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
#if GAP_SCHED_BT5_AE
    gap_config_ae_parameter();
#endif
}
#endif

