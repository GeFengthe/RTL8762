/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      uart_trans.c
* @brief    This file provides uart transport layer driver for rcu voice test.
* @details
* @author  elliot_chen
* @date     2017-03-01
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "data_trans.h"
#include "mem_config.h"
#include "app_section.h"

#if (IR_FUN && IR_LEARN_MODE)
/* Defines -------------------------------------------------------------------*/

/* Globals ------------------------------------------------------------------ */
/* Loop queue management data structure */
LoopQueue_TypeDef   LoopQueue ;

/**<  Function pointer used to send event to application from IR learn interrupt handler. */
static pfn_IR_Learn_HandlerCB_t pfnIR_Learn_HandlerCB = NULL;

//extern bool RamVectorTableUpdate(VECTORn_Type v_num, IRQ_Fun isr_handler);
void IR_Learn_Handler(void);

/**
  * @brief  Initializes loop queue to their default reset values.
  * @param  pPacket: point to loop queue data structure.
  * @retval None
  */
void LoopQueue_Init(LoopQueue_TypeDef *pQueueStruct)
{
    memset(pQueueStruct, 0, sizeof(LoopQueue_TypeDef));
}

/**
  * @brief  check loop queue if will full or not.
  * @param  pQueueStruct: point to loop queue dta struct.
  * @retval The new state of loop queue (full:TRUE, not full:FALSE).
  */
BOOL LoopQueue_IsFull(LoopQueue_TypeDef *pQueueStruct, uint16_t write_size)
{
    /* Check queue status */
    if (pQueueStruct->WriteIndex >= pQueueStruct->ReadIndex)
    {
        if ((pQueueStruct->WriteIndex + write_size) >= (LOOP_QUEUE_MAX_SIZE + pQueueStruct->ReadIndex))
        {
            DATATRANS_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "Loop Queue is OverFlow!", 0);
            pQueueStruct->OverFlow = TRUE;
            return TRUE;
        }
    }
    else
    {
        if ((pQueueStruct->WriteIndex + write_size) >= pQueueStruct->ReadIndex)
        {
            DATATRANS_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "Loop Queue is OverFlow!", 0);
            pQueueStruct->OverFlow = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}

/**
  * @brief  Get valid data length.
  * @param None.
  * @retval None.
  */
uint16_t LoopQueue_GetDataLen(LoopQueue_TypeDef *pQueueStruct)
{
    return ((LOOP_QUEUE_MAX_SIZE + pQueueStruct->WriteIndex - pQueueStruct->ReadIndex) &
            QUEUE_CAPABILITY);
}

/**
  * @brief  check loop queue if empty or not.
  * @param  pPacket: point to IR packet struct.
  * @retval The new state of loop queue (empty:true, not empty:false).
  */
BOOL LoopQueueIsEmpty(LoopQueue_TypeDef *pQueueStruct)
{
    return (pQueueStruct->WriteIndex == pQueueStruct->ReadIndex);
}

/**
  * @brief  Write buffer data to loop queue.
  * @param None.
  * @retval None.
  */
void LoopQueue_WriteBuf(LoopQueue_TypeDef *pQueueStruct, uint16_t len)
{
    uint16_t tail_len = 0;
    uint16_t discard_len = 0;

    if (!LoopQueue_IsFull(pQueueStruct, len))
    {
        if (pQueueStruct->WriteIndex + len <= LOOP_QUEUE_MAX_SIZE)
        {
            IR_ReceiveBuf(&(pQueueStruct->buf[pQueueStruct->WriteIndex]), len);
            pQueueStruct->WriteIndex += len;
        }
        else
        {
            tail_len = LOOP_QUEUE_MAX_SIZE - pQueueStruct->WriteIndex;
            IR_ReceiveBuf(&(pQueueStruct->buf[pQueueStruct->WriteIndex]), tail_len);
            pQueueStruct->WriteIndex = 0;
            IR_ReceiveBuf(&(pQueueStruct->buf[pQueueStruct->WriteIndex]), len - tail_len);
            pQueueStruct->WriteIndex += (len - tail_len);
        }
        pQueueStruct->WriteIndex &= QUEUE_CAPABILITY;
    }
    else
    {
        /* Discard data, actually IR maximum rx FIFO is 32, len <= 32 */
        LoopQueue_BufType discard_buf[32];
        while (len)
        {
            discard_len = (len > 32) ? 32 : len;
            IR_ReceiveBuf(discard_buf, discard_len);
            len -= discard_len;
        }
        DATATRANS_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "Discard bytes: 0x%x!", 1, len);
    }
}

/**
  * @brief  Read buffer data from loop queue.
  * @param None.
  * @retval None.
  */
