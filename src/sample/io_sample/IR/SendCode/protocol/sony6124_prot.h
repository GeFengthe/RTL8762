/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      sony6124_prot.h
* @brief    This file provides driver of sony 6124 protocol encoding.
* @details
* @author    elliot chen
* @date      2017-08-17
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __SONY6124_PROT_H
#define __SONY6124_PROT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_protocol_config.h"
#include "ir_encode.h"

/* Defines ------------------------------------------------------------------*/

/**
  * @brief  Configure SONY 6124 protocol repeat code transmission interval
  */

#define SONY6124_TRANSMISION_INTERVAL   ((uint32_t)((45*10000 - 1)))

/* Exported functions --------------------------------------------------------*/
IR_Return_Type SONY6124_Encode(IR_DataTypeDef *IR_DataStruct);

#ifdef __cplusplus
}
#endif

#endif /*__SONY6124_PROT_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

