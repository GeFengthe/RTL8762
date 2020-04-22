/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      ir_encode.c
* @brief    This file provides driver of IR protocol encoding.
* @details
* @author  elliot_chen
* @date     2017-09-19
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "ir_encode.h"


/* Internal defines -------------------------------------------------------------------*/

#define IR_ENCODE_BIT_LEN_TO_BYTE_LEN   7
#define IR_ENCODE_BYTE_LEN              8
#define IR_ENCODE_LOGICAL_1             (0x01)

/**
  * @brief    unit convert.
  * @param  time: time of waveform.
  * @param  carrier_cycle: cycle of carrier.
  * @retval   vaule of data whose unit is cycle of carrier.
  */
IR_DataType ConvertToCarrierCycle(uint32_t time, uint32_t freq, IR_DATA_TYPE data_type)
{
    if (data_type == TIME_UNIT)
    {
#if HARDWARE_AVOIDANCE
        return ((time & IR_PULSE_HIGH) | ((time & IR_DATA_MSK) * freq / 1000 - 1));
#else
        return ((time & IR_PULSE_HIGH) | ((time & IR_DATA_MSK) * freq / 1000));
#endif
    }
    else
    {
#if HARDWARE_AVOIDANCE
        return ((time & IR_PULSE_HIGH) | ((time & IR_DATA_MSK) - 1));
#else
        return time;
#endif
    }
}

/**
  * @brief   check specify bit value of data.
  * @param a: data which want to check.
  * @param  bit_pos: position of bit which want to check
  * @retval
  */
static uint8_t CheckBitStatus(uint32_t a, uint32_t bit_pos)
{
    return ((a >> bit_pos) & 0x1);
}

/**
  * @brief    Encode data to packet.
  * @param  IR_DataStruct: pointer to struct which store IR data code.
  * @param  IR_Protocol: pointer to specify IR protocol structure.
  * @retval None
  */
IR_Return_Type IR_EncodeCode(IR_DataTypeDef *IR_DataStruct, IR_ProtocolTypeDef *IR_Protocol,
                             uint8_t index, uint8_t bitLen, uint8_t *pBufLen)
{
    uint8_t codeWidth = 0;
    uint16_t i = 0;
    uint16_t bitPos = 0;
    IR_DataType Log1[IR_MAX_LOG_WAVFORM_SIZE];
    IR_DataType Log0[IR_MAX_LOG_WAVFORM_SIZE];

    for (i = 0; i < IR_MAX_LOG_WAVFORM_SIZE; i++)
    {
        Log1[i] = ConvertToCarrierCycle(IR_Protocol->log1Buf[i], IR_DataStruct->carrierFreq,
                                        IR_Protocol->unit);
        Log0[i] = ConvertToCarrierCycle(IR_Protocol->log0Buf[i], IR_DataStruct->carrierFreq,
                                        IR_Protocol->unit);
    }

    for (; bitLen > 0; index++)
    {
        if (bitLen > IR_ENCODE_BYTE_LEN)
        {
            bitLen -= IR_ENCODE_BYTE_LEN;
            codeWidth = IR_ENCODE_BYTE_LEN;
        }
        else
        {
            codeWidth = bitLen;
            bitLen = 0;
        }

        if (IR_Protocol->coding_order == IR_CODING_LSB_FIRST)
        {
            for (bitPos = 0; bitPos < codeWidth; bitPos++)
            {
                if (CheckBitStatus(IR_DataStruct->code[index], bitPos) == IR_ENCODE_LOGICAL_1)
                {
                    IR_DataStruct->irBuf[*pBufLen] = Log1[0];
                    IR_DataStruct->irBuf[*pBufLen + 1] = Log1[1];
                }
                else
                {
                    IR_DataStruct->irBuf[*pBufLen] = Log0[0];
                    IR_DataStruct->irBuf[*pBufLen + 1] = Log0[1];
                }
                *pBufLen += IR_MAX_LOG_WAVFORM_SIZE;
            }
        }
        else
        {
            for (bitPos = codeWidth; bitPos > 0; bitPos--)
            {
                if (CheckBitStatus(IR_DataStruct->code[index], bitPos - 1) == IR_ENCODE_LOGICAL_1)
                {
                    IR_DataStruct->irBuf[*pBufLen] = Log1[0];
                    IR_DataStruct->irBuf[*pBufLen + 1] = Log1[1];
                }
                else
                {
                    IR_DataStruct->irBuf[*pBufLen] = Log0[0];
                    IR_DataStruct->irBuf[*pBufLen + 1] = Log0[1];
                }
                *pBufLen += IR_MAX_LOG_WAVFORM_SIZE;
            }
        }
    }
    return IR_SUCCEED;
}

