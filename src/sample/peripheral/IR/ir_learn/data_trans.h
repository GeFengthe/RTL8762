/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      data_trans.h
* @brief     data transmission layer driver
* @details
* @author    elliot chen
* @date      2017-11-17
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __DATA_TRANS_H
#define __DATA_TRANS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"
#include "System_rtl876x.h"
#include "ir_learn_config.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_ir.h"
#include "rtl876x_nvic.h"
#include "trace.h"
#include "string.h"

/* Defines ------------------------------------------------------------------*/

#ifdef PRINT_LOG
#define DATATRANS_DBG_BUFFER(module, level, fmt, param_num,...) \
    DBG_BUFFER_##level(TYPE_BUMBLEBEE3, SUBTYPE_FORMAT, module, fmt, param_num, ##__VA_ARGS__)

#else
#define DATATRANS_DBG_BUFFER(MODULE, LEVEL, pFormat, para_num,...) ((void)0)
#endif

/** @defgroup pfn_IR_Learn_HandlerCB   pfn_IR_Learn_HandlerCB
  * @brief Function pointer type used by IR learn interrupt handle to general Callback and send events to application.
  * @{
  */
typedef void (*pfn_IR_Learn_HandlerCB_t)(uint8_t flag);
/** @} End of pfn_IR_Learn_HandlerCB */

/**
 * @brief Loop queue data struct
 */
typedef struct
{
    volatile uint16_t   ReadIndex;                  /* index of read queue */
    volatile uint16_t   WriteIndex;                 /* index of write queue */
    LoopQueue_BufType   buf[LOOP_QUEUE_MAX_SIZE];   /* Buffer for loop queue */
    volatile uint8_t    OverFlow;                   /* loop queue overflow or not */
} LoopQueue_TypeDef;

void DataTrans_DeInit(void);
void DataTrans_Init(void);
void DataTrans_InitQueue(void);
uint16_t DataTrans_FindDataPos(LoopQueue_BufType data, uint16_t len);
uint16_t DataTrans_GetFIFOLen(void);
void DataTrans_ReadBuf(LoopQueue_BufType *pBuf, uint16_t len);
void DataTrans_CopyBuf(LoopQueue_BufType *pBuf, uint16_t len);
BOOL DataTrans_QueueIsEmpty(void);
void DataTrans_INTConfig(FunctionalState state);
void DataTrans_RegisterIRLearnHandlerCB(pfn_IR_Learn_HandlerCB_t pFunc);

#ifdef __cplusplus
}
#endif

#endif /*__UART_TRANS_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

