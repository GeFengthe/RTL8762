/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     ir_trans.c
* @brief        This file provides IR transmission layer driver.
* @details
* @author   elliot chen
* @date         2017-08-15
* @version  v1.0
*********************************************************************************************************
*/

/* Includes -----------------------------------------------------------------*/
#include "ir_trans.h"
#include "string.h"

/* Defines ------------------------------------------------------------------*/

/* Number of protocols used in application */
#define IR_PROTOCOL_TYPE_SUPPORT_NUM    14

#ifdef IR_ONE_FRAME_SEND_TWICE
/* One frame send control which have no repeat code */
#define UIR_ONE_FRAME_COUNT             2
#endif

/* IR protocol encoder function library */
const IR_ProtocolEncodeLib_t ProtocolLib[IR_PROTOCOL_TYPE_SUPPORT_NUM] =
{
    {Raw_Pack_Encode, Raw_Pack_Encode, 38, NEC_TRANSMISION_INTERVAL},
    {NEC_Encode, NEC_RepeatCodeEncode, 38, NEC_TRANSMISION_INTERVAL},
    {RC5_Encode, NULL, 38, RC5_TRANSMISION_INTERVAL},
    {SONY6124_Encode, NULL, 40, SONY6124_TRANSMISION_INTERVAL},
    {LC7464M_Encode, NULL, 38, LC7464M_TRANSMISION_INTERVAL},
    {TC9012_Encode, TC9012_RepeatCodeEncode, 38, TC9012_TRANSMISION_INTERVAL},
    {MITC8D8_Encode, NULL, 38, MITC8D8_TRANSMISION_INTERVAL},
    {PIONEER_Encode, PIONEER_Encode, 40, PIONEER_TRANSMISION_INTERVAL},
    {VICTORC8D8_Encode, NULL, 38, VICTORC8D8_TRANSMISION_INTERVAL},
    {DVB_Encode, NULL, 38, DVB_TRANSMISION_INTERVAL},
    {SHARP_Encode, SHARP_Encode, 38, SHARP_TRANSMISION_INTERVAL},
    {KONICA_Encode, KONICA_RepeatCodeEncode, 38, KONICA_TRANSMISION_INTERVAL},
    {NEC_Encode, NEC_Encode, 38, NEC_TRANSMISION_INTERVAL},
    {RC6_32B_Encode, NULL, 36, RC6_32B_TRANSMISION_INTERVAL},
};

/* Internal defines */
#define IR_BIT_LEN_TO_BYTE_LEN          7
#define IR_BYTE_LEN                     8

/* Globals ------------------------------------------------------------------*/
/* Data structure which store encoded data */
IR_DataTypeDef IR_DataStruct;

/* Number of data which has been sent */
volatile uint16_t ir_tx_count = 0;

/* Data structure which store universal parameter information */
UIR_PARAM_INFO uir_param_info_base;

/* Record Pioneer send status */
static UIR_ALTERNATE_SEND_STATUS uir_pioneer_send_status = UIR_ALTERNATE_NO_SENDING;

/* Record sharp send status */
static UIR_ALTERNATE_SEND_STATUS uir_sharp_send_status = UIR_ALTERNATE_NO_SENDING;

#ifdef IR_ONE_FRAME_SEND_TWICE
/* Record send status in one frame mode */
static volatile uint8_t uir_one_frame_send_count = 0;
static UIR_ALTERNATE_SEND_STATUS uir_one_frame_send_status = UIR_ALTERNATE_NO_SENDING;
#endif

/* Function pointer used to send event and message to app task from IR interrupt handle */
static pfnIntrHandlerCB_t pfnIRIntrHandlerCB = NULL;

#ifdef IR_WAVEFORM_INTERVAL_CTRL
/* Function pointer used to send event and message to app task from Timer interrupt handle when send continuously */
static pfnIntrHandlerCB_t pfnTIMIntrHandlerCB = NULL;
#endif

void IR_Handler(void);

/**
  * @brief  initialization of pinmux settings and pad settings.
  * @param   No parameter.
  * @return  void
  */
