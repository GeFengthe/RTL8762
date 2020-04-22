/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     sharp_prot.c
* @brief    This file provides driver of sharp protocol encoding.
* @details
* @author  elliot_chen
* @date     2017-08-21
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "sharp_prot.h"


/*!
* @ brief: SHARP protocol structure.
* @ note: Store parameters of SHARP protocol.
* @ Carrier frequency = 38kHz
* @ duty factor = 1/3
* @ LSB is sent first !
*/
const IR_ProtocolTypeDef SHARP_PROTOCOL =
{
    38,                                                 /* Carrier freqency */
    IR_CODING_LSB_FIRST,                                /* LSB first */
    TIME_UNIT,                                          /* unit is us */
    0,                                                  /* headerLen */
    {0, 0, 0, 0, 0, 0},                                 /* headerBuf */
    {IR_PULSE_HIGH | (264), IR_PULSE_LOW | (786)},      /* log0Buf */
    {IR_PULSE_HIGH | (264), IR_PULSE_LOW | (1846)},     /* log1Buf */
    IR_PULSE_HIGH | (264)                              /* stopBuf */
};

/**
  * @brief    Encode data of SHARP protocol.
  * @param   IR_DataStruct: pointer to struct which store SHARP code.
  * @retval None
  */
IR_Return_Type SHARP_Encode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&SHARP_PROTOCOL), true);
}


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

