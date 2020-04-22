/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      konica_prot.c
* @brief    This file provides driver of konica protocol encoding.
* @details
* @author  elliot_chen
* @date     2017-09-26
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "konica_prot.h"


/*!
* @ brief:konica protocol structure.
* @ note: Store parameters of konica protocol.
* @ Carrier frequency = 38kHz
* @ duty factor = 1/3
* @ first pulse : 3.6ms 1.8ms
* @ LSB is sent first !
*/
const IR_ProtocolTypeDef KONICA_PROTOCOL =
{
    38,                                                 /* Carrier freqency */
    IR_CODING_LSB_FIRST,                                /* LSB first */
    TIME_UNIT,                                          /* unit is us */
    2,                                                  /* headerLen */
    {
        IR_PULSE_HIGH | (3600), IR_PULSE_LOW | (1800),
        0, 0,
        0, 0
    },                                             /* headerBuf */
    {IR_PULSE_HIGH | (475), IR_PULSE_LOW | (475)},      /* log0Buf */
    {IR_PULSE_HIGH | (475), IR_PULSE_LOW | (1450)},     /* log1Buf */
    IR_PULSE_HIGH | (475)                               /* stopBuf */
};

/*!
* @ brief: IR repeat code protocol structure.
* @ note: Store parameters of repeat code protocol.
* @ parameters Configuration:
* @ PULSE_HIGH|9000 is 9ms of high voltage.
* @ PULSE_LOW|2250 is low 2.25ms of low voltage.
* @ ULSE_HIGH |560 is 560us of high voltage.
*/
const IR_RepeatCodeTypeDef KONICA_REPEAT_CODE_PROTOCOL =
{
    TIME_UNIT,                                                  /* unit is us */
    3,                                                          /* length of repeat code */
    {IR_PULSE_HIGH | 3600, IR_PULSE_LOW | 3600, IR_PULSE_HIGH | 475} /* Buffer of repeat code: high 9ms, low 2.25ms, high 560us */
};

/**
  * @brief    Encode data of NEC protocol.
  * @param   IR_DataStruct: pointer to struct which store RCA code.
  * @retval None
  */
IR_Return_Type KONICA_Encode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&KONICA_PROTOCOL), true);
}

/**
  * @brief  Encode data of repeat code protocol.
  * @param  frequency: carrier frequency whose uinit is KHz.
  * @param  IR_DataStruct: pointer to struct which store repeat code data.
  * @retval None
  */
IR_Return_Type KONICA_RepeatCodeEncode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_RepeatCodeEncode(IR_DataStruct, (IR_RepeatCodeTypeDef *)(&KONICA_REPEAT_CODE_PROTOCOL));
}


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

