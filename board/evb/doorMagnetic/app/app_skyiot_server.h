/**
 ****************************************************************************************
 *
 * @file app_skyiot_server.h
 *
 * @brief mesh Application Module entry point
 *
 * @auth  
 *
 * @date  
 *
 * Copyright (C) 
 *
 *
 ****************************************************************************************
 */

#ifndef _APP_SKYIOT_SERVER_H
#define _APP_SKYIOT_SERVER_H

#include "app_msg.h"
#include "skyswitch.h"
#include "skylight.h"
#include "skyinf.h"
#include "device_app.h"

#define MESH_TEST_PRESSURE       		0

#if (SKY_SWITCH_TYPE == SKY_NIGHTLIGHT_TYPE)
#define PRODUCT_TYPE    				(123)		// TODO 淇敼涓哄皬澶滅伅:118
#define PRODUCT_MODEL   				("RBDNM01") 	// LCAM01	
#define PRODUCT_BRAND   				(11)	    // swaiot
#define PRODUCT_VERSION 				("1.0.01")      // 娉ㄦ剰鍚屾缁欐帴鍙?uart_test_read_app_version  鍜?version.h涓殑 VERSION_MAJOR
#endif

// #define MESH_ADV_NAME_LEN (30)

#define MESH_UNPROV_INTODLPS_TIME_OUT 	(5*1000)     // 5S
#define MESH_UNPROV_NORMAL_TIME_OUT   	(3*60*1000)  // 3min
#define CHANGE_SCAN_PARAM_TIME_OUT  	(5*1000)     // 5s


/* APP CONFIG DATA */
// max size, 32bit alignment  .
// ftl_save 鑰冭檻鍒癰le淇濆瓨鐨勫亸绉伙紙0xC00锛? app灞?~2000琚玬esh鍗犵敤銆傛晠鑷畾涔夋暟鎹粠2000寰�鍚庝娇鐢?
#define FTLMAP_APPCFGDATA_OFFSET    	2000
#define FTLMAP_APPCFGDATA_SIZE      	100  
/* quick onoff */
#define FTLMAP_QUICK_ONOFF_OFFSET   	2200
#define FTLMAP_QUICK_ONOFF_SIZE     	8  

typedef enum
{
    FLASH_PARAM_TYPE_APP_CFGDATA,
    FLASH_PARAM_TYPE_QUICK_ONOFF_CNT,
} FLASH_PARAM_TYPE_e;


#define FLASH_USERDATA_SAVE_LEN        	(64)    
#define MESH_DEV_UUID_LEN              	(16)




#define WRITE_DEFAULT_TIME              (5*1000)   	// 5s写flash

#define BATT_NORMAL                     0           // 电压正常标志
#define BATT_WARING                     1           // 低电压标志
#define BATT_WARIN_RANK                 2000        // 低电压值?
#define BATT_VOL1                       200         //电压降低值


#define BLEMESH_REPORT_FLAG_ALM         0x01
#define BLEMESH_REPORT_FLAG_STU         0x02
#define BLEMESH_REPORT_FLAG_BAT         0x04

typedef enum
{
    MAINLOOP_TIMEOUT,
    ENTER_DLPS_TIMEOUT,
    UNPROV_TIMEOUT,
    PROV_SUCCESS_TIMEOUT,
    test_light_TIMEOUT,
    SWITCH_TIMEOUT,
} SW_TIMER_MSG_TYPE;


typedef enum
{
    MESH_PROVISION_STATE_START,         //0
    MESH_PROVISION_STATE_FAILED,        //1
    MESH_PROVISION_STATE_SUCCEED,       //2
    MESH_PROVISION_STATE_UNPROV,
    MESH_PROVISION_STATE_PROVED,        //宸查厤缃?
} MESH_PROVISION_STATE_e;
extern MESH_PROVISION_STATE_e mesh_provison_state;

extern uint8_t attrdlps;


extern bool Hal_Get_Ble_MacAddr(uint8_t* mac);
extern void SkyBleMesh_Get_DeviceName(char *name, uint8_t *len);
extern void SkyBleMesh_Get_UUID(uint8_t *uuid, uint8_t len);
extern void SkyBleMesh_Regain_UUID(uint8_t *uuid, uint8_t len);

extern void SkyBleMesh_Provision_State(MESH_PROVISION_STATE_e sate);
extern bool SkyBleMesh_IsProvision_Sate(void);
extern void SkyBleMesh_Unprov_timer(void);	
extern void SkyBleMesh_Unprov_timer_delet(void);
extern void SkyBleMesh_ChangeScan_timer(uint8_t multi);
extern void SkyBleMesh_Handle_SwTmr_msg(T_IO_MSG *io_msg);
extern void SkyBleMesh_unBind_complete(void);
extern bool SkyBleMesh_Device_Active_Sate(void);


extern int SkyBleMesh_WriteConfig(void);
extern bool SkyBleMesh_Is_No_ReportMsg(void);
extern void SkyBleMesh_DlpsLight_Handle(bool isenter);
extern void SkyBleMesh_MainLoop_timer(void);
extern void SkyBleMesh_StopMainLoop_tmr(void);
extern void SkyBleMesh_StartMainLoop_tmr(void);
extern void SkyBleMesh_StopScanSwitch_tmr(void);
extern void SkyBleMesh_StartScanSwitch_tmr(void);
extern void SkyBleMesh_MainLoop(void);
extern uint8_t SkyBleMesh_App_Init(void);
extern void SkyBleMesh_Batterval_Lightsense(bool onlybatt);
extern uint8_t SkyBleMesh_Batt_Station(void);
extern void SkyBleMesh_alivetimer(void);
extern void SkySwitch_Handle(uint8_t key_mode, bool isprov);
extern void skyble_almctrl(bool allow);
extern void skyblemesh_battCheck(void);


extern void SkyBleMesh_Vendormodel_init(uint8_t elmt_idx);

extern void SkyMesh_ProductFactoryCheck_cb(T_GAP_ADV_EVT_TYPE adv_type, uint8_t* bd_addr, uint8_t* data, uint8_t data_len);


#define SKYMESH_FACTORYTEST_EABLE       0

#if SKYMESH_FACTORYTEST_EABLE==1
extern bool SkyMesh_Product_Factory_Checking(void);
#endif


#if MESH_TEST_PRESSURE == 1
extern void test_update_attr(void);
extern void SkyBleMesh_Test_Timeout_cb(void *timer);
extern uint8_t test_flag;
extern uint16_t testperid ;
extern uint32_t makepackcnt ;
extern uint32_t sendpackcnt ;
extern uint32_t revackpackcnt ;
extern uint32_t acktimoutcnt  ;
#endif



#endif // 

