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

#define MESH_TEST_PRESSURE       		0

#if (SKY_SWITCH_TYPE == SKY_NIGHTLIGHT_TYPE)
#define PRODUCT_TYPE    				(118)		// TODO 修改为小夜灯:118
#define PRODUCT_MODEL   				("LCAM01") 	// LCAM01	
#define PRODUCT_BRAND   				(11)	    // swaiot
#define PRODUCT_VERSION 				("1.0.05")
#endif

// #define MESH_ADV_NAME_LEN (30)

#define MESH_UNPROV_INTODLPS_TIME_OUT 	(5*1000)     // 5S
#define MESH_UNPROV_NORMAL_TIME_OUT   	(3*60*1000)  // 3min
#define CHANGE_SCAN_PARAM_TIME_OUT  	(5*1000)     // 5s


/* APP CONFIG DATA */
// max size, 32bit alignment  .
// ftl_save 考虑到ble保存的偏移（0xC00）. app层0~2000被mesh占用。故自定义数据从2000往后使用
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

#define NIGHT_LIMIT_VOL					160		    // 20lux limit val
#define LIGHT_DEFAULT_WAY				(0x00)	    // 光暗时灯亮
#define LED_NORMAL_VER                  2           // LED正常版本
#define LED_FIRST_RELEASE				1           // 出厂标志
#define LIGHT_DAYTIME                   (0x02)      // 白天
#define ENV_DETECT_TIME                 (1*60*1000) // 默认1min检测一次环境
#define WRITE_DEFAULT_TIME              (5*1000)   	// 默认每5s据状态写flash
#define APP_UNREACT_MODE_N              0
#define APP_REACT_MODE_M                1
#define APP_REACT_MODE_S                2
#define APP_REACT_MODE_A                3


typedef enum
{
    MAINLOOP_TIMEOUT,
    ENTER_DLPS_TIMEOUT,
    UNPROV_TIMEOUT,
    PROV_SUCCESS_TIMEOUT
} SW_TIMER_MSG_TYPE;

typedef enum
{
    MESH_PROVISION_STATE_START,
    MESH_PROVISION_STATE_FAILED,
    MESH_PROVISION_STATE_SUCCEED,
    MESH_PROVISION_STATE_UNPROV,
    MESH_PROVISION_STATE_PROVED,
} MESH_PROVISION_STATE_e;
extern MESH_PROVISION_STATE_e mesh_provison_state;

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


extern void SkyBleMesh_Vendormodel_init(uint8_t elmt_idx);
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

