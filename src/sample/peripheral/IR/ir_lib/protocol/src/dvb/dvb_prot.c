/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      dvb_prot.c
* @brief    This file provides driver of dvb protocol encoding.
* @details
* @author  elliot_chen
* @date     2017-08-21
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "dvb_prot.h"


/*!
* @ brief: DVB protocol structure.
* @ note: Store parameters of DVB protocol.
* @ Carrier frequency = 38kHz
* @ duty factor = 1/3
* @ first pulse : 3.5ms 1.75ms
* @ Custom (32 bits) is sent first
* @ Data (32 bits) follows
* @ LSB is sent first !
*/
const IR_ProtocolTypeDef DVB_PROTOCOL =
{
    38,                                                 /* Carrier freqency */
    IR_CODING_LSB_FIRST,                                /* LSB first */
    TIME_UNIT,                                          /* unit is us */
    2,                                                  /* headerLen */
    {
        IR_PULSE_HIGH | (3500), IR_PULSE_LOW | (1750), /* headerBuf */
        0, 0,
        0, 0
    },
    {IR_PULSE_HIGH | 421, IR_PULSE_LOW | 421},          /* log0Buf */
    {IR_PULSE_HIGH | 421, IR_PULSE_LOW | 1263},         /* log1Buf */
    IR_PULSE_HIGH | 421                                 /* stopBuf */
};

/**
  * @brief    Encode data of DVB protocol.
  * @param   IR_DataStruct: pointer to struct which store DVB code.
  * @retval None
  */
IR_Return_Type DVB_Encode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&DVB_PROTOCOL), true);
}


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

