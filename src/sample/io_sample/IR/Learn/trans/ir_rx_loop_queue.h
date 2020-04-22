/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     ir_rx_loop_queue.h
* @brief    ir rx data loop queue
* @details
* @author   yuan
* @date     2019-01-23
* @version  v1.0
* *********************************************************************************************************
*/

#ifndef __IR_RX_LOOP_QUEUE_H
#define __IR_RX_LOOP_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "board.h"


#if (IR_FUN_EN && IR_FUN_LEARN_EN)
#include <string.h>

#include "rtl876x_ir.h"

#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#ifdef IR_RX_PRINT_LOG
#define IR_RX_DBG_BUFFER(module, level, fmt, param_num,...) \
    DBG_BUFFER_##level(TYPE_BUMBLEBEE3, SUBTYPE_FORMAT, module, fmt, param_num, ##__VA_ARGS__)
#else
#define IR_RX_DBG_BUFFER(MODULE, LEVEL, pFormat, para_num,...) ((void)0)
#endif

/**
  * @brief  IR loop queue parameters
  */
#define LOOP_QUEUE_USE_MEMCPY_FUNCTION      0

#define LOOP_QUEUE_BUF_SIZE_MAX             (1024)
#define LOOP_QUEUE_CAPABILITY               (LOOP_QUEUE_BUF_SIZE_MAX-1)

typedef struct
{
    volatile uint16_t   InQueueIndex;   /* index of in queue */
    volatile uint16_t   OutQueueIndex;  /* index of out queue */
    uint32_t            LoopQueueBuf[LOOP_QUEUE_BUF_SIZE_MAX];
    bool                IsOverFlow;
} IR_LoopQueue_TypeDef;

extern IR_LoopQueue_TypeDef IR_RX_Queue;

void ir_loop_queue_init(IR_LoopQueue_TypeDef *pLoopQueue);
bool ir_loop_queue_is_full(IR_LoopQueue_TypeDef *pLoopQueue, uint16_t vLenth);
bool ir_loop_queue_is_empty(IR_LoopQueue_TypeDef *pLoopQueue);
uint16_t ir_loop_queue_get_data_len(IR_LoopQueue_TypeDef *pLoopQueue);
void ir_loop_queue_data_in(IR_LoopQueue_TypeDef *pLoopQueue, uint8_t vLen);
void ir_loop_queue_data_out(IR_LoopQueue_TypeDef *pLoopQueue, uint32_t *pBuf, uint16_t vLen);
void ir_loop_queue_data_copy(IR_LoopQueue_TypeDef *pLoopQueue, uint32_t *pBuf, uint16_t vLen);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __IR_RX_LOOP_QUEUE_H */

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/