void Board_IR_Init(void)
{
    Pad_Config(IR_SEND_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(IR_SEND_PIN, IRDA_TX);
}

/**
  * @brief  Initialize IR peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_IR_Init(uint32_t freq)
{
    /* Disable IR clock */
    RCC_PeriphClockCmd(APBPeriph_IR, APBPeriph_IR_CLOCK, DISABLE);
    /* Enable IR clock */
    RCC_PeriphClockCmd(APBPeriph_IR, APBPeriph_IR_CLOCK, ENABLE);

    /* Initialize IR */
    IR_InitTypeDef IR_InitStruct;
    IR_StructInit(&IR_InitStruct);
    IR_InitStruct.IR_Freq           = freq;
    IR_InitStruct.IR_DutyCycle      = 2; /* !< 1/2 duty cycle */
    IR_InitStruct.IR_Mode           = IR_MODE_TX;
    IR_InitStruct.IR_TxInverse      = IR_TX_DATA_NORMAL;
    IR_InitStruct.IR_TxFIFOThrLevel = IR_TX_FIFO_THR_LEVEL;
    IR_InitStruct.IR_TxIdleLevel    = IR_IDLE_OUTPUT_LOW;
    IR_Init(&IR_InitStruct);
    IR_Cmd(IR_MODE_TX, ENABLE);
    /* Modify IR interrupt handle */
    RamVectorTableUpdate(IR_VECTORn, IR_Handler);

    /* Configure NVIC */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = IR_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Initialize TIM peripheral which used to send repeat code.
  * @param   No parameter.
  * @return  void
  */
void Driver_IR_TIM_Init(void)
{
    /* Enable TIM clock */
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    /* Initialize TIM */
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);
    /* Timer clock = 10MHz */
    TIM_InitStruct.TIM_SOURCE_DIV = TIM_CLOCK_DIVIDER_4;
    TIM_InitStruct.TIM_Period = 0xFFFFFFFF;
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_TimeBaseInit(IR_TIM, &TIM_InitStruct);
    TIM_INTConfig(IR_TIM, ENABLE);

    /* Configure NVIC */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = IR_TIM_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Configure IR repeat code transmission interval and control repeat send function.
  * @param   interval: IR repeat code transmission interval.
  * @param  NewState: Enabel or disable sending repeat code.
  * @return  void
  */
void UIR_BurstSendCmd(uint32_t interval, FunctionalState NewState)
{
    if (NewState != DISABLE)
    {
        /* Enable transmission continuously */
        TIM_Cmd(IR_TIM, DISABLE);
        TIM_ChangePeriod(IR_TIM, interval);
        TIM_Cmd(IR_TIM, ENABLE);
    }
    else
    {
        /* Disable transmission continuously */
        TIM_Cmd(IR_TIM, DISABLE);
    }
}

/**
  * @brief Get timer status.
  * @param  none.
  * @return  execution status.
  */
UIR_STATUS UIR_GetTIMStatus(void)
{
    if (!(IR_TIM->ControlReg & BIT(0)))
    {
        return UIR_STATUS_NO_ERROR;
    }
    else
    {
        return UIR_STATUS_ALREADY_IN_TRANSMITTING;
    }
}

/**
  * @brief  Send IR waveform data.
  * @param  IR_DataStruct: pointer to struct which store IR data code.
  * @return  none.
  */
static void UIR_SendIRWaveform(IR_DataTypeDef *pIR_DataStruct, uint16_t buf_index)
{
    /* Start to send first bytes data of encoded data */
    if (pIR_DataStruct->bufLen >= IR_TX_FIFO_SIZE)
    {
        /* Enable IR threshold interrupt. Only when TX FIFO offset = threshold value, trigger interrupt */
        IR_SetTxThreshold(IR_TX_FIFO_THR_LEVEL);

        IR_SendBuf(&(pIR_DataStruct->irBuf[buf_index]), IR_TX_FIFO_SIZE, DISABLE);

        /* Record number which has been sent */
        ir_tx_count = IR_TX_FIFO_SIZE;

        IR_INTConfig(IR_INT_TF_LEVEL, ENABLE);
    }
    else
    {
        IR_SendBuf(&(pIR_DataStruct->irBuf[buf_index]), pIR_DataStruct->bufLen, DISABLE);

        /* Record number which has been sent */
        ir_tx_count = pIR_DataStruct->bufLen;

        /* Enable IR Tx FIFO empty interrupt */
        IR_INTConfig(IR_INT_TF_EMPTY, ENABLE);
    }
}

/**
  * @brief  copy first code from IR database struct.
  * @param   uir_param_info: universal parameter information.
   * @param  IR_DataStruct: pointer to struct which store IR data code.
  * @return  none.
  */
static void UIR_GetFirstCode(UIR_PARAM_INFO *uir_param_info, IR_DataTypeDef *pIR_DataStruct)
{
    //memset(pIR_DataStruct, 0, sizeof(IR_DataTypeDef));
    pIR_DataStruct->carrierFreq = ProtocolLib[uir_param_info->uir_protocol_index].carrierFreq;
    pIR_DataStruct->custom_bit_len = uir_param_info->custom_code_length;
    pIR_DataStruct->key_bit_len = uir_param_info->key_code_length;

    memcpy(pIR_DataStruct->code, uir_param_info->custom_code_value,
           (uir_param_info->custom_code_length + IR_BIT_LEN_TO_BYTE_LEN) / IR_BYTE_LEN);

    memcpy(pIR_DataStruct->code + (uir_param_info->custom_code_length + IR_BIT_LEN_TO_BYTE_LEN) /
           IR_BYTE_LEN,
           uir_param_info->key_code_value,
           (uir_param_info->key_code_length + IR_BIT_LEN_TO_BYTE_LEN) / IR_BYTE_LEN);
}

/**
  * @brief  copy first code from IR database struct.
  * @param   uir_param_info: universal parameter information.
   * @param  IR_DataStruct: pointer to struct which store IR data code.
  * @return  none.
  */
static void UIR_GetSecondCode(UIR_PARAM_INFO *uir_param_info, IR_DataTypeDef *pIR_DataStruct)
{
    //memset(pIR_DataStruct, 0, sizeof(IR_DataTypeDef));

    pIR_DataStruct->carrierFreq = ProtocolLib[uir_param_info->uir_protocol_index].carrierFreq;
    pIR_DataStruct->custom_bit_len = uir_param_info->second_custom_code_length;
    pIR_DataStruct->key_bit_len = uir_param_info->second_key_code_length;

    memcpy(pIR_DataStruct->code, uir_param_info->second_custom_code_value,
           (uir_param_info->second_custom_code_length + IR_BIT_LEN_TO_BYTE_LEN) / IR_BYTE_LEN);

    memcpy(pIR_DataStruct->code + (uir_param_info->second_custom_code_length + IR_BIT_LEN_TO_BYTE_LEN) /
           IR_BYTE_LEN,
           uir_param_info->second_key_code_value,
           (uir_param_info->second_key_code_length + IR_BIT_LEN_TO_BYTE_LEN) / IR_BYTE_LEN);
}

#ifdef RAW_PACK_PROT
/**
  * @brief  Send IR raw packet protocol data.
  * @param   uir_param_info: universal parameter information.
  * @param  IR_DataStruct: pointer to struct which store IR data code.
  * @param  NewState: Enabel or disable sending repeat code.
  * @return  execution status.
  */
static UIR_STATUS UIR_SendRawPack(UIR_PARAM_INFO *uir_param_info, IR_DataTypeDef *pIR_DataStruct,
                                  FunctionalState NewState)
{
    /* Initialize IR parameters */
    //memset(pIR_DataStruct, 0, sizeof(IR_DataTypeDef));
    pIR_DataStruct->carrierFreq = ProtocolLib[uir_param_info->uir_protocol_index].carrierFreq;

    if (uir_param_info->raw_pack_len <= IR_MAX_BUF_SIZE)
    {
        memcpy(pIR_DataStruct->irBuf, uir_param_info->pRawPackAddr,
               uir_param_info->raw_pack_len * sizeof(pIR_DataStruct->irBuf[0]));
        pIR_DataStruct->bufLen = uir_param_info->raw_pack_len;
    }
    else
    {
        return UIR_STATUS_EXCEED_MAX_BUF_LEN;
    }

    /* Initialize IR peripheral or not */
    if (pIR_DataStruct->carrierFreq != ProtocolLib[uir_param_info->uir_protocol_index].carrierFreq)
    {
        Driver_IR_Init(ProtocolLib[uir_param_info->uir_protocol_index].carrierFreq);
    }

    /* Encode */
    if (ProtocolLib[uir_param_info->uir_protocol_index].IREncodeFunCb)
    {
        if (IR_SUCCEED != (ProtocolLib[uir_param_info->uir_protocol_index].IREncodeFunCb(pIR_DataStruct)))
        {
            return UIR_STATUS_ENCODE_ERROR;
        }
    }
    else
    {
        APP_PRINT_INFO0("No IR protocol encode function");
    }

    /* Start to send first bytes data of encoded data */
    UIR_SendIRWaveform(pIR_DataStruct, 0);

    return UIR_STATUS_NO_ERROR;
}
#endif

/**
  * @brief  Send IR protocol data.
  * @param   uir_param_info: universal parameter information.
  * @param  IR_DataStruct: pointer to struct which store IR data code.
  * @param  NewState: Enabel or disable sending repeat code.
  * @return  execution status.
  */
static UIR_STATUS UIR_SendData(UIR_PARAM_INFO *uir_param_info, IR_DataTypeDef *pIR_DataStruct,
                               FunctionalState NewState)
{
    /* Check parameters */
    if (uir_param_info->uir_protocol_index > IR_PROTOCOL_TYPE_SUPPORT_NUM)
    {
        return UIR_STATUS_INVALID_CODESET_TYPE;
    }

    /* Initialize IR peripheral or not */
    if (pIR_DataStruct->carrierFreq != ProtocolLib[uir_param_info->uir_protocol_index].carrierFreq)
    {
        Driver_IR_Init(ProtocolLib[uir_param_info->uir_protocol_index].carrierFreq);
    }

    /* Initialize IR parameters */
    memset(pIR_DataStruct, 0, sizeof(IR_DataTypeDef));
    pIR_DataStruct->carrierFreq = ProtocolLib[uir_param_info->uir_protocol_index].carrierFreq;
    pIR_DataStruct->custom_bit_len = uir_param_info->custom_code_length;
    pIR_DataStruct->key_bit_len = uir_param_info->key_code_length;

    memcpy(pIR_DataStruct->code, uir_param_info->custom_code_value,
           (uir_param_info->custom_code_length + IR_BIT_LEN_TO_BYTE_LEN) / IR_BYTE_LEN);

    memcpy(pIR_DataStruct->code + (uir_param_info->custom_code_length + IR_BIT_LEN_TO_BYTE_LEN) /
           IR_BYTE_LEN,
           uir_param_info->key_code_value,
           (uir_param_info->key_code_length + IR_BIT_LEN_TO_BYTE_LEN) / IR_BYTE_LEN);

    /* Encode */
    if (ProtocolLib[uir_param_info->uir_protocol_index].IREncodeFunCb)
    {
        if (IR_SUCCEED != (ProtocolLib[uir_param_info->uir_protocol_index].IREncodeFunCb(pIR_DataStruct)))
        {
            return UIR_STATUS_ENCODE_ERROR;
        }
    }
    else
    {
        APP_PRINT_INFO0("No IR protocol encode function");
    }


    /* Configure repeat interval and repeat code sending function */
    if (NewState == ENABLE)
    {
        pIR_DataStruct->isSendRepeatcode = true;
    }

    if ((uir_param_info->uir_protocol_index != UIR_CODESET_TYPE_LC7464M) && \
        (uir_param_info->uir_protocol_index != UIR_CODESET_TYPE_DVB) && \
        (uir_param_info->uir_protocol_index != UIR_CODESET_TYPE_MIT_C8D8) && \
        (uir_param_info->uir_protocol_index != UIR_CODESET_TYPE_KONICA))
    {
        UIR_BurstSendCmd(ProtocolLib[uir_param_info->uir_protocol_index].repeat_interval, NewState);
    }

    /* Start to send first bytes data of encoded data */
    UIR_SendIRWaveform(pIR_DataStruct, 0);

    return UIR_STATUS_NO_ERROR;
}

/**
  * @brief  Send IR pioneer protocol repeat data alternately.(A->B->A->B)
  * @param   uir_param_info: universal parameter information.
   * @param  IR_DataStruct: pointer to struct which store IR data code.
  * @return  none.
  */
static void UIR_PioneerAlternateSendRepeatData(UIR_PARAM_INFO *uir_param_info,
                                               IR_DataTypeDef *pIR_DataStruct)
{
    if (uir_param_info->is_second_code_valid)
    {
        if (uir_pioneer_send_status == UIR_ALTERNATE_REPEAT_SEND_FIRST_DATA)
        {
            uir_pioneer_send_status = UIR_ALTERNATE_REPEAT_SEND_SECOND_DATA;
            UIR_GetSecondCode(uir_param_info, pIR_DataStruct);
        }
        else if (uir_pioneer_send_status == UIR_ALTERNATE_REPEAT_SEND_SECOND_DATA)
        {
            uir_pioneer_send_status = UIR_ALTERNATE_REPEAT_SEND_FIRST_DATA;
            UIR_GetFirstCode(uir_param_info, pIR_DataStruct);
        }
        else
        {
            if (uir_pioneer_send_status == UIR_ALTERNATE_SEND_ONE_FRAME_DATA)
            {
                uir_pioneer_send_status = UIR_ALTERNATE_NO_SENDING;
                UIR_GetSecondCode(uir_param_info, pIR_DataStruct);
                UIR_BurstSendCmd(0, DISABLE);
            }
            else
            {
                APP_PRINT_INFO1("Pioneer error: uir_pioneer_send_status = 0x%x!", uir_pioneer_send_status);
                return;
            }
        }
    }
}

/**
  * @brief  Send IR sharp protocol repeat data alternately.
  * @param   uir_param_info: universal parameter information.
   * @param  IR_DataStruct: pointer to struct which store IR data code.
  * @return  none.
  */
static void UIR_SharpAlternateSendRepeatData(UIR_PARAM_INFO *uir_param_info,
                                             IR_DataTypeDef *pIR_DataStruct)
{
    if (uir_param_info->is_second_code_valid)
    {
        if (uir_sharp_send_status == UIR_ALTERNATE_REPEAT_SEND_FIRST_DATA)
        {
            uir_sharp_send_status = UIR_ALTERNATE_REPEAT_SEND_SECOND_DATA;
            UIR_GetSecondCode(uir_param_info, pIR_DataStruct);
        }
        else if (uir_sharp_send_status == UIR_ALTERNATE_REPEAT_SEND_SECOND_DATA)
        {
            uir_sharp_send_status = UIR_ALTERNATE_REPEAT_SEND_FIRST_DATA;
            UIR_GetFirstCode(uir_param_info, pIR_DataStruct);
        }
        else
        {
            if (uir_sharp_send_status == UIR_ALTERNATE_SEND_ONE_FRAME_DATA)
            {
                uir_sharp_send_status = UIR_ALTERNATE_SEND_ONE_FRAME_SECOND_DATA;
                UIR_GetSecondCode(uir_param_info, pIR_DataStruct);
            }
            else if (uir_sharp_send_status == UIR_ALTERNATE_SEND_ONE_FRAME_SECOND_DATA)
            {
                uir_sharp_send_status = UIR_ALTERNATE_NO_SENDING;
                UIR_GetFirstCode(uir_param_info, pIR_DataStruct);
                UIR_BurstSendCmd(0, DISABLE);
            }
            else
            {
                APP_PRINT_INFO1("Pioneer error: uir_pioneer_send_status = 0x%x!", uir_sharp_send_status);
                return;
            }
        }
    }
}

#ifdef IR_ONE_FRAME_SEND_TWICE
/**
  * @brief  Send IR sony 6124 protocol data continously.
  * @param  none.
  * @return  none.
  */
static uint8_t UIR_OneFrameSendContinuously(void)
{
    if (uir_one_frame_send_count > 0)
    {
        uir_one_frame_send_count--;
        if (!uir_one_frame_send_count)
        {
            IR_DataStruct.isSendRepeatcode = false;
            uir_one_frame_send_status = UIR_ALTERNATE_NO_SENDING;
            UIR_BurstSendCmd(0, DISABLE);
        }
    }

    return uir_one_frame_send_count;
}
#endif

/**
  * @brief  Send IR protocol repeat data.
  * @param   uir_param_info: universal parameter information.
  * @param  IR_DataStruct: pointer to struct which store IR data code.
  * @return  execution status.
  */
static UIR_STATUS UIR_SendRepeatData(UIR_PARAM_INFO *uir_param_info, IR_DataTypeDef *pIR_DataStruct)
{
    /* Check repeat encode function */
    if (ProtocolLib[uir_param_info->uir_protocol_index].IRRepeatCodeEncodeFunCb)
    {
        if ((uir_param_info->uir_protocol_index == UIR_CODESET_TYPE_Pioneer) || \
            (uir_param_info->uir_protocol_index == UIR_CODESET_TYPE_SHARP))
        {
            if (uir_param_info->uir_protocol_index == UIR_CODESET_TYPE_Pioneer)
            {
                UIR_PioneerAlternateSendRepeatData(uir_param_info, pIR_DataStruct);
            }
            else
            {
                UIR_SharpAlternateSendRepeatData(uir_param_info, pIR_DataStruct);
            }

            if (IR_SUCCEED != ProtocolLib[uir_param_info->uir_protocol_index].IRRepeatCodeEncodeFunCb(
                    pIR_DataStruct))
            {
                return UIR_STATUS_ENCODE_ERROR;
            }
        }
        else
        {
            if (pIR_DataStruct->isRepeatcodeEncoded == false)
            {
                if (IR_SUCCEED != ProtocolLib[uir_param_info->uir_protocol_index].IRRepeatCodeEncodeFunCb(
                        pIR_DataStruct))
                {
                    return UIR_STATUS_ENCODE_ERROR;
                }
                pIR_DataStruct->isRepeatcodeEncoded = true;
            }
        }

        UIR_SendIRWaveform(pIR_DataStruct, 0);
    }
    else
    {
        /* Special protocol interval config */
        if (uir_param_info->uir_protocol_index == UIR_CODESET_TYPE_Victor_C8D8)
        {
            if (pIR_DataStruct->isRepeatcodeEncoded == false)
            {
                pIR_DataStruct->bufLen -= 2;
                pIR_DataStruct->isRepeatcodeEncoded = true;
            }

            UIR_BurstSendCmd(VICTORC8D8_TRANSMISION_RE_INTERVAL, ENABLE);

            UIR_SendIRWaveform(pIR_DataStruct, 2);
        }
        else
        {
            UIR_SendIRWaveform(pIR_DataStruct, 0);
        }
    }

#ifdef IR_ONE_FRAME_SEND_TWICE
    if ((uir_param_info->uir_protocol_index != UIR_CODESET_TYPE_Pioneer) && \
        (uir_param_info->uir_protocol_index != UIR_CODESET_TYPE_SHARP))
    {
        if (uir_one_frame_send_status == UIR_ALTERNATE_SEND_ONE_FRAME_DATA)
        {
            UIR_OneFrameSendContinuously();
        }
    }
#endif

    return UIR_STATUS_NO_ERROR;
}

/**
  * @brief  Initialize IR transmission layer module.
  * @param   No parameter.
  * @return  void
  */
void UIR_TransInit(void)
{
    Board_IR_Init();
    memset(&IR_DataStruct, 0, sizeof(IR_DataTypeDef));
    ir_tx_count = 0;
    Driver_IR_Init(38);
    Driver_IR_TIM_Init();
}

/**
  * @brief  Configure universal parameter information.
  * @param   No parameter.
  * @return  void
  */
UIR_STATUS UIR_ParaInit(UIR_PARAM_INFO *uir_param_info)
{
    /* Check parameters */
    if (uir_param_info->custom_code_length > MAX_CUSTOM_CODE_BIT_LEN)
    {
        return UIR_STATUS_CUSTOM_CODE_OVERFLOW;
    }

    if (uir_param_info->key_code_length > MAX_KEY_CODE_BIT_LEN)
    {
        return UIR_STATUS_KEY_CODE_OVERFLOW;
    }

    if ((uir_param_info->custom_code_length + uir_param_info->key_code_length) == 0)
    {
        return UIR_STATUS_CUSTOM_KEY_CODE_LEN_ZERO;
    }

    if (uir_param_info->is_second_code_valid == true)
    {
        if (uir_param_info->second_custom_code_length > MAX_CUSTOM_CODE_BIT_LEN)
        {
            return UIR_STATUS_SECOND_CUSTOM_CODE_OVERFLOW;
        }

        if (uir_param_info->second_key_code_length > MAX_KEY_CODE_BIT_LEN)
        {
            return UIR_STATUS_SECOND_KEY_CODE_OVERFLOW;
        }
    }

    memcpy(&uir_param_info_base, uir_param_info, sizeof(UIR_PARAM_INFO));

    return UIR_STATUS_NO_ERROR;
}

/**
  * @brief  Send one frame universal IR data.
  * @param   uir_param_info: universal parameter information.
  * @return  execution status.
  */
UIR_STATUS UIR_OneFrameTransmitting(UIR_PARAM_INFO *uir_param_info, FunctionalState NewState)
{
#ifdef RAW_PACK_PROT
    if (uir_param_info->uir_protocol_index == UIR_CODESET_TYPE_RAW_Pack)
    {
        memcpy(&uir_param_info_base, uir_param_info, sizeof(UIR_PARAM_INFO));
        return UIR_SendRawPack(&uir_param_info_base, &IR_DataStruct, DISABLE);
    }
#endif

    UIR_STATUS status = UIR_ParaInit(uir_param_info);

    if (status != UIR_STATUS_NO_ERROR)
    {
        return status;
    }

    if (uir_param_info->uir_protocol_index == UIR_CODESET_TYPE_Pioneer)
    {
        uir_pioneer_send_status = UIR_ALTERNATE_SEND_ONE_FRAME_DATA;

        return UIR_SendData(&uir_param_info_base, &IR_DataStruct, ENABLE);
    }
    else if (uir_param_info->uir_protocol_index == UIR_CODESET_TYPE_SHARP)
    {
        uir_sharp_send_status = UIR_ALTERNATE_SEND_ONE_FRAME_DATA;

        return UIR_SendData(&uir_param_info_base, &IR_DataStruct, ENABLE);
    }
#ifdef IR_ONE_FRAME_SEND_TWICE
    /* Send two times */
    else
    {
        if (NewState == ENABLE)
        {
            uir_one_frame_send_count = UIR_ONE_FRAME_COUNT;
            uir_one_frame_send_status = UIR_ALTERNATE_SEND_ONE_FRAME_DATA;
            UIR_OneFrameSendContinuously();
            return UIR_SendData(&uir_param_info_base, &IR_DataStruct, ENABLE);
        }
        else
        {
            return UIR_SendData(&uir_param_info_base, &IR_DataStruct, DISABLE);
        }
    }
#else
    else
    {
        return UIR_SendData(&uir_param_info_base, &IR_DataStruct, DISABLE);
    }
#endif
}

/**
  * @brief  Send universal IR data continuously.
  * @param   uir_param_info: universal parameter information.
  * @return  execution status.
  */
UIR_STATUS UIR_StartContinuouslyTransmitting(UIR_PARAM_INFO *uir_param_info)
{
    UIR_STATUS status = UIR_ParaInit(uir_param_info);

    if (status != UIR_STATUS_NO_ERROR)
    {
        return status;
    }

    /* Special protocol control */
    if (uir_param_info->uir_protocol_index == UIR_CODESET_TYPE_Pioneer)
    {
        uir_pioneer_send_status = UIR_ALTERNATE_REPEAT_SEND_FIRST_DATA;
    }

    if (uir_param_info->uir_protocol_index == UIR_CODESET_TYPE_SHARP)
    {
        uir_sharp_send_status = UIR_ALTERNATE_REPEAT_SEND_FIRST_DATA;
    }

    return UIR_SendData(&uir_param_info_base, &IR_DataStruct, ENABLE);
}

/**
  * @brief  Stop sending universal IR data continuously.
  * @param   uir_param_info: universal parameter information.
  * @return  execution status.
  */
UIR_STATUS UIR_StopContinuouslyTransmitting(void)
{
#ifdef IR_WAVEFORM_INTERVAL_CTRL
    IR_DataStruct.intervalCtrl = true;

    if (IR_DataStruct.isTimerOccured)
    {
        return UIR_STATUS_SEND_REPEAT_CODE;
    }
    else
    {
        /* Stop to send repeat code */
        UIR_BurstSendCmd(0, DISABLE);
        IR_DataStruct.isSendRepeatcode = false;
        IR_DataStruct.intervalCtrl = false;
        IR_DataStruct.isTimerOccured = false;

        if (ir_tx_count > 0)
        {
            return UIR_STATUS_ALREADY_IN_TRANSMITTING;
        }
        else
        {
            return UIR_STATUS_SEND_FIRST_FRAME;
        }
    }
#else
    /* Stop to send repeat code */
    IR_DataStruct.isSendRepeatcode = false;
    UIR_BurstSendCmd(0, DISABLE);

    if (ir_tx_count > 0)
    {
        return UIR_STATUS_ALREADY_IN_TRANSMITTING;
    }

    return UIR_STATUS_NO_ERROR;
#endif
}

/**
 * @brief Register callback function to send events from IR interrupt handle to application.
 * @param pFunc callback function.
 * @return none
*/
void UIR_RegisterIRIntrHandlerCB(pfnIntrHandlerCB_t pFunc)
{
    pfnIRIntrHandlerCB = pFunc;
}

#ifdef IR_WAVEFORM_INTERVAL_CTRL
/**
 * @brief Register callback function to send events from Timer interrupt handle to application.
 * @param pFunc callback function.
 * @return none
*/
void UIR_RegisterTIMIntrHandlerCB(pfnIntrHandlerCB_t pFunc)
{
    pfnTIMIntrHandlerCB = pFunc;
}
#endif

/**
* @brief IR interrupt handler function.
* @param   No parameter.
* @return  void
*/
void IR_Handler(void)
{
    /* Mask IR interrupt */
    IR_MaskINTConfig(IR_INT_TF_EMPTY, ENABLE);
    IR_MaskINTConfig(IR_INT_TF_LEVEL, ENABLE);

    /* Continue to send by interrupt */
    if (IR_GetINTStatus(IR_INT_TF_LEVEL) == SET)
    {
        /* The remaining data is larger than the TX FIFO length */
        if ((IR_DataStruct.bufLen - ir_tx_count) >= IR_TX_FIFO_SIZE)
        {
            IR_SendBuf(IR_DataStruct.irBuf + ir_tx_count, IR_TX_FIFO_SIZE - IR_TX_FIFO_THR_LEVEL, DISABLE);
            ir_tx_count += (IR_TX_FIFO_SIZE - IR_TX_FIFO_THR_LEVEL);
        }
        else if ((IR_DataStruct.bufLen - ir_tx_count) > 0)
        {
            /* The remaining data is less than the TX FIFO length */
            IR_INTConfig(IR_INT_TF_LEVEL, DISABLE);

            /* Enable IR tx empty interrupt */
            IR_INTConfig(IR_INT_TF_EMPTY, ENABLE);
            IR_SendBuf(IR_DataStruct.irBuf + ir_tx_count, IR_DataStruct.bufLen - ir_tx_count, DISABLE);
            ir_tx_count = IR_DataStruct.bufLen;
        }
        else
        {
            /* Disable IR tx empty interrupt */
            IR_INTConfig(IR_INT_TF_LEVEL, DISABLE);
            /* Enable IR tx empty interrupt */
            IR_INTConfig(IR_INT_TF_EMPTY, ENABLE);

            //Add application code here!!!

        }
        /* Clear threshold interrupt */
        IR_ClearINTPendingBit(IR_INT_TF_LEVEL_CLR);
    }

    if (IR_GetINTStatus(IR_INT_TF_EMPTY) == SET)
    {
        /* Configure repeat interval and repeat code sending function */
        if (IR_DataStruct.isSendRepeatcode)
        {
            if ((uir_param_info_base.uir_protocol_index == UIR_CODESET_TYPE_LC7464M) || \
                (uir_param_info_base.uir_protocol_index == UIR_CODESET_TYPE_DVB) || \
                (uir_param_info_base.uir_protocol_index == UIR_CODESET_TYPE_MIT_C8D8) || \
                (uir_param_info_base.uir_protocol_index == UIR_CODESET_TYPE_KONICA))
            {
                UIR_BurstSendCmd(ProtocolLib[uir_param_info_base.uir_protocol_index].repeat_interval, ENABLE);
            }
        }

        /* Tx completed */
        ir_tx_count = 0;
        /* Disable IR tx empty interrupt */
        IR_INTConfig(IR_INT_TF_EMPTY, DISABLE);

        /* Clear interrupt flag */
        IR_ClearINTPendingBit(IR_INT_TF_EMPTY_CLR);

        if (UIR_GetTIMStatus() == UIR_STATUS_ALREADY_IN_TRANSMITTING)
        {
            /* Unmask IR interrupt */
            IR_MaskINTConfig(IR_INT_TF_LEVEL, DISABLE);
            IR_MaskINTConfig(IR_INT_TF_EMPTY, DISABLE);
            return;
        }
        //Add application code here!!!
        //Send Msg to App task to inform that IR complete a complete transmission!!!

        if (pfnIRIntrHandlerCB)
        {
            pfnIRIntrHandlerCB();
        }
    }

    /* Unmask IR interrupt */
    IR_MaskINTConfig(IR_INT_TF_LEVEL, DISABLE);
    IR_MaskINTConfig(IR_INT_TF_EMPTY, DISABLE);
}

/**
* @brief TIM interrupt handler function.
* @param   No parameter.
* @return  void
*/
void IR_TIMIntrHandler(void)
{
    // Add application code here
    if (ir_tx_count == 0)
    {
#ifdef IR_WAVEFORM_INTERVAL_CTRL
        IR_DataStruct.isTimerOccured = true;

        if (IR_DataStruct.intervalCtrl)
        {
            UIR_BurstSendCmd(0, DISABLE);
            /* Stop to send repeat code */
            IR_DataStruct.isSendRepeatcode = false;
            IR_DataStruct.intervalCtrl = false;
            IR_DataStruct.isTimerOccured = false;
            TIM_ClearINT(IR_TIM);

            /* Notify To app task */
            if (pfnTIMIntrHandlerCB)
            {
                pfnTIMIntrHandlerCB();
            }
            return;
        }
#endif
        UIR_SendRepeatData(&uir_param_info_base, &IR_DataStruct);
    }
    else
    {
        //IR send error
    }

    TIM_ClearINT(IR_TIM);
}

/******************* (C) COPYRIGHT 2016 Realtek Semiconductor Corporation *****END OF FILE****/
