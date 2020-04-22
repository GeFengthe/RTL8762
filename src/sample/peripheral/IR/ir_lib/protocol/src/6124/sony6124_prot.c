/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      sony6124_prot.c
* @brief    This file provides driver of sony 6124 protocol encoding.
* @details
* @author  elliot_chen
* @date     2017-08-17
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "sony6124_prot.h"


/*!
* @ brief:sony 6124 protocol structure.
* @ note: Store parameters of sony 6124 protocol.
* @ Carrier frequency = 40kHz
* @ duty factor = 1/3
* @ LSB is sent first !
*/
const IR_ProtocolTypeDef SONY6124_PROTOCOL =
{
    40,                                                 /* Carrier freqency */
    IR_CODING_LSB_FIRST,                                /* LSB first */
    TIME_UNIT,                                          /* unit is us */
    2,                                                  /* headerLen */
    {
        IR_PULSE_HIGH | 2400, IR_PULSE_LOW | 600,
        0, 0,
        0, 0
    },                                             /* headerBuf */
    {IR_PULSE_HIGH | 600, IR_PULSE_LOW | 600},          /* log0Buf */
    {IR_PULSE_HIGH | 1200, IR_PULSE_LOW | 600},         /* log1Buf */
    0,                                                 /* stopBuf */
};

/**
  * @brief    Encode data of SONY 6124 protocol.
  * @param  address: user code.
  * @param   command: data code.
  * @param   IR_DataStruct: pointer to struct which store RCA code.
  * @retval None
  */
IR_Return_Type SONY6124_Encode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&SONY6124_PROTOCOL), false);
}


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

