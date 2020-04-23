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

#include "skylight.h"

#define MESH_TEST_PRESSURE 0

#if (SKY_LIGHT_TYPE == SKY_LIGHT_BELT_TYPE)

#define PRODUCT_TYPE    (104)	  //sunlight
#define PRODUCT_MODEL   ("IPA03") //ipa03	
#define PRODUCT_BRAND   (1)	      //skyworth
#define PRODUCT_VERSION ("1.0.05")

#elif (SKY_LIGHT_TYPE == SKY_LIGHT_BULB_TYPE)

#define PRODUCT_TYPE    (42)	  //sunlight
#define PRODUCT_MODEL   ("IPA03") //ipa03	
#define PRODUCT_BRAND   (1)	      //skyworth
#define PRODUCT_VERSION ("1.0.05")

#elif (SKY_LIGHT_TYPE == SKY_LIGHT_BULB_RGBWY_TYPE)

#define PRODUCT_TYPE    (107)	  //RGBWY灯泡
#define PRODUCT_MODEL   ("IPA03") //ipa03	
#define PRODUCT_BRAND   (1)	      //skyworth
#define PRODUCT_VERSION ("1.0.05")

#endif

#define MESH_ADV_NAME_LEN (30)

#define SKYBLEMESH_MAINLOOP_TIMER  (5)      // 50ms
// #define SKYBLEMESH_USR_TIMER       (2)      // 20ms

#define FLASH_USERDATA_SAVE_ADDR       (0x7D000)   // 0x7D000~7E000
#define FLASH_USERDATA_SAVE_LEN        (64)  // 大于 sizeof(SkyBleMeshIotManager) 




extern bool Hal_Get_Ble_MacAddr(uint8_t* mac);


extern void SkyBleMesh_Get_DeviceName(char *name, uint8_t *len);

extern void SkyBleMesh_Get_UUID(uint8_t *uuid, uint8_t len);
extern void SkyBleMesh_Regain_UUID(uint8_t *uuid, uint8_t len);
extern void SkyIotSaveAttr(void);


extern void SkyBleMesh_UnPro_Adv_timeout_cb(void);

extern void SkyBleMesh_Provision_Start(void);
extern void SkyBleMesh_Provision_Fail(void);
extern void SkyBleMesh_Provision_Success(void);

extern void SkyBleMesh_unBind_complete(void);
extern void SkyBleMesh_MainLoop_timer(void );
extern void SkyBleMesh_App_Init(void);


extern void SkyBleMesh_Vendormodel_init(uint8_t elmt_idx);
#if MESH_TEST_PRESSURE == 1
extern void test_update_attr(void);
extern uint16_t testperid ;
extern uint32_t makepackcnt ;
extern uint32_t sendpackcnt ;
extern uint32_t revackpackcnt ;
extern uint32_t acktimoutcnt  ;
#endif

#endif // 

