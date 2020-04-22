/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     rc6_prot.c
* @brief    This file provides driver of RC6 protocol encoding.
* @details
* @author   yuan_feng
* @date     2018-08-22
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rc6_prot.h"


/* Globals -------------------------------------------------------------------*/
bool RC6_Toggle_Flag = false;

/*!
* @ brief:RC6 protocol structure.
* @ note: Store parameters of RC6 protocol.
* @ Carrier frequency = 36kHz
* @ duty factor = 1/3
* @ first pulse : logical 0, logical 0, logical 0
* @ Leader bit (1 bit:6t+2t) is sent first
* @ Start bit(1 bit:logical 1) follows
* @ Mode bits(3 bits:110b) follows
* @ Trailer bit (1 bit;2t+2t) follows
* @ Customer cdoe (16 bits:0x8010) follows
* @ Command cdoe (8 bits:0x26 or 0xA6 bit7:toggle bit) follows
* @ Data cdoe (8 bits) follows
* @ MSB is sent first !
*/
#if (IR_PROTOTCOL_CARRIER_TYPE == 0)

static IR_DataType RC6_32B_Protocol_Header_Add[5] = {IR_PULSE_HIGH | 96, IR_PULSE_LOW | 32,
                                                     IR_PULSE_HIGH | 16, IR_PULSE_LOW | 16,
                                                     IR_PULSE_HIGH | 16
                                                    };
const IR_ProtocolTypeDef RC6_32B_PROTOCOL =
{
    36,                                                 /* Carrier freqency */
    IR_CODING_MSB_FIRST,                                /* MSB first */
    CARRIER_UNIT,                                       /* unit is us */
    6,                                                  /* header length */
    {
        IR_PULSE_LOW | 16, IR_PULSE_HIGH | 16,          /* Start bit: logical 1 */
        IR_PULSE_LOW | 32, IR_PULSE_HIGH | 16,          /* Enlarged bit */
        IR_PULSE_LOW | 32, IR_PULSE_HIGH | 32
    },             /* Toggle bit */
    {IR_PULSE_LOW | 16, IR_PULSE_HIGH | 16},            /* log0Buf */
    {IR_PULSE_HIGH | 16, IR_PULSE_LOW | 16},            /* log1Buf */
    IR_PULSE_LOW | 0                                    /* stopBuf */
};

#elif (IR_PROTOTCOL_CARRIER_TYPE == 1)

static IR_DataType RC6_32B_Protocol_Header_Add[5] = {IR_PULSE_HIGH | 2667, IR_PULSE_LOW | 889,
                                                     IR_PULSE_HIGH | 445, IR_PULSE_LOW | 445,
                                                     IR_PULSE_HIGH | 445
                                                    };
const IR_ProtocolTypeDef RC6_32B_PROTOCOL =
{
    36,                                                 /* Carrier freqency */
    IR_CODING_MSB_FIRST,                                /* MSB first */
    TIME_UNIT,                                         /* unit is us */
    6,                                                  /* header length */
    {
        IR_PULSE_LOW | 445, IR_PULSE_HIGH | 445,          /* Start bit: logical 1 */
        IR_PULSE_LOW | 889, IR_PULSE_HIGH | 445,          /* Enlarged bit */
        IR_PULSE_LOW | 889, IR_PULSE_HIGH | 889
    },             /* Toggle bit */
    {IR_PULSE_LOW | 445, IR_PULSE_HIGH | 445},            /* log0Buf */
    {IR_PULSE_HIGH | 445, IR_PULSE_LOW | 445},            /* log1Buf */
    IR_PULSE_LOW | 0                                    /* stopBuf */
};

#endif

/**
  * @brief  Encode data of RC6 protocol.
  * @param  address: user code.
  * @param  command: data code.
  * @param  IR_DataStruct: pointer to struct which store RCA code.
  * @retval None
  */
IR_Return_Type RC6_32B_Encode(IR_DataTypeDef *IR_DataStruct)
{
    IR_Return_Type return_type;

    /* Encoding toggle bit */
    if (RC6_Toggle_Flag)
    {
        IR_DataStruct->code[2] |= 1 << 7;
        RC6_Toggle_Flag = false;
    }
    else
    {
        IR_DataStruct->code[2] &= 0x7F;
        RC6_Toggle_Flag = true;
    }

    return_type = IR_Encode(IR_DataStruct, (IR_ProtocolTypeDef *)(&RC6_32B_PROTOCOL), true);

    if (return_type != IR_SUCCEED)
    {
        return return_type;
    }

    memmove(&IR_DataStruct->irBuf[5], IR_DataStruct->irBuf, IR_DataStruct->bufLen * 4);

    for (uint8_t i = 0; i < 5; i++)
    {
        IR_DataStruct->irBuf[i] = ConvertToCarrierCycle(RC6_32B_Protocol_Header_Add[i],
                                                        RC6_32B_PROTOCOL.carrierFreq, RC6_32B_PROTOCOL.unit);
    }
    IR_DataStruct->bufLen += 5;

    return return_type;
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

