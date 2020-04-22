/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     dvb_40bit_prot.c
* @brief    This file provides driver of dvb protocol encoding.
* @details
* @author  elliot_chen
* @date     2019-083-19
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "dvb_40bit_prot.h"


#define DVB_40BIT_SEPAR_FIRST_PERIOD       (IR_PULSE_HIGH | (3690))
#define DVB_40BIT_SEPAR_SECOND_PERIOD      (IR_PULSE_LOW  | (1770))

/*!
* @ brief: DVB_40BIT_Encode protocol structure.
* @ note: Store parameters of DVB protocol.
* @ Carrier frequency = 38kHz
* @ duty factor = 1/3
* @ first pulse : 3.5ms 1.75ms
* @ Custom (32 bits) is sent first
* @ Data (32 bits) follows
* @ LSB is sent first !
*/
const IR_ProtocolTypeDef DVB_40BIT_PROTOCOL =
{
    38,                                                 /* Carrier freqency */
    IR_CODING_LSB_FIRST,                                /* LSB first */
    TIME_UNIT,                                          /* unit is us */
    2,                                                  /* headerLen */
    {
        IR_PULSE_HIGH | (3720), IR_PULSE_LOW | (1775), /* headerBuf */
        0, 0,
        0, 0
    },
    {IR_PULSE_HIGH | 605, IR_PULSE_LOW | 524},          /* log0Buf */
    {IR_PULSE_HIGH | 605, IR_PULSE_LOW | 1096},         /* log1Buf */
    IR_PULSE_HIGH | 605                                 /* stopBuf */
};

/*!
* @ User can modify this data structure to send user-defined repeat code!!!
* @ brief: IR repeat code protocol structure.
* @ note: Store parameters of repeat code protocol.
*/
const IR_RepeatCodeTypeDef DVB_40BIT_REPEAT_CODE_PROTOCOL =
{
    TIME_UNIT,                                                  /* unit is us */
    3,                                                          /* length of repeat code */
    {IR_PULSE_HIGH | 3710, IR_PULSE_LOW | 3580, IR_PULSE_HIGH | 630} /* Buffer of repeat code */
};

/**
  * @brief    Encode data of DVB protocol.
  * @param   IR_DataStruct: pointer to struct which store DVB code.
  * @retval None
  */
IR_Return_Type DVB_40BIT_Encode(IR_DataTypeDef *IR_DataStruct)
{
    IR_Return_Type ret = IR_SUCCEED;
    uint32_t key_code_idx = (IR_DataStruct->custom_bit_len << 1) + DVB_40BIT_PROTOCOL.headerLen;

    ret = IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&DVB_40BIT_PROTOCOL), true);

    memmove(&IR_DataStruct->irBuf[key_code_idx + 2], \
            &IR_DataStruct->irBuf[key_code_idx], \
            (IR_DataStruct->bufLen - key_code_idx) * 4);

    IR_DataStruct->irBuf[key_code_idx] = ConvertToCarrierCycle(DVB_40BIT_SEPAR_FIRST_PERIOD, \
                                                               DVB_40BIT_PROTOCOL.carrierFreq, DVB_40BIT_PROTOCOL.unit);
    IR_DataStruct->irBuf[key_code_idx + 1] = ConvertToCarrierCycle(DVB_40BIT_SEPAR_SECOND_PERIOD, \
                                                                   DVB_40BIT_PROTOCOL.carrierFreq, DVB_40BIT_PROTOCOL.unit);
    IR_DataStruct->bufLen += 2;
    return ret;
}

/**
  * @brief  Encode data of repeat code protocol.
  * @param  frequency: carrier frequency whose uinit is KHz.
  * @param  IR_DataStruct: pointer to struct which store repeat code data.
  * @retval None
  */
IR_Return_Type DVB_40BIT_RepeatCodeEncode(IR_DataTypeDef *IR_DataStruct)
{
    return IR_RepeatCodeEncode(IR_DataStruct,
                               (IR_RepeatCodeTypeDef *)(&DVB_40BIT_REPEAT_CODE_PROTOCOL));
}

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/

