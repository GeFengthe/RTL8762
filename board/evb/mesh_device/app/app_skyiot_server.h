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

#ifndef _APP_LIGHT_LIGHT_ALI_SERVER_H
#define _APP_LIGHT_LIGHT_ALI_SERVER_H

#include "skyswitch.h"

#define MESH_TEST_PRESSURE 0

#define PRODUCT_TYPE    (100)	  //switch
#define PRODUCT_MODEL   ("IPA03") //ipa03	
#define PRODUCT_BRAND   (1)	      //skyworth
#define PRODUCT_VERSION ("1.0.05")

#define MESH_ADV_NAME_LEN (30)

#define SKYBLEMESH_MAINLOOP_TIMER  (5)      // 50ms
#define SKYBLEMESH_USR_TIMER       (2)      // 20ms



/* APP CONFIG DATA */
// max size, 32bit alignment  .
// ftl_save 考虑到ble保存的偏移（0xC00）. app层0~2000被mesh占用。故自定义数据从2000往后使用
#define FTLMAP_APPCFGDATA_OFFSET   2000
#define FTLMAP_APPCFGDATA_SIZE     100  

/* quick onoff */
#define FTLMAP_QUICK_ONOFF_OFFSET   2200
#define FTLMAP_QUICK_ONOFF_SIZE     8  

typedef enum
{
    FLASH_PARAM_TYPE_APP_CFGDATA,
    FLASH_PARAM_TYPE_QUICK_ONOFF_CNT,
} FLASH_PARAM_TYPE_e;


#define FLASH_USERDATA_SAVE_LEN        (64)    // 大于 sizeof(SkyBleMeshIotManager) 

#define MESH_DEV_UUID_LEN  16




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


extern void SkyBleMesh_UnPro_Adv_timeout_cb(void);
extern void SkyBleMesh_Provision_State(MESH_PROVISION_STATE_e sate);
extern void SkyBleMesh_unBind_complete(void);

extern void SkyBleMesh_PowerOn_Save(void);
extern int SkyBleMesh_WriteConfig(void);
extern void SkyBleMesh_MainLoop_timer(void);
extern void SkyBleMesh_MainLoop(void);
extern uint8_t SkyBleMesh_App_Init(void);


extern void SkyBleMesh_Vendormodel_init(uint8_t elmt_idx);
#if MESH_TEST_PRESSURE == 1
extern void test_update_attr(void);
extern uint8_t test_flag;
extern uint16_t testperid ;
extern uint32_t makepackcnt ;
extern uint32_t sendpackcnt ;
extern uint32_t revackpackcnt ;
extern uint32_t acktimoutcnt  ;
#endif






typedef union
{
    uint32_t dword;
    struct
    {
        uint32_t io: 1; // switch
        uint32_t cmd: 1; // test cmd
        uint32_t rsvd: 30;
    } bit;
} DLPS_CTRL_STATU_T;

extern DLPS_CTRL_STATU_T DlpsCtrlStatu_t;


extern void test_dlps_func(uint8_t code);
extern bool switch_check_dlps_statu(void);
extern void switch_io_ctrl_dlps(bool allowenter);
extern void test_cmd_ctrl_dlps(bool allowenter);


extern void SkyBleMesh_ReadyEnterDlps_cfg(void);
extern void SkyBleMesh_EnterDlps_cfg(void);
extern void SkyBleMesh_ExitDlps_cfg(void);




#define UNPROV_TIME_OUT                     (2*60*1000)  // 10min
#define CHANGE_SCAN_PARAM_TIME_OUT          (10*1000)    // 10s

#include "app_msg.h"
typedef enum
{
    MAINLOOP_TIMEOUT,
    UNPROV_TIMEOUT,
    PROV_SUCCESS_TIMEOUT
} SW_TIMER_MSG_TYPE;


extern void SkyBleMesh_Unprov_timer(void);	
extern void SkyBleMesh_Unprov_timer_delet(void);
extern void SkyBleMesh_ChangeScan_timer(void);
extern void switch_handle_sw_timer_msg(T_IO_MSG *io_msg);


#endif // 

