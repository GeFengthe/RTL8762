/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
* @file     datatrans_server_app.h
* @brief    Source file for data transmission server model.
* @details  Data types and external functions declaration.
* @author   hector_huang
* @date     2018-10-30
* @version  v1.0
* *************************************************************************************
*/
#ifndef _DATATRANS_SERVER_APP_H_
#define _DATATRANS_SERVER_APP_H_

#include "platform_types.h"
#include "datatrans_model.h"

BEGIN_DECLS

/**
 * @addtogroup DATATRANS_SERVER_APP
 * @{
 */
#if 1
#define ATTR_CLUSTER_ID_SWT (0x0001) // 1 byte
#define ATTR_CLUSTER_ID_SW1 (0x0002) // 1 BYTE 
#define ATTR_CLUSTER_ID_SW2 (0x0003) // 1 byte
#define ATTR_CLUSTER_ID_SW3 (0x0004) // 1 byre
#define ATTR_CLUSTER_ID_BRI (0x0005) // 4 byte
#define ATTR_CLUSTER_ID_CTP (0x0006) // 4 byte
#define ATTR_CLUSTER_ID_HUE (0x0007) // 4 byte
#define ATTR_CLUSTER_ID_SAT (0x0008) // 4 byte

// 这里的command是从网关角度来定义的

/*创维设备mesh UUID: 随机数 + MAC 地址*/
#define BLEMESH_VENDOR_COMPANY_ID  (0x0094)   /*  MTK */
#define BLEMESH_SKYWORTH_MODEL_ID  (0x5320)	  /*  SKYWORTH MODEL ID*/

#define OPCODE_HEARTBEAT_CMD_PING 	(0x33)
#define OPCODE_HEARTBEAT_CMD_PONG 	(0x34)
//子设备没有收到回复，默认网关断线，状态不上报。只发心跳消息请求上线
//1.SEND PING TO GATEWAY opcode(1B) + company id(2B) + 0x33(1B)
//2.GATEWAY REPONSE:     opcode(1B) + company id(2B) + 0x32(1B)
#define BLEMESH_SKYWORTH_OPCODE_HEARTBEAT (0xC7)

#define OPCODE_PROPERTY_CMD_GETATTR		(0x22)  /* 网关-->节点 */
#define OPCODE_PROPERTY_CMD_GETATTR_ACK	(0x20)  
#define OPCODE_PROPERTY_CMD_UPDATTR		(0x21)  /* 网关-->节点 */
#define OPCODE_PROPERTY_CMD_UPDATTR_ACK	(0x19)
#define OPCODE_PROPERTY_CMD_RPTATTR		(0x23)  /* 节点-->网关 */
#define OPCODE_PROPERTY_CMD_RPTATTR_ACK	(0x18)
// 1.get all attr:     opcode(1B) + company id(2B) + 0x22(1B) + seq(1B)
// 2.get all attr ack: opcode(1B) + company id(2B) + 0x20(1B) + seq(1B)
// 3.update attr:      opcode(1B) + company id(2B) + 0x21(1B) + seq(1B) + attrID(2B) + attr value(4B)
// 4.update attr ack:  opcode(1B) + company id(2B) + 0x19(1B) + seq(1B)
// 5.report attr:      opcode(1B) + company id(2B) + 0x23(1B) + seq(1B) + attrID(2B) + attr value(4B)
// 6.report attr ack:  opcode(1B) + company id(2B) + 0x18(1B) + attrID(2B) + attr value(4B)
#define BLEMESH_SKYWORTH_OPCODE_PROPERTY (0xC6)  /* */

#define OPCODE_DEVINFO_GET_PRODUCT_TYPE	 (0x1)
#define OPCODE_DEVINFO_RSP_PRODUCT_TYPE	 (0x2)
#define OPCODE_DEVINFO_GET_PRODUCT_BRAND (0x3)
#define OPCODE_DEVINFO_RSP_PRODUCT_BRAND (0x4)
#define OPCODE_DEVINFO_GET_PRODUCT_MODEL (0x5)
#define OPCODE_DEVINFO_RSP_PRODUCT_MODEL (0x6)
#define OPCODE_DEVINFO_GET_PRODUCT_MACADDR (0x7)
#define OPCODE_DEVINFO_RSP_PRODUCT_MACADDR (0x8)

// 1.get product type request:  opcode(1B) + company id(2B) + 0x1(1B)
// 2.get product type reponse:  opcode(1B) + company id(2B) + 0x2(1B) + type(4B)
// 1.get product brand request: opcode(1B) + company id(2B) + 0x3(1B)
// 2.get product brand reponse: opcode(1B) + company id(2B) + 0x4(1B) + brand(4B)
// 3.get product model:         opcode(1B) + company id(2B) + 0x5(1B)
// 4.get product model ack:     opcode(1B) + company id(2B) + 0x6(1B) + product model(16B)
// 7.get product mac addr:      opcode(1B) + company id(2B) + 0x7(1B)
// 8.get product mac addr ack:  opcode(1B) + company id(2B) + 0x8(1B) + mac addr(6B)
#define BLEMESH_SKYWORTH_OPCODE_DEVINFO	 (0xC5)



//#define MESH_OPENCODE_POS     0
//#define MESH_COMPANYID_POS    1
#define MESH_COMMAND_POS      0
#define MESH_SEQUENCE_POS     1
#define MESH_ATTR_ID_POS      2
#define MESH_ATTR_VAL_POS     4
#define MESH_PRODUCT_TYPE_POS      1
#define MESH_PRODUCT_BRAND_POS     1
#define MESH_PRODUCT_MODEL_POS     1

#endif


extern mesh_model_info_t datatrans_server;  

/**
 * @defgroup Datatrans_Server_Exported_Functions Datatrans Server Exported Functions
 * @brief
 * @{
 */
/**
 * @brief initialize data transmission server model
 */
void datatrans_server_model_init(void *rx_cb);
/** @} */
/** @} */

END_DECLS

#endif /* _DATATRANS_SERVER_APP_H_ */
