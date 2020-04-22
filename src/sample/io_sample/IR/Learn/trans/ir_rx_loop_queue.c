/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     ir_rx_loop_queue.c
* @brief
* @details
* @author   yuan
* @date     2019-01-23
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "ir_rx_loop_queue.h"

#if (IR_FUN_EN && IR_FUN_LEARN_EN)

/* Globals ------------------------------------------------------------------ */
/* Loop queue management data structure */
IR_LoopQueue_TypeDef   IR_RX_Queue;


/* Functions ------------------------------------------------------------------*/

/**
  * @brief  Initializes loop queue to their default reset values.
  * @param  pPacket: point to loop queue data structure.
  * @retval None
  */
void ir_loop_queue_init(IR_LoopQueue_TypeDef *pLoopQueue)
{
    memset(pLoopQueue, 0, sizeof(IR_LoopQueue_TypeDef));
}

/**
  * @brief  check loop queue if will full or not.
  * @param  pLoopQueue: point to loop queue dta struct.
  * @retval The new state of loop queue (full:TRUE, not full:FALSE).
  */
bool ir_loop_queue_is_full(IR_LoopQueue_TypeDef *pLoopQueue, uint16_t vLenth)
{
    /* Check loop queue status */
    if (pLoopQueue->InQueueIndex >= pLoopQueue->OutQueueIndex)
    {
        if ((pLoopQueue->InQueueIndex + vLenth) >= (LOOP_QUEUE_BUF_SIZE_MAX + pLoopQueue->OutQueueIndex))
        {
            IR_RX_DBG_BUFFER(MODULE_APP, LEVEL_ERROR,
                             "[ir_trans_rx]loop_queue_is_full: Loop Queue is overflow!", 0);
            pLoopQueue->IsOverFlow = true;
            return true;
        }
    }
    else
    {
        if ((pLoopQueue->InQueueIndex + vLenth) >= pLoopQueue->OutQueueIndex)
        {
            IR_RX_DBG_BUFFER(MODULE_APP, LEVEL_ERROR,
                             "[ir_trans_rx]loop_queue_is_full: Loop Queue is overflow!", 0);
            pLoopQueue->IsOverFlow = true;
            return true;
        }
    }
    return false;
}

/**
  * @brief  Check loop queue if empty or not.
  * @param  pLoopQueue: point to IR packet struct.
  * @retval The new state of loop queue (empty:true, not empty:false).
  */
bool ir_loop_queue_is_empty(IR_LoopQueue_TypeDef *pLoopQueue)
{
    return ((pLoopQueue->InQueueIndex) == pLoopQueue->OutQueueIndex);
}

/**
  * @brief  Get valid data length.
  * @param  pLoopQueue:
  * @retval valid data length
  */
uint16_t ir_loop_queue_get_data_len(IR_LoopQueue_TypeDef *pLoopQueue)
{
    return ((LOOP_QUEUE_BUF_SIZE_MAX + pLoopQueue->InQueueIndex - pLoopQueue->OutQueueIndex) &
            LOOP_QUEUE_CAPABILITY);
}

/**
  * @brief  Write buffer data to loop queue.
  * @param  None.
  * @retval None.
  */
void ir_loop_queue_data_in(IR_LoopQueue_TypeDef *pLoopQueue, uint8_t vLen)
{
    uint8_t tail_data_len = 0;

    if (!ir_loop_queue_is_full(pLoopQueue, vLen))
    {
        if (pLoopQueue->InQueueIndex + vLen <= LOOP_QUEUE_BUF_SIZE_MAX)
        {
            IR_ReceiveBuf(&(pLoopQueue->LoopQueueBuf[pLoopQueue->InQueueIndex]), vLen);
            pLoopQueue->InQueueIndex += vLen;
        }
        else
        {
            tail_data_len = LOOP_QUEUE_BUF_SIZE_MAX - pLoopQueue->InQueueIndex;
            IR_ReceiveBuf(&(pLoopQueue->LoopQueueBuf[pLoopQueue->InQueueIndex]), tail_data_len);
            pLoopQueue->InQueueIndex = 0;
            IR_ReceiveBuf(&(pLoopQueue->LoopQueueBuf[pLoopQueue->InQueueIndex]), vLen - tail_data_len);
            pLoopQueue->InQueueIndex += (vLen - tail_data_len);
        }
        pLoopQueue->InQueueIndex &= LOOP_QUEUE_CAPABILITY;
    }
    else
    {
        /* Discard data. Actually IR maximum rx FIFO is 32, vLen <= 32. */
        uint8_t discard_data_len = 0;
        uint32_t discard_buf[32];
        while (vLen)
        {
            discard_data_len = (vLen > 32) ? 32 : vLen;
            IR_ReceiveBuf(discard_buf, discard_data_len);
            vLen -= discard_data_len;
        }
        IR_RX_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[ir_trans_rx]loop_queue_data_in: Discard bytes = %d!",
                         1, vLen);
    }
}

