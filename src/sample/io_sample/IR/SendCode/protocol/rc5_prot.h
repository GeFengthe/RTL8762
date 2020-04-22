/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rc5_prot.h
* @brief    This file provides driver of RC5 protocol encoding.
* @details
* @author    elliot chen
* @date      2017-08-16
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __RC5_PROT_H
#define __RC5_PROT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_protocol_config.h"
#include "ir_encode.h"

/* Defines ------------------------------------------------------------------*/

/**
  * @brief  Configure RC5 protocol repeat code transmission interval
  */

#define RC5_TRANSMISION_INTERVAL    ((uint32_t)((1076*1000 - 1)))

/* Exported functions --------------------------------------------------------*/
IR_Return_Type RC5_Encode(IR_DataTypeDef *IR_DataStruct);

#ifdef __cplusplus
}
#endif

#endif /*__RC5_PROT_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

