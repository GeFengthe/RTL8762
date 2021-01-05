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
#define PRODUCT_TYPE    				(125)		// TODO 修改为无线开关:125
#define PRODUCT_MODEL   				("SYS1M01") 	// SYS1M01	
#define PRODUCT_BRAND   				(11)	    // swaiot
#define PRODUCT_VERSION 				("1.0.01")      // 注意同步给接口 uart_test_read_app_version  和 version.h中的 VERSION_MAJOR
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


#define LED_NORMAL_VER                  2           // LED正常版本
#define LIGHT_DAYTIME                   (0x02)      // 白天
//#define ENV_DETECT_TIME                 (1*60*1000) // 默认1min检测一次环境
#define WRITE_DEFAULT_TIME              (5*1000)   	// 默认每5s据状态写flash

#define BATT_NORMAL                     0           // 电压正常
#define BATT_WARING                     1           // 警戒电压
#define BATT_WARING_RANK                2400        // 警戒电压等级1对应的电压值
#define BATT_THRESHOLD_VAL              100         // 电压判定误差范畴
#define BATT_TIMEOUT                    10*60*1000  // 电压采集时间周期

#define BLEMESH_REPORT_FLAG_ALM         0x01
#define BLEMESH_REPORT_FLAG_BAT         0x04

typedef enum
{
    MAINLOOP_TIMEOUT,           //主定时器超时标志
    ENTER_DLPS_TIMEOUT,         //低功耗定时器
    UNPROV_TIMEOUT,             //未入网定时器
    PROV_SUCCESS_TIMEOUT,       //入网成功定时器
    test_light_TIMEOUT,         //亮灯定时器
} SW_TIMER_MSG_TYPE;

typedef enum
{
    MESH_PROVISION_STATE_START,         //0 开始入网
    MESH_PROVISION_STATE_FAILED,        //1
    MESH_PROVISION_STATE_SUCCEED,       //2
    MESH_PROVISION_STATE_UNPROV,        //未入网    
    MESH_PROVISION_STATE_PROVED,        //已配网
} MESH_PROVISION_STATE_e;
typedef enum{
    ALM_KEY_INIT = 0x00,
    ALM_KEY_ONE,
    ALM_KEY_TWO,
    ALM_KEY_LONG,
}ALM_KEY_STATUS_e;

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

