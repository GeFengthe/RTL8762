/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     ir_protocol_config.h
* @brief    ir protocol confguration header file.
* @details
* @author   elliot chen
* @date     2017-08-15
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __IR_PROTOCOL_CONFIG_H
#define __IR_PROTOCOL_CONFIG_H

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"

/**
  * @brief  Configure protocol type which to be used
  */
#define RAW_PACK_PROT
#define NEC_PROT
#define RC5_PROT
#define SONY6124_PROT
#define LC7464M_PROT
#define TC9012_PROT
#define MITC8D8_PROT
#define PIONEER_PROT
#define VICTORC8D8_PROT
#define DVB_PROT
#define SHARP_PROT
#define KONICA_PROT
#define RAW_IR_PROT
#define RC6_32B_PROT

/**
  * @brief  Enable send two group data in one frame situation or not
  */
#define IR_ONE_FRAME_SEND_TWICE

/**
  * @brief  IR send pin definition
  */

#define IR_SEND_PIN             P4_0

/**
  * @brief  IR_DataType definition
  */

typedef uint32_t IR_DataType;

/**
  * @brief  Carrier waveform data type bit select
  */

//#define IR_PACK_TYPE            ((uint32_t)15)

/**
  * @brief  IR data mask
  */

#define IR_DATA_MSK             (0x7fffffffUL)

/**
  * @brief  Carrier waveform data type select
  */

#define IR_PULSE_HIGH           ((uint32_t)0x80000000)


/**
  * @brief  No carrier waveform data type select
  */

#define IR_PULSE_LOW            (0x0)

/**
  * @brief  Configure maxium length of user code and data code
  */

#define IR_MAX_CODE_SIZE        8

/**
  * @brief  Configure maxium length of data sent by IR peripheral
  */

#define IR_MAX_BUF_SIZE         500


/**
  * @brief  Configure maxium length of customer code
  */

#define MAX_CUSTOM_CODE_LEN         4

#define MAX_CUSTOM_CODE_BIT_LEN     (MAX_CUSTOM_CODE_LEN*8)

/**
  * @brief  Configure maxium length of key code
  */

#define MAX_KEY_CODE_LEN            4

#define MAX_KEY_CODE_BIT_LEN        (MAX_KEY_CODE_LEN*8)


/**
  * @brief  IR peripheral parameter definition
  */

//#define IR_TX_MAX_LEN                   (IR_TX_FIFO_SIZE - 1)
#define IR_TX_FIFO_THR_LEVEL            2

/**
  * @brief  TIM peripheral parameter definition
  */

#define IR_TIM                          TIM6
#define IR_TIM_IRQ                      TIMER6_IRQ
#define IR_TIMIntrHandler               Timer6_Handler//Timer6IntrHandler

/* brief: Configure maxium length of repeat code of IR protocol.
  * note: MAX_REPEAT_CODE_LEN >= RepeatCodeLen!!!
  * User can modify this define to adjust this driver to various application.
  */
#define IR_REPEAT_CODE_MAX_LEN      8

/* brief Software avoidance.
 */

#define SOFTWARE_AVOIDANCE          1

/* brief Hadware avoidance.
 */

#define HARDWARE_AVOIDANCE          1

/* brief IR waveform interval time control.
 */

#define IR_WAVEFORM_INTERVAL_CTRL

/**
  * @brief  IR return type of encoding function
  */

typedef enum
{
    IR_SUCCEED              = 0,
    IR_FREQENCY_ERROR       = 1,
    IR_HEARDE_TYPE_ERROR    = 2,
    IR_HEADER_DATA_ERROR    = 3,
    IR_DATA_TYPE_ERROR      = 4,
    IR_DATA_ERROR           = 5
} IR_Return_Type;

/**
  * @brief  IR data structure definition
  */

typedef struct
{
    uint16_t            carrierFreq;
    uint8_t             code[IR_MAX_CODE_SIZE];
    uint8_t             custom_bit_len;
    uint8_t             key_bit_len;
    IR_DataType         irBuf[IR_MAX_BUF_SIZE];
    uint16_t            bufLen;
    volatile bool       isSendRepeatcode;
    bool                isRepeatcodeEncoded;
#ifdef IR_WAVEFORM_INTERVAL_CTRL
    volatile bool       intervalCtrl;
    volatile bool       isTimerOccured;
#endif
} IR_DataTypeDef, *pIR_DataTypeDef;

typedef IR_Return_Type(*IRProtocolEncodeFnCB)(pIR_DataTypeDef);

typedef struct
{
    IRProtocolEncodeFnCB IREncodeFunCb;
    IRProtocolEncodeFnCB IRRepeatCodeEncodeFunCb;
    uint32_t carrierFreq;
    uint32_t repeat_interval;
} IR_ProtocolEncodeLib_t;

#define ABS_TIME(a,b) ((a>b) ? (a-b):(b-a))

#ifdef __cplusplus
}
#endif

#endif /* !defined (__IR_PROTOCOL_CONFIG_H) */

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

