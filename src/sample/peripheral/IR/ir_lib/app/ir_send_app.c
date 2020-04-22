/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     ir_send_app.c
* @brief    This file provides IR transmission application code.
* @details
* @author   elliot chen
* @date     2018-3-20
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "ir_send_app.h"
#include "ir_led.h"
#include "platform_utils.h"

/* Defines ------------------------------------------------------------------*/

/* Globals -------------------------------------------------------------------*/
UIR_PARAM_INFO uir_param_info;
extern bool app_send_msg_to_apptask(T_IO_MSG *p_msg);

/**
  * @brief  Callback function which send message from IR interrupt handle to app task.
  * @param   void.
  * @return  void.
  */
void sendMsgToAppTaskFromISR(void)
{
    T_IO_MSG bee_io_msg;
    bee_io_msg.type = IO_MSG_TYPE_IR;

    /*send ir learn data message to app*/
    bee_io_msg.subtype = IO_MSG_TYPE_IR_SEND_COMPLETE;
    app_send_msg_to_apptask(&bee_io_msg);
}

/**
  * @brief  IR send initialization.
  * @param   void.
  * @return  void.
  */
void IR_Send_Init(void)
{
    /* Initialize IR send module */
    UIR_TransInit();

    /* Register IR interrupt handle callback function */
    UIR_RegisterIRIntrHandlerCB(sendMsgToAppTaskFromISR);
}

/**
  * @brief  Raw Packet send demo.
  * @param   void.
  * @return  void.
  */
void IR_Raw_Packet_Send(uint32_t *pBuf, uint32_t len)
{
    UIR_STATUS status;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index   = UIR_CODESET_TYPE_RAW_Pack;
    uir_param_info.pRawPackAddr = pBuf;
    uir_param_info.raw_pack_len = len;

    status = UIR_OneFrameTransmitting(&uir_param_info, DISABLE);
    if (UIR_STATUS_NO_ERROR == status)
    {
        LED_IR_Send_Swap();
    }
}

/**
  * @brief  NEC send demo.
  * @param   void.
  * @return  void.
  */
void NEC_SendCode(void)
{
    UIR_STATUS status;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index   = UIR_CODESET_TYPE_NEC;
    uir_param_info.custom_code_value[0] = 0x55;
    uir_param_info.custom_code_value[1] = (uint8_t)(~(0x55));
    uir_param_info.custom_code_length   = 16;
    uir_param_info.key_code_value[0]    = 0xAA;
    uir_param_info.key_code_value[1]    = (uint8_t)(~(0xAA));
    uir_param_info.key_code_length      = 16;

    UIR_OneFrameTransmitting(&uir_param_info, DISABLE);

    platform_delay_ms(2000);

    status = UIR_StartContinuouslyTransmitting(&uir_param_info);

    if (status == UIR_STATUS_NO_ERROR)
    {
        // Start IR send successfully
    }
    else
    {
        if (status == UIR_STATUS_INVALID_CODESET_TYPE)
        {
            // Invalid IR protocol type!
        }
    }

    platform_delay_ms(2000);

    status = UIR_StopContinuouslyTransmitting();

    if (status == UIR_STATUS_ALREADY_IN_TRANSMITTING)
    {
        // IR is sending data, waiting for IR tx empty interrupt
        // Add application code here
    }
    else
    {
        if (status == UIR_STATUS_NO_ERROR)
        {
            // IR has stopped sending data
            // Add application code here
        }
    }

    platform_delay_ms(1000);
}

/**
  * @brief  RC5 send demo.
  * @param   void.
  * @return  void.
  */
void RC5_SendCode(void)
{
    UIR_STATUS status;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index   = UIR_CODESET_TYPE_RC5;
    uir_param_info.custom_code_value[0] = 0x0A;
    uir_param_info.custom_code_length   = 5;
    uir_param_info.key_code_value[0]    = 0x3A | (0 << 6);
    uir_param_info.key_code_length      = 6;

    status = UIR_StartContinuouslyTransmitting(&uir_param_info);

    if (status == UIR_STATUS_NO_ERROR)
    {
        // Start IR send successfully
    }
    else
    {
        if (status == UIR_STATUS_INVALID_CODESET_TYPE)
        {
            // Invalid IR protocol type!
        }
    }

    platform_delay_ms(1000);

    status = UIR_StopContinuouslyTransmitting();

    if (status == UIR_STATUS_ALREADY_IN_TRANSMITTING)
    {
        // IR is sending data, waiting for IR tx empty interrupt
        // Add application code here
    }
    else
    {
        if (status == UIR_STATUS_NO_ERROR)
        {
            // IR has stopped sending data
            // Add application code here
        }
    }

    platform_delay_ms(1000);
}

