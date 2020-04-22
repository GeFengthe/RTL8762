/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     lc7464m_prot.c
* @brief    This file provides driver of lc7464m protocol encoding.
* @details
* @author  elliot_chen
* @date     2017-08-18
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "lc7464m_prot.h"


/*!
* @ brief:lc7464m protocol structure.
* @ note: Store parameters of lc7464m protocol.
* @ Carrier frequency = 38kHz
* @ duty factor = 1/3
* @ first pulse :
* @ MSB is sent first !
*/
const IR_ProtocolTypeDef LC7464M_PROTOCOL =
{
    38,                                                 /* Carrier freqency */
    IR_CODING_LSB_FIRST,                                /* LSB first */
    CARRIER_UNIT,                                       /* unit is carrier period */
    2,                                                  /* headerLen */
    {
        IR_PULSE_HIGH | (16 * 8), IR_PULSE_LOW | (16 * 4), /* headerBuf */
        0, 0,
        0, 0
    },
    {IR_PULSE_HIGH | (16),   IR_PULSE_LOW | (16)}, /* log0Buf */
    {IR_PULSE_HIGH | (16),   IR_PULSE_LOW | (16 * 3)}, /* log1Buf */
    IR_PULSE_HIGH | (16)                            /* stopBuf */
};

/**
  * @brief    Encode data of LC7464M protocol.
  * @param  address: user code.
  * @param   command: data code.
  * @param   IR_DataStruct: pointer to struct which store LC7464M code.
  * @retval None
  */
IR_Return_Type LC7464M_Encode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&LC7464M_PROTOCOL), true);
}


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

