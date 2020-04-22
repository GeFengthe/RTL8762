/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      nec_prot.h
* @brief    This file provides driver of NEC protocol encoding.
* @details
* @author    elliot chen
* @date      2017-08-15
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __NEC_PROT_H
#define __NEC_PROT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_protocol_config.h"
#include "ir_encode.h"

/* Defines ------------------------------------------------------------------*/

/**
  * @brief  Configure NEC protocol repeat code transmission interval
  */

#define NEC_TRANSMISION_INTERVAL    ((uint32_t)((108*10000 - 1)))

/* Exported functions --------------------------------------------------------*/
IR_Return_Type NEC_Encode(IR_DataTypeDef *IR_DataStruct);
IR_Return_Type NEC_RepeatCodeEncode(IR_DataTypeDef *IR_DataStruct);

#ifdef __cplusplus
}
#endif

#endif /*__NEC_PROT_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

