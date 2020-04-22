/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      konica_prot.h
* @brief    This file provides driver of konica protocol encoding.
* @details
* @author    elliot chen
* @date      2017-09-26
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __KONICA_PROT_H
#define __KONICA_PROT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_protocol_config.h"
#include "ir_encode.h"

/* Defines ------------------------------------------------------------------*/

/**
  * @brief  Configure konica protocol repeat code transmission interval
  */

#define KONICA_TRANSMISION_INTERVAL ((uint32_t)((52*10000 - 1)))

/* Exported functions --------------------------------------------------------*/
IR_Return_Type KONICA_Encode(IR_DataTypeDef *IR_DataStruct);
IR_Return_Type KONICA_RepeatCodeEncode(IR_DataTypeDef *IR_DataStruct);

#ifdef __cplusplus
}
#endif

#endif /*__KONICA_PROT_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

