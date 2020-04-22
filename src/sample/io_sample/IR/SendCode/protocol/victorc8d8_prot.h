/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      victorc8d8_prot.h
* @brief    This file provides driver of victorc8d8 protocol encoding.
* @details
* @author    elliot chen
* @date      2017-08-21
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __VICTORC8D8_PROT_H
#define __VICTORC8D8_PROT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_protocol_config.h"
#include "ir_encode.h"

/* Defines ------------------------------------------------------------------*/

/**
  * @brief  Configure VICTORC8D8 protocol repeat code transmission interval
  */

#define VICTORC8D8_TRANSMISION_INTERVAL     ((uint32_t)((589211 - 1)))//112T
#define VICTORC8D8_TRANSMISION_RE_INTERVAL  ((uint32_t)((462895 - 1)))//88T

/* Exported functions --------------------------------------------------------*/
IR_Return_Type VICTORC8D8_Encode(IR_DataTypeDef *IR_DataStruct);

#ifdef __cplusplus
}
#endif

#endif /*__VICTORC8D8_PROT_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