/**
  * @brief  RC5 send demo.
  * @param   void.
  * @return  void.
  */
void RC6_SendCode(void)
{
    UIR_STATUS status;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index   = UIR_CODESET_TYPE_RC6_32B;
    uir_param_info.custom_code_value[0] = 0x80;
    uir_param_info.custom_code_value[1] = 0x10;
    uir_param_info.custom_code_length   = 16;
    uir_param_info.key_code_value[0]    = 0x26 | (0 << 7);
    uir_param_info.key_code_value[1]    = 0x0C;
    uir_param_info.key_code_length      = 16;

    status = UIR_StartContinuouslyTransmitting(&uir_param_info);

    if (status == UIR_STATUS_NO_ERROR)
    {
        // Start IR send successfully
    }
    else
    {
        if (status == UIR_STATUS_INVALID_CODESET_TYPE)
        {
            // Invalid IR protocol type!
        }
    }

    platform_delay_ms(1000);

    status = UIR_StopContinuouslyTransmitting();

    if (status == UIR_STATUS_ALREADY_IN_TRANSMITTING)
    {
        // IR is sending data, waiting for IR tx empty interrupt
        // Add application code here
    }
    else
    {
        if (status == UIR_STATUS_NO_ERROR)
        {
            // IR has stopped sending data
            // Add application code here
        }
    }

    platform_delay_ms(1000);
}

/**
  * @brief  SONY6124 send demo.
  * @param   void.
  * @return  void.
  */
void SONY6124_SendCode(void)
{
    UIR_STATUS status;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index   = UIR_CODESET_TYPE_6124;
    uir_param_info.custom_code_value[0] = 0x2A;
    uir_param_info.custom_code_length   = 8;
    uir_param_info.key_code_value[0]    = 0x1A;
    uir_param_info.key_code_length      = 8;

    status = UIR_StartContinuouslyTransmitting(&uir_param_info);

    if (status == UIR_STATUS_NO_ERROR)
    {
        // Start IR send successfully
    }
    else
    {
        if (status == UIR_STATUS_INVALID_CODESET_TYPE)
        {
            // Invalid IR protocol type!
        }
    }

    platform_delay_ms(1000);

    status = UIR_StopContinuouslyTransmitting();

    if (status == UIR_STATUS_ALREADY_IN_TRANSMITTING)
    {
        // IR is sending data, waiting for IR tx empty interrupt
        // Add application code here
    }
    else
    {
        if (status == UIR_STATUS_NO_ERROR)
        {
            // IR has stopped sending data
            // Add application code here
        }
    }

    platform_delay_ms(1000);
}

/**
  * @brief  LC7464M send demo.
  * @param   void.
  * @return  void.
  */
void LC7464M_SendCode(void)
{
    UIR_STATUS status;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index   = UIR_CODESET_TYPE_LC7464M;
    uir_param_info.custom_code_value[0] = 0x55;
    uir_param_info.custom_code_length   = 8;
    uir_param_info.key_code_value[0]    = 0xAA;
    uir_param_info.key_code_length      = 8;

    status = UIR_StartContinuouslyTransmitting(&uir_param_info);

    if (status == UIR_STATUS_NO_ERROR)
    {
        // Start IR send successfully
    }
    else
    {
        if (status == UIR_STATUS_INVALID_CODESET_TYPE)
        {
            // Invalid IR protocol type!
        }
    }

    platform_delay_ms(1000);

    status = UIR_StopContinuouslyTransmitting();

    if (status == UIR_STATUS_ALREADY_IN_TRANSMITTING)
    {
        // IR is sending data, waiting for IR tx empty interrupt
        // Add application code here
    }
    else
    {
        if (status == UIR_STATUS_NO_ERROR)
        {
            // IR has stopped sending data
            // Add application code here
        }
    }

    platform_delay_ms(1000);
}

/**
  * @brief  TC9012 send demo.
  * @param   void.
  * @return  void.
  */
void TC9012_SendCode(void)
{
    UIR_STATUS status;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index   = UIR_CODESET_TYPE_TC9012;
    uir_param_info.custom_code_value[0] = 0x55;
    uir_param_info.custom_code_value[1] = (uint8_t)~(0x55);
    uir_param_info.custom_code_length   = 16;
    uir_param_info.key_code_value[0]    = 0xAA;
    uir_param_info.key_code_value[1]    = (uint8_t)~(0xAA);
    uir_param_info.key_code_length      = 16;

    status = UIR_StartContinuouslyTransmitting(&uir_param_info);

    if (status == UIR_STATUS_NO_ERROR)
    {
        // Start IR send successfully
    }
    else
    {
        if (status == UIR_STATUS_INVALID_CODESET_TYPE)
        {
            // Invalid IR protocol type!
        }
    }

    platform_delay_ms(1000);

    status = UIR_StopContinuouslyTransmitting();

    if (status == UIR_STATUS_ALREADY_IN_TRANSMITTING)
    {
        // IR is sending data, waiting for IR tx empty interrupt
        // Add application code here
    }
    else
    {
        if (status == UIR_STATUS_NO_ERROR)
        {
            // IR has stopped sending data
            // Add application code here
        }
    }

    platform_delay_ms(1000);
}

