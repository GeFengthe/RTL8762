/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      pioneer_prot.c
* @brief    This file provides driver of pioneer protocol encoding.
* @details
* @author  elliot_chen
* @date     2017-08-21
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "pioneer_prot.h"


/*!
* @ brief: PIONEER protocol structure.
* @ note: Store parameters of PIONEER protocol.
* @ Carrier frequency = 38kHz
* @ duty factor = 1/3
* @ first pulse : 8.5ms 4.25ms
* @ Custom (8 bits) is sent first, then ~Custom
* @ Data (8 bits) follows, then ~Data
* @ LSB is sent first !
*/
const IR_ProtocolTypeDef PIONEER_PROTOCOL =
{
    40,                                                 /* Carrier freqency */
    IR_CODING_LSB_FIRST,                                /* LSB first */
    TIME_UNIT,                                          /* unit is us */
    2,                                                  /* headerLen */
    {
        IR_PULSE_HIGH | 8000, IR_PULSE_LOW | 4250,
        0, 0,
        0, 0
    },                                             /* headerBuf */
    {IR_PULSE_HIGH | 536, IR_PULSE_LOW | 536},          /* log0Buf */
    {IR_PULSE_HIGH | 536, IR_PULSE_LOW | 1590},         /* log1Buf */
    IR_PULSE_HIGH | 536                                /* stopBuf */
};

/**
  * @brief    Encode data of PIONEER protocol.
  * @param   IR_DataStruct: pointer to struct which store PIONEER code.
  * @retval None
  */
IR_Return_Type PIONEER_Encode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&PIONEER_PROTOCOL), true);
}


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