void LoopQueue_ReadBuf(LoopQueue_TypeDef *pQueueStruct, LoopQueue_BufType *pBuf,
                       uint16_t len)
{
    uint16_t remain = 0;
    uint16_t i = 0;

    /* Check parameters */
    if ((len == 0) || (pBuf == NULL))
    {
        return;
    }

    if (pQueueStruct->ReadIndex + len <= LOOP_QUEUE_MAX_SIZE)
    {
#ifdef USE_DATA_TRANS_MEMCPY
        memcpy(pBuf, &(pQueueStruct->buf[pQueueStruct->ReadIndex]), len * sizeof(LoopQueue_BufType));
        pQueueStruct->ReadIndex += len;
#else
        while (len--)
        {
            *pBuf++ = pQueueStruct->buf[pQueueStruct->ReadIndex++];
        }
#endif
    }
    else
    {
#ifdef USE_DATA_TRANS_MEMCPY
        remain = LOOP_QUEUE_MAX_SIZE - pQueueStruct->ReadIndex;
        memcpy(pBuf, &(pQueueStruct->buf[pQueueStruct->ReadIndex]), remain * sizeof(LoopQueue_BufType));
        pQueueStruct->ReadIndex = 0;
        memcpy(pBuf + remain, &(pQueueStruct->buf[pQueueStruct->ReadIndex]),
               (len - remain)*sizeof(LoopQueue_BufType));
#else
        remain = LOOP_QUEUE_MAX_SIZE - pQueueStruct->ReadIndex;
        i = remain;
        while (i--)
        {
            *pBuf++ = pQueueStruct->buf[pQueueStruct->ReadIndex++];
        }
        pQueueStruct->ReadIndex = 0;
        i = len - remain;
        while (i--)
        {
            *pBuf++ = pQueueStruct->buf[pQueueStruct->ReadIndex++];
        }

#endif
    }
    pQueueStruct->ReadIndex &= QUEUE_CAPABILITY;
}

/**
  * @brief  Read buffer data from loop queue.
  * @param None.
  * @retval None.
  */
void LoopQueue_CopyBuf(LoopQueue_TypeDef *pQueueStruct, LoopQueue_BufType *pBuf,
                       uint16_t len)
{
    uint16_t remain = 0;
    uint16_t i = 0;
    uint16_t idx = 0;

    /* Check parameters */
    if ((len == 0) || (pBuf == NULL))
    {
        return;
    }

    if (pQueueStruct->ReadIndex + len <= LOOP_QUEUE_MAX_SIZE)
    {
        idx = 0;
        while (len--)
        {
            *pBuf++ = pQueueStruct->buf[pQueueStruct->ReadIndex + idx];
            idx++;
        }

    }
    else
    {
        remain = LOOP_QUEUE_MAX_SIZE - pQueueStruct->ReadIndex;
        i = remain;
        idx = 0;
        while (i--)
        {
            *pBuf++ = pQueueStruct->buf[pQueueStruct->ReadIndex + idx];
            idx++;
        }

        i = len - remain;
        idx = 0;
        while (i--)
        {
            *pBuf++ = pQueueStruct->buf[idx++];
        }
    }
}

/**
  * @brief  Initializes IR peripheral.
  * @param  None.
  * @retval None.
  */
static void DataTrans_InitIR(void)
{
    /* Enable IR clock */
    RCC_PeriphClockCmd(APBPeriph_IR, APBPeriph_IR_CLOCK, DISABLE);
    RCC_PeriphClockCmd(APBPeriph_IR, APBPeriph_IR_CLOCK, ENABLE);

    /* Initialize IR */
    IR_InitTypeDef IR_InitStruct;
    IR_StructInit(&IR_InitStruct);
    /* IR carrier freqency is 38KHz */
    IR_InitStruct.IR_Freq               = IR_LEARN_FREQ;
    /* Duty ratio = 1/IR_DutyCycle */
    IR_InitStruct.IR_DutyCycle          = 2;
    /* IR receiveing mode */
    IR_InitStruct.IR_Mode               = IR_MODE_RX;
    IR_InitStruct.IR_RxStartMode        = IR_RX_AUTO_MODE;
    /* Configure RX FIFO threshold level to trigger IR_INT_RF_LEVEL interrupt */
    IR_InitStruct.IR_RxFIFOThrLevel     = IR_RX_FIFO_THR_LEVEL;
    /* Discard the lastest received dta if RX FIFO is full */
    IR_InitStruct.IR_RxFIFOFullCtrl     = IR_RX_FIFO_FULL_DISCARD_NEWEST;
    /* Configure trigger type */
    IR_InitStruct.IR_RxTriggerMode      = IR_RX_RISING_EDGE;
    /* If high to low or low to high transition time <= 50ns,Filter out it. */
    IR_InitStruct.IR_RxFilterTime       = IR_RX_FILTER_TIME_200ns;
    /* IR_RX_Count_Low_Level is counting low level */
    IR_InitStruct.IR_RxCntThrType       = IR_RX_Count_Low_Level;
    /* Configure RX counter threshold.You can use it to decide to stop receiving IR data */
    IR_InitStruct.IR_RxCntThr           = IR_LEARN_MAX_NO_WAVEFORM_TIME;
    IR_Init(&IR_InitStruct);
    IR_Cmd(IR_MODE_RX, ENABLE);
    IR_ClearRxFIFO();

    IR_INTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, ENABLE);
    IR_MaskINTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, DISABLE);

    /* Configure NVIC */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = IR_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Deinitializes data transmission layer.
  * @param  None.
  * @retval None
  */