/**
  * @brief  MITC8D8 send demo.
  * @param   void.
  * @return  void.
  */
void MITC8D8_SendCode(void)
{
    UIR_STATUS status;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index   = UIR_CODESET_TYPE_MIT_C8D8;
    uir_param_info.custom_code_value[0] = 0x55;
    uir_param_info.custom_code_length   = 8;
    uir_param_info.key_code_value[0]    = 0xAA;
    uir_param_info.key_code_length      = 8;

    status = UIR_StartContinuouslyTransmitting(&uir_param_info);

    if (status == UIR_STATUS_NO_ERROR)
    {
        // Start IR send successfully
    }
    else
    {
        if (status == UIR_STATUS_INVALID_CODESET_TYPE)
        {
            // Invalid IR protocol type!
        }
    }

    platform_delay_ms(1000);

    status = UIR_StopContinuouslyTransmitting();

    if (status == UIR_STATUS_ALREADY_IN_TRANSMITTING)
    {
        // IR is sending data, waiting for IR tx empty interrupt
        // Add application code here
    }
    else
    {
        if (status == UIR_STATUS_NO_ERROR)
        {
            // IR has stopped sending data
            // Add application code here
        }
    }

    platform_delay_ms(1000);
}

/**
  * @brief  PIONEER send demo.
  * @param   void.
  * @return  void.
  */
void PIONEER_SendCode(void)
{
    UIR_STATUS status;

    uint16_t custom = 0x55AA;
    uint16_t data = 0xAA55;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index           = UIR_CODESET_TYPE_Pioneer;
    uir_param_info.custom_code_value[0]         = custom & 0xFF;
    uir_param_info.custom_code_value[1]         = (custom >> 8) & 0xFF;
    uir_param_info.custom_code_length           = 16;
    uir_param_info.key_code_value[0]            = data & 0xFF;
    uir_param_info.key_code_value[1]            = (data >> 8) & 0xFF;
    uir_param_info.key_code_length              = 16;
    uir_param_info.is_second_code_valid         = 1;
    uir_param_info.second_custom_code_value[0]  = data & 0xFF;
    uir_param_info.second_custom_code_value[1]  = (data >> 8) & 0xFF;
    uir_param_info.second_custom_code_length    = 16;
    uir_param_info.second_key_code_value[0]     = custom & 0xFF;
    uir_param_info.second_key_code_value[1]     = (custom >> 8) & 0xFF;
    uir_param_info.second_key_code_length       = 16;

    status = UIR_StartContinuouslyTransmitting(&uir_param_info);

    if (status == UIR_STATUS_NO_ERROR)
    {
        // Start IR send successfully
    }
    else
    {
        if (status == UIR_STATUS_INVALID_CODESET_TYPE)
        {
            // Invalid IR protocol type!
        }
    }

    platform_delay_ms(1000);

    status = UIR_StopContinuouslyTransmitting();

    if (status == UIR_STATUS_ALREADY_IN_TRANSMITTING)
    {
        // IR is sending data, waiting for IR tx empty interrupt
        // Add application code here
    }
    else
    {
        if (status == UIR_STATUS_NO_ERROR)
        {
            // IR has stopped sending data
            // Add application code here
        }
    }

    platform_delay_ms(1000);
}

/**
  * @brief  VICTORC8D8 send demo.
  * @param   void.
  * @return  void.
  */
void VICTORC8D8_SendCode(void)
{
    UIR_STATUS status;

    uint8_t custom = 0x55;
    uint8_t data = 0xAA;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index     = UIR_CODESET_TYPE_Victor_C8D8;
    uir_param_info.custom_code_value[0] = custom;
    uir_param_info.custom_code_length     = 8;
    uir_param_info.key_code_value[0]     = data;
    uir_param_info.key_code_length         = 8;

    status = UIR_StartContinuouslyTransmitting(&uir_param_info);

    if (status == UIR_STATUS_NO_ERROR)
    {
        // Start IR send successfully
    }
    else
    {
        if (status == UIR_STATUS_INVALID_CODESET_TYPE)
        {
            // Invalid IR protocol type!
        }
    }

    platform_delay_ms(1000);

    status = UIR_StopContinuouslyTransmitting();

    if (status == UIR_STATUS_ALREADY_IN_TRANSMITTING)
    {
        // IR is sending data, waiting for IR tx empty interrupt
        // Add application code here
    }
    else
    {
        if (status == UIR_STATUS_NO_ERROR)
        {
            // IR has stopped sending data
            // Add application code here
        }
    }

    platform_delay_ms(1000);
}

