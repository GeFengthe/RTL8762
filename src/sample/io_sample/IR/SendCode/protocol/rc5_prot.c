/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     rc5_prot.c
* @brief    This file provides driver of RC5 protocol encoding.
* @details
* @author   elliot_chen
* @date     2017-08-16
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rc5_prot.h"


/* Defines -------------------------------------------------------------------*/
#define IR_RC5_ENLARGED_BIT_INDEX               2
#define IR_RC5_TOGGLE_BIT_INDEX                 4

/* Globals -------------------------------------------------------------------*/
bool toggle_flag = false;

/*!
* @ brief:RC5 protocol structure.
* @ note: Store parameters of RC5 protocol.
* @ Carrier frequency = 38kHz
* @ duty factor = 1/3
* @ first pulse : logical 0, logical 0, logical 0
* @ System (5 bits) is sent first
* @ Data (6 bits) follows
* @ MSB is sent first !
*/
const IR_ProtocolTypeDef RC5_PROTOCOL =
{
    38,                                                 /* Carrier freqency */
    IR_CODING_MSB_FIRST,                                /* MSB first */
    TIME_UNIT,                                          /* unit is us */
    6,                                                  /* header length */
    {
        IR_PULSE_LOW | 847, IR_PULSE_HIGH | 827,           /* Start bit: logical 1 */
        IR_PULSE_LOW | 847, IR_PULSE_HIGH | 827,           /* Enlarged bit */
        IR_PULSE_LOW | 847, IR_PULSE_HIGH | 827
    },             /* Toggle bit */
    {IR_PULSE_HIGH | 839, IR_PULSE_LOW | 835},          /* log0Buf */
    {IR_PULSE_LOW | 847, IR_PULSE_HIGH | 827},          /* log1Buf */
    IR_PULSE_HIGH | 839                                /* stopBuf */
};

/**
  * @brief    Encode data to packet.
  * @param  IR_DataStruct: pointer to struct which store IR data code.
  * @param  IR_Protocol: pointer to specify IR protocol structure.
  * @param  flag: logical level type.
  * @param  index: encoder buffer index.
  * @retval None
  */
IR_Return_Type RC5_EncodeBit(IR_DataTypeDef *IR_DataStruct, IR_ProtocolTypeDef *IR_Protocol,
                             uint8_t index, bool flag)
{
    uint16_t i = 0;
    IR_DataType Log1[IR_MAX_LOG_WAVFORM_SIZE];
    IR_DataType Log0[IR_MAX_LOG_WAVFORM_SIZE];

    for (i = 0; i < IR_MAX_LOG_WAVFORM_SIZE; i++)
    {
        Log1[i] = ConvertToCarrierCycle(IR_Protocol->log1Buf[i], IR_DataStruct->carrierFreq,
                                        IR_Protocol->unit);
        Log0[i] = ConvertToCarrierCycle(IR_Protocol->log0Buf[i], IR_DataStruct->carrierFreq,
                                        IR_Protocol->unit);
    }

    /* Encoding Enlarged bit */
    if (flag == false)
    {
        /* Logical 0 data */
        IR_DataStruct->irBuf[index] = Log0[0];
        IR_DataStruct->irBuf[index + 1] = Log0[1];

    }
    else
    {
        /* Logical 1 data */
        IR_DataStruct->irBuf[index] = Log1[0];
        IR_DataStruct->irBuf[index + 1] = Log1[1];
    }

    return IR_SUCCEED;
}

/**
  * @brief    Encode data of RC5 protocol.
  * @param  address: user code.
  * @param   command: data code.
  * @param   IR_DataStruct: pointer to struct which store RCA code.
  * @retval None
  */
IR_Return_Type RC5_Encode(IR_DataTypeDef *IR_DataStruct)
{
    IR_Return_Type return_type;

    return_type = IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&RC5_PROTOCOL), true);

    if (return_type != IR_SUCCEED)
    {
        return return_type;
    }

    /* Encoding enlarged bit */
    if (IR_DataStruct->code[(IR_DataStruct->custom_bit_len + 7) / 8] > 0x3F)
    {
        return_type = RC5_EncodeBit(IR_DataStruct, (IR_ProtocolTypeDef *)(&RC5_PROTOCOL),
                                    IR_RC5_ENLARGED_BIT_INDEX, false);
    }
    else
    {
        return_type = RC5_EncodeBit(IR_DataStruct, (IR_ProtocolTypeDef *)(&RC5_PROTOCOL),
                                    IR_RC5_ENLARGED_BIT_INDEX, true);
    }

    /* Encoding toggle bit */
    if (toggle_flag)
    {
        return_type = RC5_EncodeBit(IR_DataStruct, (IR_ProtocolTypeDef *)(&RC5_PROTOCOL),
                                    IR_RC5_TOGGLE_BIT_INDEX, true);
        toggle_flag = false;
    }
    else
    {
        return_type = RC5_EncodeBit(IR_DataStruct, (IR_ProtocolTypeDef *)(&RC5_PROTOCOL),
                                    IR_RC5_TOGGLE_BIT_INDEX, false);
        toggle_flag = true;
    }

    return return_type;
}


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

