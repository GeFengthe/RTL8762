/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     rc6_prot.h
* @brief    This file provides driver of RC6 protocol encoding.
* @details
* @author   yuan_feng
* @date     2018-08-22
* @version  v1.0
* *********************************************************************************************************
*/

#ifndef __RC6_PROT_H
#define __RC6_PROT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ir_protocol_config.h"
#include "ir_encode.h"

/* Defines ------------------------------------------------------------------*/
#define IR_PROTOTCOL_CARRIER_TYPE       0


/**
  * @brief  Configure RC6 protocol repeat code transmission interval
  */

#define RC6_32B_TRANSMISION_INTERVAL    ((uint32_t)((1067*1000 - 1)))

/* Exported functions --------------------------------------------------------*/
IR_Return_Type RC6_32B_Encode(IR_DataTypeDef *IR_DataStruct);

#ifdef __cplusplus
}
#endif

#endif /*__RC6_PROT_H*/

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

