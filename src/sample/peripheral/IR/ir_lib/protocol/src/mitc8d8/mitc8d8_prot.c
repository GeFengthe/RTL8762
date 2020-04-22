/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     mitc8d8_prot.c
* @brief    This file provides driver of mitc8d8 protocol encoding.
* @details
* @author  elliot_chen
* @date     2017-08-21
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "mitc8d8_prot.h"


/*!
* @ brief: MITC8D8 protocol structure.
* @ note: Store parameters of MITC8D8 protocol.
* @ Carrier frequency = 38kHz
* @ duty factor = 1/3
* @ LSB is sent first !
*/
#if 0
const IR_ProtocolTypeDef MITC8D8_PROTOCOL =
{
    38,                                                 /* Carrier freqency */
    IR_CODING_LSB_FIRST,                                /* LSB first */
    CARRIER_UNIT,                                       /* unit is carrier period */
    0,                                                  /* headerLen */
    {
        0, 0,
        0, 0,
        0, 0
    },                                             /* headerBuf */
    {IR_PULSE_HIGH | (10 - 1), IR_PULSE_LOW | (10 - 1)}, /* log0Buf */
    {IR_PULSE_HIGH | (10 - 1), IR_PULSE_LOW | (30 - 1)}, /* log1Buf */
    IR_PULSE_HIGH | (10 - 1)                            /* stopBuf */
};
#else
const IR_ProtocolTypeDef MITC8D8_PROTOCOL =
{
    38,                                                 /* Carrier freqency */
    IR_CODING_LSB_FIRST,                                /* LSB first */
    TIME_UNIT,                                          /* unit is carrier period */
    0,                                                  /* headerLen */
    {
        0, 0,
        0, 0,
        0, 0
    },                                             /* headerBuf */
    {IR_PULSE_HIGH | (430), IR_PULSE_LOW | (750)}, /* log0Buf */
    {IR_PULSE_HIGH | (460), IR_PULSE_LOW | (1920)}, /* log1Buf */
    IR_PULSE_HIGH | (430)                           /* stopBuf */
};

#endif

/**
  * @brief    Encode data of MITC8D8 protocol.
  * @param   IR_DataStruct: pointer to struct which store MITC8D8 code.
  * @retval None
  */
IR_Return_Type MITC8D8_Encode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&MITC8D8_PROTOCOL), true);
}


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

