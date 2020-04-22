/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      lc7464m_prot.h
* @brief    This file provides driver of lc7464m protocol encoding.
* @details
* @author    elliot chen
* @date      2017-08-18
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __LC7464M_PROT_H
#define __LC7464M_PROT_H

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

#define LC7464M_TRANSMISION_INTERVAL        ((uint32_t)((421053 - 1)))//100T

/* Exported functions --------------------------------------------------------*/
IR_Return_Type LC7464M_Encode(IR_DataTypeDef *IR_DataStruct);

#ifdef __cplusplus
}
#endif

#endif /*__SONY6124_PROT_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

