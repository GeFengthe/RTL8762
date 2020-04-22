/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      ir_trans.h
* @brief
* @details
* @author    elliot chen
* @date      2017-08-15
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __IR_TRANS_H
#define __IR_TRANS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_nvic.h"
#include "ir_protocol_config.h"
#include "rtl876x_ir.h"
#include "rtl876x_tim.h"
#include "trace.h"

#ifdef RAW_PACK_PROT
#include "raw_pack_prot.h"
#endif
#ifdef NEC_PROT
#include "nec_prot.h"
#endif
#ifdef RC5_PROT
#include "rc5_prot.h"
#endif
#ifdef SONY6124_PROT
#include "sony6124_prot.h"
#endif
#ifdef LC7464M_PROT
#include "lc7464m_prot.h"
#endif
#ifdef TC9012_PROT
#include "tc9012_prot.h"
#endif
#ifdef MITC8D8_PROT
#include "mitc8d8_prot.h"
#endif
#ifdef PIONEER_PROT
#include "pioneer_prot.h"
#endif
#ifdef VICTORC8D8_PROT
#include "victorc8d8_prot.h"
#endif
#ifdef DVB_PROT
#include "dvb_prot.h"
#endif
#ifdef SHARP_PROT
#include "sharp_prot.h"
#endif
#ifdef KONICA_PROT
#include "konica_prot.h"
#endif
#ifdef RC6_32B_PROT
#include "rc6_prot.h"
#endif

/* Defines ------------------------------------------------------------------*/

/* brief Universe IR parameters information.
 */

typedef struct _UIR_PARAM_INFO
{
    uint8_t uir_protocol_index;
    uint8_t custom_code_length;
    uint8_t key_code_length;
    uint8_t custom_code_value[MAX_CUSTOM_CODE_LEN];
    uint8_t key_code_value[MAX_KEY_CODE_LEN];

    bool is_second_code_valid;
    uint8_t second_custom_code_length;  /* length in BIT */
    uint8_t second_key_code_length;     /* length in BIT */
    uint8_t second_custom_code_value[MAX_CUSTOM_CODE_LEN];
    uint8_t second_key_code_value[MAX_KEY_CODE_LEN];

#ifdef RAW_PACK_PROT
    uint32_t *pRawPackAddr;
    uint32_t raw_pack_len;
#endif
} UIR_PARAM_INFO;

/**
  * @brief Function pointer type used by interrupt handle to general Callback, to send events to application.
  */
typedef void (*pfnIntrHandlerCB_t)(void);

/**
  * @brief  IR protocols definition
  */

typedef enum
{
    UIR_CODESET_TYPE_RAW_Pack = 0,
    UIR_CODESET_TYPE_NEC,
    UIR_CODESET_TYPE_RC5,
    UIR_CODESET_TYPE_6124,
    UIR_CODESET_TYPE_LC7464M,
    UIR_CODESET_TYPE_TC9012,
    UIR_CODESET_TYPE_MIT_C8D8,
    UIR_CODESET_TYPE_Pioneer,
    UIR_CODESET_TYPE_Victor_C8D8,
    UIR_CODESET_TYPE_DVB,
    UIR_CODESET_TYPE_SHARP,
    UIR_CODESET_TYPE_KONICA,
    UIR_CODESET_TYPE_SP_NEC,
    UIR_CODESET_TYPE_RC6_32B,
} UIR_CODESET_TYPE;

/**
  * @brief  Pioneer protocol send status definition
  */

typedef enum
{
    UIR_ALTERNATE_NO_SENDING,
    UIR_ALTERNATE_SEND_ONE_FRAME_DATA,
    UIR_ALTERNATE_SEND_ONE_FRAME_SECOND_DATA,
    UIR_ALTERNATE_REPEAT_SEND_FIRST_DATA,
    UIR_ALTERNATE_REPEAT_SEND_SECOND_DATA
} UIR_ALTERNATE_SEND_STATUS;

/**
  * @brief  Universe IR status definition
  */

typedef enum
{
    UIR_STATUS_NO_ERROR = 0,
    UIR_STATUS_INVALID_CODESET_TYPE,
    UIR_STATUS_INVALID_KEY_INDEX,
    UIR_STATUS_INVALID_UIR_DATABASE,
    UIR_STATUS_ALREADY_IN_TRANSMITTING,
    UIR_STATUS_ENCODE_ERROR,
    UIR_STATUS_CUSTOM_CODE_OVERFLOW,
    UIR_STATUS_KEY_CODE_OVERFLOW,
    UIR_STATUS_SECOND_CUSTOM_CODE_OVERFLOW,
    UIR_STATUS_SECOND_KEY_CODE_OVERFLOW,
    UIR_STATUS_CUSTOM_KEY_CODE_LEN_ZERO,
    UIR_STATUS_EXCEED_MAX_BUF_LEN,
    UIR_STATUS_SEND_FIRST_FRAME,
    UIR_STATUS_SEND_REPEAT_CODE,
    UIR_STATUS_OTHERS = 0xFF,
} UIR_STATUS;

/**
  * @brief  Enable print log or not
  */

#define PRINT_LOG

#ifdef PRINT_LOG
#define IR_DBG_BUFFER(MODULE, LEVEL, pFormat, para_num,...)  DBG_BUFFER_##LEVEL(MODULE, pFormat, para_num, ##__VA_ARGS__)
#else
#define IR_DBG_BUFFER(MODULE, LEVEL, pFormat, para_num,...) ((void)0)
#endif

void UIR_TransInit(void);
UIR_STATUS UIR_OneFrameTransmitting(UIR_PARAM_INFO *uir_param_info, FunctionalState NewState);
UIR_STATUS UIR_StartContinuouslyTransmitting(UIR_PARAM_INFO *uir_param_info);
UIR_STATUS UIR_StopContinuouslyTransmitting(void);
void UIR_RegisterIRIntrHandlerCB(pfnIntrHandlerCB_t pFunc);
#ifdef IR_WAVEFORM_INTERVAL_CTRL
void UIR_RegisterTIMIntrHandlerCB(pfnIntrHandlerCB_t pFunc);
#endif

#ifdef __cplusplus
}
#endif

#endif /*__IR_TRANS_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

