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
#define PRODUCT_TYPE    				(124)		// TODO 淇敼涓烘棤绾垮紑鍏?125
#define PRODUCT_MODEL   				("RBTSM01") 	// SYS1M01	
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


#define LED_NORMAL_VER                  2           // LED姝ｅ父鐗堟湰
#define LIGHT_DAYTIME                   (0x02)      // 鐧藉ぉ
//#define ENV_DETECT_TIME                 (1*60*1000) // 榛樿1min妫�娴嬩竴娆＄幆澧?
#define WRITE_DEFAULT_TIME              (5*1000)   	// 榛樿姣?s鎹姸鎬佸啓flash

//电压等级
#define BATT_TERY_0                     0               //2.9-3.0
#define BATT_TERY_1                     10              //2.8-2.9
#define BATT_TERY_2                     30              //2.6-2.8
#define BATT_TERY_3                     50              //2.4-2.6
#define BATT_TERY_4                     70              //2.2-2.4
#define BATT_TERY_5                     90


#define BATT_NORMAL                     0           // 正常电压
#define BATT_WARING                     1           // 设备低电量停止
#define BATT_WARING_RANK                2000        // 停止电压
#define BATT_THRESHOLD_VAL              100         // 上报电压到停止电压差值
#define BATT_TIMEOUT                    10*60*1000  // 鐢靛帇閲囬泦鏃堕棿鍛ㄦ湡

#define BLEMESH_REPORT_FLAG_TEM         0x01         //00000001     温度标志
#define BLEMESH_REPORT_FLAG_HUM         0x02         //00000010     湿度标志
#define BLEMESH_REPORT_FLAG_BAT         0x04        // 00000100     电量标志

typedef enum
{
    MAINLOOP_TIMEOUT,           //涓诲畾鏃跺櫒瓒呮椂鏍囧織
    ENTER_DLPS_TIMEOUT,         //浣庡姛鑰楀畾鏃跺櫒
    UNPROV_TIMEOUT,             //鏈叆缃戝畾鏃跺櫒
    PROV_SUCCESS_TIMEOUT,       //鍏ョ綉鎴愬姛瀹氭椂鍣?
    DISPLAY_LCD_TIMEOUT,         //浜伅瀹氭椂鍣?
} SW_TIMER_MSG_TYPE;

typedef enum
{
    MESH_PROVISION_STATE_START,         //0 寮�濮嬪叆缃?
    MESH_PROVISION_STATE_FAILED,        //1
    MESH_PROVISION_STATE_SUCCEED,       //2
    MESH_PROVISION_STATE_UNPROV,        //鏈叆缃?   
    MESH_PROVISION_STATE_PROVED,        //宸查厤缃?
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
extern void SkyBle_ReadSHTC3(void);
extern void SkyBleMesh_BattWaringTx(uint32_t ms);
extern void SkyBleMesh_DisPlayTempHum(void);
extern void SkyLcd_Process(void);
extern void SkyBleMesh_TempHumTx(uint32_t time_ms,int temcnt,uint32_t humcnt);
extern void SkyBleMesh_ReadOpentime(void);
extern void SkyBleMesh_alivetimer(void);
extern void SkyBleMesh_attrtimer(void);
extern void SkyBleMesh_attrdelettimer(void);
void SkyBleMesh_alivetimedelete(void);
void Main_WithoutNet_Handle(void);

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

