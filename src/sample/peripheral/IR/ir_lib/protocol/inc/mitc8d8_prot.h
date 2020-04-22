/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      mitc8d8_prot.h
* @brief    This file provides driver of mitc8d8 protocol encoding.
* @details
* @author    elliot chen
* @date      2017-08-21
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __MITC8D8_PROT_H
#define __MITC8D8_PROT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_protocol_config.h"
#include "ir_encode.h"

/* Defines ------------------------------------------------------------------*/

/**
  * @brief  Configure MITC8D8 protocol repeat code transmission interval
  */

#define MITC8D8_TRANSMISION_INTERVAL        ((uint32_t)((30*10000 - 1)))

/* Exported functions --------------------------------------------------------*/
IR_Return_Type MITC8D8_Encode(IR_DataTypeDef *IR_DataStruct);

#ifdef __cplusplus
}
#endif

#endif /*__MITC8D8_PROT_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

