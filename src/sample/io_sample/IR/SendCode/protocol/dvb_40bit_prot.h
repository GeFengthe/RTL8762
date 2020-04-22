/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      dvb_40bit_prot.h
* @brief    This file provides driver of dvb protocol encoding.
* @details
* @author    elliot chen
* @date      2019-03-19
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __DVB_40BIT_PROT_H
#define __DVB_40BIT_PROT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ir_protocol_config.h"
#include "ir_encode.h"

/* Defines ------------------------------------------------------------------*/

/**
  * @brief  Configure DVB_4BIT protocol repeat code transmission interval
  */

#define DVB_40BIT_TRANSMISION_INTERVAL     ((uint32_t)((538*1000 - 1)))
#define DVB_40BIT_TRANSMISION_RE_INTERVAL  ((uint32_t)((101*10000 - 1)))

/* Exported functions --------------------------------------------------------*/
IR_Return_Type DVB_40BIT_Encode(IR_DataTypeDef *IR_DataStruct);
IR_Return_Type DVB_40BIT_RepeatCodeEncode(IR_DataTypeDef *IR_DataStruct);

#ifdef __cplusplus
}
#endif

#endif /*__DVB_40BIT_PROT_H*/

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/