/**
  * @brief  Read data from loop queue.
  * @param  None.
  * @retval None.
  */
void ir_loop_queue_data_out(IR_LoopQueue_TypeDef *pLoopQueue, uint32_t *pBuf, uint16_t vLen)
{
    uint16_t remain_data_len = 0;
    uint16_t temp = 0;

    /* Check parameters */
    if ((vLen == 0) || (pBuf == NULL))
    {
        return;
    }

    if (pLoopQueue->InQueueIndex + vLen <= LOOP_QUEUE_BUF_SIZE_MAX)
    {
#ifdef LOOP_QUEUE_USE_MEMCPY_FUNCTION
        memcpy(pBuf, &(pLoopQueue->LoopQueueBuf[pLoopQueue->OutQueueIndex]), vLen * sizeof(uint32_t));
        pLoopQueue->OutQueueIndex += vLen;
#else
        while (vLen--)
        {
            *pBuf++ = pLoopQueue->LoopQueueBuf[pLoopQueue->OutQueueIndex++];
        }
#endif
    }
    else
    {
#ifdef LOOP_QUEUE_USE_MEMCPY_FUNCTION
        remain_data_len = LOOP_QUEUE_BUF_SIZE_MAX - pLoopQueue->OutQueueIndex;
        memcpy(pBuf, &(pLoopQueue->LoopQueueBuf[pLoopQueue->OutQueueIndex]),
               remain_data_len * sizeof(uint32_t));
        pLoopQueue->OutQueueIndex = 0;
        memcpy(pBuf + remain_data_len, &(pLoopQueue->LoopQueueBuf[pLoopQueue->OutQueueIndex]),
               (vLen - remain_data_len)*sizeof(uint32_t));
#else
        remain_data_len = LOOP_QUEUE_BUF_SIZE_MAX - pLoopQueue->OutQueueIndex;
        temp = remain_data_len;
        while (temp--)
        {
            *pBuf++ = pLoopQueue->LoopQueueBuf[pLoopQueue->OutQueueIndex++];
        }
        pLoopQueue->OutQueueIndex = 0;
        temp = vLen - remain_data_len;
        while (temp--)
        {
            *pBuf++ = pLoopQueue->LoopQueueBuf[pLoopQueue->OutQueueIndex++];
        }

#endif
    }
    temp += 0;
    pLoopQueue->OutQueueIndex &= LOOP_QUEUE_CAPABILITY;
}

/**
  * @brief  Copy data from loop queue.
  * @param  None.
  * @retval None.
  */
void ir_loop_queue_data_copy(IR_LoopQueue_TypeDef *pLoopQueue, uint32_t *pBuf, uint16_t vLen)
{
    uint16_t remain_data_len = 0;
    uint16_t index = 0;

    /* Check parameters */
    if ((vLen == 0) || (pBuf == NULL))
    {
        return;
    }

    if (pLoopQueue->OutQueueIndex + vLen <= LOOP_QUEUE_BUF_SIZE_MAX)
    {
        index = 0;
        while (vLen--)
        {
            *pBuf++ = pLoopQueue->LoopQueueBuf[pLoopQueue->OutQueueIndex + index];
            index++;
        }
    }
    else
    {
        remain_data_len = LOOP_QUEUE_BUF_SIZE_MAX - pLoopQueue->OutQueueIndex;
        uint16_t i = remain_data_len;
        index = 0;
        while (i--)
        {
            *pBuf++ = pLoopQueue->LoopQueueBuf[pLoopQueue->OutQueueIndex + index];
            index++;
        }

        i = vLen - remain_data_len;
        index = 0;
        while (i--)
        {
            *pBuf++ = pLoopQueue->LoopQueueBuf[index++];
        }
    }
}

#endif

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/