/**
  * @brief  DVB send demo.
  * @param   void.
  * @return  void.
  */
void DVB_SendCode(void)
{
    UIR_STATUS status;

    uint32_t custom = 0x12345678;
    uint16_t data = 0x55AA;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index   = UIR_CODESET_TYPE_DVB;
    uir_param_info.custom_code_value[0] = custom & 0xFF;
    uir_param_info.custom_code_value[1] = (custom >> 8) & 0xFF;
    uir_param_info.custom_code_value[2] = (custom >> 16) & 0xFF;
    uir_param_info.custom_code_value[3] = (custom >> 24) & 0xFF;
    uir_param_info.custom_code_length   = 32;
    uir_param_info.key_code_value[0]    = data & 0xFF;
    uir_param_info.key_code_value[1]    = (data >> 8) & 0xFF;
    uir_param_info.key_code_length      = 16;

    status = UIR_StartContinuouslyTransmitting(&uir_param_info);

    if (status == UIR_STATUS_NO_ERROR)
    {
        // Start IR send successfully
    }
    else
    {
        if (status == UIR_STATUS_INVALID_CODESET_TYPE)
        {
            // Invalid IR protocol type!
        }
    }

    platform_delay_ms(1000);

    status = UIR_StopContinuouslyTransmitting();

    if (status == UIR_STATUS_ALREADY_IN_TRANSMITTING)
    {
        // IR is sending data, waiting for IR tx empty interrupt
        // Add application code here
    }
    else
    {
        if (status == UIR_STATUS_NO_ERROR)
        {
            // IR has stopped sending data
            // Add application code here
        }
    }

    platform_delay_ms(1000);
}

/**
  * @brief  SHARP send demo.
  * @param   void.
  * @return  void.
  */
void SHARP_SendCode(void)
{
    UIR_STATUS status;

    uint16_t data = 0x35A;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index   = UIR_CODESET_TYPE_SHARP;
    uir_param_info.key_code_value[0]    = data & 0xFF;
    uir_param_info.key_code_value[0]    = (data >> 8) & 0xFF;
    uir_param_info.key_code_length      = 16;

    status = UIR_StartContinuouslyTransmitting(&uir_param_info);

    if (status == UIR_STATUS_NO_ERROR)
    {
        // Start IR send successfully
    }
    else
    {
        if (status == UIR_STATUS_INVALID_CODESET_TYPE)
        {
            // Invalid IR protocol type!
        }
    }

    platform_delay_ms(1000);

    status = UIR_StopContinuouslyTransmitting();

    if (status == UIR_STATUS_ALREADY_IN_TRANSMITTING)
    {
        // IR is sending data, waiting for IR tx empty interrupt
        // Add application code here
    }
    else
    {
        if (status == UIR_STATUS_NO_ERROR)
        {
            // IR has stopped sending data
            // Add application code here
        }
    }

    platform_delay_ms(1000);
}

/**
  * @brief KONICA send demo.
  * @param   void.
  * @return  void.
  */
void KONICA_SendCode(void)
{
    UIR_STATUS status;

    memset(&uir_param_info, 0, sizeof(uir_param_info));
    uir_param_info.uir_protocol_index   = UIR_CODESET_TYPE_KONICA;
    uir_param_info.custom_code_value[0] = 0x55;
    uir_param_info.custom_code_value[1] = (uint8_t)(~(0x55));
    uir_param_info.custom_code_length   = 16;
    uir_param_info.key_code_value[0]    = 0xAA;
    uir_param_info.key_code_value[1]    = (uint8_t)(~(0xAA));
    uir_param_info.key_code_length      = 16;

    //UIR_OneFrameTransmitting(&uir_param_info);
    status = UIR_StartContinuouslyTransmitting(&uir_param_info);

    if (status == UIR_STATUS_NO_ERROR)
    {
        // Start IR send successfully
    }
    else
    {
        if (status == UIR_STATUS_INVALID_CODESET_TYPE)
        {
            // Invalid IR protocol type!
        }
    }

    platform_delay_ms(1000);

    status = UIR_StopContinuouslyTransmitting();

    if (status == UIR_STATUS_ALREADY_IN_TRANSMITTING)
    {
        // IR is sending data, waiting for IR tx empty interrupt
        // Add application code here
    }
    else
    {
        if (status == UIR_STATUS_NO_ERROR)
        {
            // IR has stopped sending data
            // Add application code here
        }
    }

    platform_delay_ms(1000);
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

