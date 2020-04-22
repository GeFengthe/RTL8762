/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      nec_prot.c
* @brief    This file provides driver of raw packet protocol encoding.
* @details
* @author  elliot_chen
* @date     2018-07-23
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "raw_pack_prot.h"

/**
  * @brief    Encode data of raw packet protocol.
  * @param   IR_DataStruct: pointer to struct which store RCA code.
  * @retval None
  */
IR_Return_Type Raw_Pack_Encode(IR_DataTypeDef *IR_DataStruct)
{
    uint32_t i = 0;

    for (i = 0; i < IR_DataStruct->bufLen; i++)
    {
        IR_DataStruct->irBuf[i] = ConvertToCarrierCycle(IR_DataStruct->irBuf[i], IR_DataStruct->carrierFreq,
                                                        TIME_UNIT);

        if ((i % 2) == 0)
        {
            IR_DataStruct->irBuf[i] |= IR_PULSE_HIGH;
        }
    }

#if SOFTWARE_AVOIDANCE
    IR_DataStruct->irBuf[IR_DataStruct->bufLen++] = IR_PULSE_HIGH | ConvertToCarrierCycle(560,
                                                    IR_DataStruct->carrierFreq, TIME_UNIT);
    IR_DataStruct->irBuf[IR_DataStruct->bufLen++] = 0;
#endif
    return IR_SUCCEED;
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

