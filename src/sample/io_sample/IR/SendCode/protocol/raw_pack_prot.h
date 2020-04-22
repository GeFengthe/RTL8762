/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      raw_pack_prot.h
* @brief    This file provides driver of raw packet protocol encoding.
* @details
* @author    elliot chen
* @date      2018-07-23
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __RAW_PACK_PROT_H
#define __RAW_PACK_PROT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_protocol_config.h"
#include "ir_encode.h"

/* Defines ------------------------------------------------------------------*/
#define RAW_PACK_TRANSMISION_INTERVAL    ((uint32_t)((108*10000 - 1)))

/* Exported functions --------------------------------------------------------*/
IR_Return_Type Raw_Pack_Encode(IR_DataTypeDef *IR_DataStruct);

#ifdef __cplusplus
}
#endif

#endif /*__RAW_PACK_PROT_H*/

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

