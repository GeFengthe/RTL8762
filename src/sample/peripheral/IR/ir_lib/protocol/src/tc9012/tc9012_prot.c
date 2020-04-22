/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      tc9012_prot.c
* @brief    This file provides driver of TC9012 protocol encoding.
* @details
* @author  elliot_chen
* @date     2017-08-21
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "tc9012_prot.h"


/*!
* @ brief: TC9012 protocol structure.
* @ note: Store parameters of TC9012 protocol.
* @ Carrier frequency = 38kHz
* @ duty factor = 1/3
* @ first pulse : 4.5ms 4.5ms
* @ Custom (8 bits) is sent first, then ~Custom
* @ Data (8 bits) follows, then ~Data
* @ LSB is sent first !
*/
const IR_ProtocolTypeDef TC9012_PROTOCOL =
{
    38,                                                 /* Carrier freqency */
    IR_CODING_LSB_FIRST,                                /* LSB first */
    TIME_UNIT,                                          /* unit is us */
    2,                                                  /* headerLen */
    {
        IR_PULSE_HIGH | 4500, IR_PULSE_LOW | 4500,
        0, 0,
        0, 0
    },                                             /* headerBuf */
    {IR_PULSE_HIGH | 560, IR_PULSE_LOW | 565},          /* log0Buf */
    {IR_PULSE_HIGH | 560, IR_PULSE_LOW | 1690},         /* log1Buf */
    IR_PULSE_HIGH | 560                                /* stopBuf */
};

/*!
* @ User can modify this data structure to send user-defined repeat code!!!
* @ brief: IR repeat code protocol structure.
* @ note: Store parameters of repeat code protocol.
*/
const IR_RepeatCodeTypeDef TC9012_REPEAT_CODE0_PROTOCOL =
{
    TIME_UNIT,                                          /* unit is us */
    5,                                                      /* length of repeat code */
    {
        IR_PULSE_HIGH | 4500, IR_PULSE_LOW | 4500, \
        IR_PULSE_HIGH | 560, IR_PULSE_LOW | 1690, \
        IR_PULSE_HIGH | 560
    }
};

/*!
* @ User can modify this data structure to send user-defined repeat code!!!
* @ brief: IR repeat code protocol structure.
* @ note: Store parameters of repeat code protocol.
*/
const IR_RepeatCodeTypeDef TC9012_REPEAT_CODE1_PROTOCOL =
{
    TIME_UNIT,                                          /* unit is us */
    5,                                                  /* length of repeat code */
    {
        IR_PULSE_HIGH | 4500, IR_PULSE_LOW | 4500, \
        IR_PULSE_HIGH | 560, IR_PULSE_LOW | 565, \
        IR_PULSE_HIGH | 560
    }
};

/**
  * @brief    Encode data of NEC protocol.
  * @param  address: user code.
  * @param   command: data code.
  * @param   IR_DataStruct: pointer to struct which store RCA code.
  * @retval None
  */
IR_Return_Type TC9012_Encode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&TC9012_PROTOCOL), true);
}

/**
  * @brief  Encode data of repeat code protocol.
  * @param  frequency: carrier frequency whose uinit is KHz.
  * @param  IR_DataStruct: pointer to struct which store repeat code data.
  * @retval None
  */
IR_Return_Type TC9012_RepeatCodeEncode(IR_DataTypeDef *IR_DataStruct)
{
    if ((IR_DataStruct->code[0]) & 0x01)
    {
        return IR_RepeatCodeEncode(IR_DataStruct, (IR_RepeatCodeTypeDef *)(&TC9012_REPEAT_CODE1_PROTOCOL));
    }
    else
    {
        return IR_RepeatCodeEncode(IR_DataStruct, (IR_RepeatCodeTypeDef *)(&TC9012_REPEAT_CODE0_PROTOCOL));
    }
}


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

