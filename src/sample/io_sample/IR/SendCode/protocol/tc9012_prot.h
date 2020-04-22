/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      tc9012_prot.h
* @brief    This file provides driver of tc9012 protocol encoding.
* @details
* @author    elliot chen
* @date      2017-08-21
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __TC9012_PROT_H
#define __TC9012_PROT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_protocol_config.h"
#include "ir_encode.h"

/* Defines ------------------------------------------------------------------*/

/**
  * @brief  Configure TC9012 protocol repeat code transmission interval
  */

#define TC9012_TRANSMISION_INTERVAL ((uint32_t)((108*10000 - 1)))

/* Exported functions --------------------------------------------------------*/
IR_Return_Type TC9012_Encode(IR_DataTypeDef *IR_DataStruct);
IR_Return_Type TC9012_RepeatCodeEncode(IR_DataTypeDef *IR_DataStruct);

#ifdef __cplusplus
}
#endif

#endif /*__TC9012_PROT_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

