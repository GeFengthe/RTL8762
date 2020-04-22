/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      ir_encode.h
* @brief    This file provides driver of IR protocol encoding.
* @details
* @author    elliot chen
* @date      2017-09-19
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __IR_ENCODE_H
#define __IR_ENCODE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_protocol_config.h"

/* Defines ------------------------------------------------------------------*/

/**
  * @brief  Configure maxium length of header of IR protocol
  */

#define IR_MAX_HEADDER_LEN              6

/**
  * @brief  Configure maxium data length of logical 0 and logical 1
  */

#define IR_MAX_LOG_WAVFORM_SIZE         2

/**
  * @brief  IR coding order type definition.
  */

typedef enum
{
    IR_CODING_LSB_FIRST,
    IR_CODING_MSB_FIRST
} IR_CODING_ORDER;

/**
  * @brief  IR coding order type definition.
  */

typedef enum
{
    CARRIER_UNIT,
    TIME_UNIT
} IR_DATA_TYPE;

/**
  * @brief  IR protocol structure definition
  */
typedef struct
{
    uint16_t        carrierFreq;
    IR_CODING_ORDER coding_order;
    IR_DATA_TYPE    unit;
    uint8_t         headerLen;
    IR_DataType     headerBuf[IR_MAX_HEADDER_LEN];
    IR_DataType     log0Buf[IR_MAX_LOG_WAVFORM_SIZE];
    IR_DataType     log1Buf[IR_MAX_LOG_WAVFORM_SIZE];
    IR_DataType     stopBuf;
} IR_ProtocolTypeDef;

/**
  * @brief  IR protocol structure definition  which store IR repeat code.
  */

typedef struct
{
    IR_DATA_TYPE    unit;
    uint8_t     RepeatCodeLen;
    IR_DataType RepeatCodeBuf[IR_REPEAT_CODE_MAX_LEN];
} IR_RepeatCodeTypeDef;

/* Exported functions --------------------------------------------------------*/
IR_Return_Type IR_Encode(IR_DataTypeDef *IR_DataStruct, IR_ProtocolTypeDef *IR_Protocol,
                         bool in_order);
IR_Return_Type IR_RepeatCodeEncode(IR_DataTypeDef *IR_DataStruct,
                                   IR_RepeatCodeTypeDef *IR_Protocol);
IR_DataType ConvertToCarrierCycle(uint32_t time, uint32_t freq, IR_DATA_TYPE data_type);

#ifdef __cplusplus
}
#endif

#endif /*__IR_ENCODE_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