/**
  * @brief    Encode data to packet.
  * @param  IR_DataStruct: pointer to struct which store IR data code.
  * @param  IR_Protocol: pointer to specify IR protocol structure.
  * @param  in_order: encoding order
  * @retval None
  */
IR_Return_Type IR_Encode(IR_DataTypeDef *IR_DataStruct, IR_ProtocolTypeDef *IR_Protocol,
                         bool in_order)
{
    uint8_t custom_code_len = 0;
    uint16_t index = 0;
    uint8_t bufLen = IR_Protocol->headerLen;

    /* Check parameters */
    if (IR_DataStruct->carrierFreq != IR_Protocol->carrierFreq)
    {
        return IR_FREQENCY_ERROR;
    }

    /* Encoding header */
    for (index = 0; index < IR_Protocol->headerLen; index++)
    {
        IR_DataStruct->irBuf[index] =  ConvertToCarrierCycle(IR_Protocol->headerBuf[index], \
                                                             IR_DataStruct->carrierFreq, IR_Protocol->unit);
    }

    custom_code_len = (IR_DataStruct->custom_bit_len + IR_ENCODE_BIT_LEN_TO_BYTE_LEN) /
                      IR_ENCODE_BYTE_LEN;
    if (in_order == true)
    {
        IR_EncodeCode(IR_DataStruct, IR_Protocol, 0, IR_DataStruct->custom_bit_len, &bufLen);
        IR_EncodeCode(IR_DataStruct, IR_Protocol, custom_code_len, IR_DataStruct->key_bit_len, &bufLen);
    }
    else
    {
        IR_EncodeCode(IR_DataStruct, IR_Protocol, custom_code_len, IR_DataStruct->key_bit_len, &bufLen);
        IR_EncodeCode(IR_DataStruct, IR_Protocol, 0, IR_DataStruct->custom_bit_len, &bufLen);
    }

    /* Encode stop code */
    if (IR_Protocol->stopBuf != 0)
    {
        IR_DataStruct->irBuf[bufLen++] = ConvertToCarrierCycle(IR_Protocol->stopBuf, \
                                                               IR_DataStruct->carrierFreq, IR_Protocol->unit);
    }

#if SOFTWARE_AVOIDANCE
    IR_DataStruct->irBuf[bufLen++] = 0;
#endif

    IR_DataStruct->bufLen = bufLen;

    return IR_SUCCEED;
}

/**
  * @brief    Encode repeat code to packet.
  * @param  IR_DataStruct: pointer to struct which store repeat code data.
  * @param  IR_Protocol: pointer to specify IR protocol structure.
  * @retval None
  */
IR_Return_Type IR_RepeatCodeEncode(IR_DataTypeDef *IR_DataStruct, IR_RepeatCodeTypeDef *IR_Protocol)
{
    uint16_t index = 0;

    /* Error handle */
    if (IR_DataStruct->carrierFreq == 0)
    {
        return IR_FREQENCY_ERROR;
    }

    /* Encode repeat code */
    for (index = 0; index < IR_Protocol->RepeatCodeLen; index++)
    {
        IR_DataStruct->irBuf[index] =  ConvertToCarrierCycle(IR_Protocol->RepeatCodeBuf[index],
                                                             IR_DataStruct->carrierFreq, IR_Protocol->unit);
    }

    IR_DataStruct->bufLen = IR_Protocol->RepeatCodeLen;

#if SOFTWARE_AVOIDANCE
    IR_DataStruct->irBuf[index] = 0;
    IR_DataStruct->bufLen++;
#endif

    return IR_SUCCEED;
}


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

