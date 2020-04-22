/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      nec_prot.c
* @brief    This file provides driver of NEC protocol encoding.
* @details
* @author  elliot_chen
* @date     2017-08-15
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "nec_prot.h"


/*!
* @ brief:NEC protocol structure.
* @ note: Store parameters of NEC protocol.
* @ Carrier frequency = 38kHz
* @ duty factor = 1/3
* @ first pulse : 9.5ms 4.5ms
* @ Address (8 bits) is sent first, then ~Address
* @ Command (8 bits) follows, then ~Command
* @ LSB is sent first !
*/
const IR_ProtocolTypeDef NEC_PROTOCOL =
{
    38,                                                 /* Carrier freqency */
    IR_CODING_LSB_FIRST,                                /* LSB first */
    TIME_UNIT,                                          /* unit is us */
    2,                                                  /* headerLen */
    {
        IR_PULSE_HIGH | (9000), IR_PULSE_LOW | (4500),
        0, 0,
        0, 0
    },                                             /* headerBuf */
    {IR_PULSE_HIGH | (560), IR_PULSE_LOW | (560)},      /* log0Buf */
    {IR_PULSE_HIGH | (560), IR_PULSE_LOW | (1690)},     /* log1Buf */
    IR_PULSE_HIGH | (560)                               /* stopBuf */
};

/*!
* @ User can modify this data structure to send user-defined repeat code!!!
* @ brief: IR repeat code protocol structure.
* @ note: Store parameters of repeat code protocol.
* @ parameters Configuration:
* @ PULSE_HIGH|9000 is 9ms of high voltage.
* @ PULSE_LOW|2250 is low 2.25ms of low voltage.
* @ ULSE_HIGH |560 is 560us of high voltage.
*/
const IR_RepeatCodeTypeDef NEC_REPEAT_CODE_PROTOCOL =
{
    TIME_UNIT,                                                  /* unit is us */
    3,                                                          /* length of repeat code */
    {IR_PULSE_HIGH | 9000, IR_PULSE_LOW | 2250, IR_PULSE_HIGH | 560} /* Buffer of repeat code: high 9ms, low 2.25ms, high 560us */
};

/**
  * @brief    Encode data of NEC protocol.
  * @param   IR_DataStruct: pointer to struct which store RCA code.
  * @retval None
  */
IR_Return_Type NEC_Encode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&NEC_PROTOCOL), true);
}

/**
  * @brief  Encode data of repeat code protocol.
  * @param  frequency: carrier frequency whose uinit is KHz.
  * @param  IR_DataStruct: pointer to struct which store repeat code data.
  * @retval None
  */
IR_Return_Type NEC_RepeatCodeEncode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_RepeatCodeEncode(IR_DataStruct, (IR_RepeatCodeTypeDef *)(&NEC_REPEAT_CODE_PROTOCOL));
}


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

