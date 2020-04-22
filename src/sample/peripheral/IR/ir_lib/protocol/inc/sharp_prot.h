/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      sharp_prot.h
* @brief    This file provides driver of sharp protocol encoding.
* @details
* @author    elliot chen
* @date      2017-08-21
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __SHARP_PROT_H
#define __SHARP_PROT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_protocol_config.h"
#include "ir_encode.h"

/* Defines ------------------------------------------------------------------*/

/**
  * @brief  Configure SHARP protocol repeat code transmission interval
  */

#define SHARP_TRANSMISION_INTERVAL      ((uint32_t)((668*1000 - 1)))

/* Exported functions --------------------------------------------------------*/
IR_Return_Type SHARP_Encode(IR_DataTypeDef *IR_DataStruct);

#ifdef __cplusplus
}
#endif

#endif /*__SHARP_PROT_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