void DataTrans_DeInit(void)
{
    /* Initialize loop queue */
    LoopQueue_Init(&LoopQueue);

    /* Close IR peripheral */
    IR_Cmd(IR_MODE_RX, DISABLE);

    /* Modify IR interrupt handle */
    //RamVectorTableUpdate(IR_VECTORn, IR_Handler);
}

/**
  * @brief  Initializes data transmission layer.
  * @param  None.
  * @retval None
  */
void DataTrans_Init(void)
{
    /* Initialize loop queue */
    LoopQueue_Init(&LoopQueue);

    /* Modify IR interrupt handle */
    RamVectorTableUpdate(IR_VECTORn, IR_Learn_Handler);

    /* Initialize IR peripheral */
    DataTrans_InitIR();
}

/**
  * @brief  Initializes loop queue.
  * @param  pPacket: point to UART packet structure.
  * @retval None
  */
void DataTrans_InitQueue(void)
{
    LoopQueue_Init(&LoopQueue);
}

/**
  * @brief  Check loop queue is empty or not.
  * @param None.
  * @retval status of loop queue.:
    TRUE: loop queue is empty.
    FALSE: loop queue is not empty.
  */
BOOL DataTrans_QueueIsEmpty(void)
{
    return LoopQueueIsEmpty(&LoopQueue);
}

/**
  * @brief  Get valid data length.
  * @param None.
  * @retval loop queue length.
  */
uint16_t DataTrans_GetFIFOLen(void)
{
    return LoopQueue_GetDataLen(&LoopQueue);
}

/**
  * @brief  Read data from loop queue.
  * @param  pBuf: point to address of buffer.
  * @param  len: buffer length.
  * @retval None.
  */
void DataTrans_ReadBuf(LoopQueue_BufType *pBuf, uint16_t len)
{
    LoopQueue_ReadBuf(&LoopQueue, pBuf, len);
}

/**
  * @brief  copy data from loop queue.
  * @param  pBuf: point to address of buffer.
  * @param  len: buffer length.
  * @retval None.
  */
void DataTrans_CopyBuf(LoopQueue_BufType *pBuf, uint16_t len)
{
    LoopQueue_CopyBuf(&LoopQueue, pBuf, len);
}

/**
  * @brief  Find special data in loop queue.
  * @param data: Find data.
  * @param len: data length in Loop queue.
  * @retval actual data position in loop queue.
  */
uint16_t DataTrans_FindDataPos(LoopQueue_BufType data, uint16_t len)
{
    uint16_t pos = LoopQueue.ReadIndex;

    while (len--)
    {
        pos &= QUEUE_CAPABILITY;

        if (data == LoopQueue.buf[pos++])
        {
            return (pos + LOOP_QUEUE_MAX_SIZE - LoopQueue.ReadIndex) & QUEUE_CAPABILITY;
        }
    }

    return 0;
}

/**
  * @brief  Enables or disables the specified interrupt.
  * @param  NewState: new state of the specified interrupt.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None.
  */
void DataTrans_INTConfig(FunctionalState state)
{
    if (state != DISABLE)
    {
        /* Enable interrupt */
        IR_INTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, ENABLE);
    }
    else
    {
        /* Disable interrupt */
        IR_INTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, DISABLE);
    }
}

/**
 * @brief Register callback function to send events from IR learn interrupt handle to application layer.
 * @param pFunc callback function.
 * @return none
*/
void DataTrans_RegisterIRLearnHandlerCB(pfn_IR_Learn_HandlerCB_t pFunc)
{
    pfnIR_Learn_HandlerCB = pFunc;
}

/**
* @brief IR interrupt handler function.
* @param   No parameter.
* @return  void
*/
void IR_Learn_Handler(void)
{
    uint16_t len = 0;
    uint8_t ir_learn_end_flag = FALSE;
    //long xHigherPriorityTaskWoken = pdFALSE;

    /* Mask IR all interrupt */
    IR_MaskINTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, ENABLE);

    /* Received interrupt */
    if (IR_GetINTStatus(IR_INT_RF_LEVEL) == SET)
    {
        len = IR_GetRxDataLen();
        LoopQueue_WriteBuf(&LoopQueue, len);
        IR_ClearINTPendingBit(IR_INT_RF_LEVEL_CLR);
    }

    /* Stop to receive IR data */
    if (IR_GetINTStatus(IR_INT_RX_CNT_THR) == SET)
    {
        /* Read remaining data */
        len = IR_GetRxDataLen();
        LoopQueue_WriteBuf(&LoopQueue, len);
        IR_ClearINTPendingBit(IR_INT_RX_CNT_THR_CLR);

        /* Send IR learn end signal */
        //Add application code here
        ir_learn_end_flag = TRUE;
    }

    if (pfnIR_Learn_HandlerCB)
    {
        pfnIR_Learn_HandlerCB(ir_learn_end_flag);
    }

    /* Unmask IR all interrupt */
    IR_MaskINTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, DISABLE);

    //portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

#endif /*end Micro @IR_FUN && IR_LEARN_MODE*/
/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

