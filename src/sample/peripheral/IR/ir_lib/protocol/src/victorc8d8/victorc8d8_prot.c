/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     VICTORC8D8_prot.c
* @brief    This file provides driver of VICTORC8D8 protocol encoding.
* @details
* @author  elliot_chen
* @date     2017-08-21
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "victorc8d8_prot.h"


/*!
* @ brief: VICTORC8D8 protocol structure.
* @ note: Store parameters of VICTORC8D8 protocol.
* @ Carrier frequency = 38kHz
* @ duty factor = 1/3
* @ LSB is sent first !
*/
const IR_ProtocolTypeDef VICTORC8D8_PROTOCOL =
{
    38,                                                     /* Carrier freqency */
    IR_CODING_LSB_FIRST,                                    /* LSB first */
    CARRIER_UNIT,                                           /* unit is carrier period */
    2,                                                      /* headerLen */
    {
        IR_PULSE_HIGH | (20 * 16), IR_PULSE_LOW | (20 * 8), /* headerBuf */
        0, 0,
        0, 0
    },
    {IR_PULSE_HIGH | (20), IR_PULSE_LOW | (20)},    /* log0Buf */
    {IR_PULSE_HIGH | (20), IR_PULSE_LOW | (20 * 3)}, /* log1Buf */
    IR_PULSE_HIGH | (20)                               /* stopBuf */
};

/**
  * @brief    Encode data of VICTORC8D8 protocol.
  * @param   IR_DataStruct: pointer to struct which store VICTORC8D8 code.
  * @retval None
  */
IR_Return_Type VICTORC8D8_Encode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&VICTORC8D8_PROTOCOL), true);
}


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

