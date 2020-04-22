/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     ir_trans_rx.c
* @brief
* @details
* @author   yuan
* @date     2019-01-23
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "ir_trans_rx.h"

#if (IR_FUN_EN && IR_FUN_LEARN_EN)

/**
  * @brief  Deinitializes data transmission layer.
  * @param  None.
  * @retval None
  */
void ir_trans_rx_deinit(void)
{
    /* Initialize loop queue */
    ir_loop_queue_init(&IR_RX_Queue);

    /* Close IR peripheral */
    IR_Cmd(IR_MODE_RX, DISABLE);

    /* Modify IR interrupt handle */
    RamVectorTableUpdate(IR_VECTORn, IR_Handler);
}

/**
  * @brief  Initializes data transmission layer.
  * @param  None.
  * @retval None
  */
void ir_trans_rx_init(void)
{
    /* Initialize loop queue */
    ir_loop_queue_init(&IR_RX_Queue);

    /* Modify IR interrupt handle */
    RamVectorTableUpdate(IR_VECTORn, IR_RX_Handler);

    /* Initialize IR peripheral */
    driver_ir_rx_init();
}

/**
  * @brief  Initializes loop queue.
  * @param  None
  * @retval None
  */
void ir_trans_rx_queue_init(void)
{
    ir_loop_queue_init(&IR_RX_Queue);
}

/**
  * @brief  Check loop queue is empty or not.
  * @param  None.
  * @retval status of loop queue.:
            TRUE: loop queue is empty.
            FALSE: loop queue is not empty.
  */
bool ir_trans_rx_queue_is_empty(void)
{
    return ir_loop_queue_is_empty(&IR_RX_Queue);
}

/**
  * @brief  Get valid data length.
  * @param  None.
  * @retval loop queue length
  */
uint16_t ir_trans_rx_get_data_len(void)
{
    return ir_loop_queue_get_data_len(&IR_RX_Queue);
}

/**
  * @brief  Read data from loop queue.
  * @param  pBuf: point to address of buffer.
  * @param  vLen: data length.
  * @retval None.
  */
void ir_trans_rx_read_data(uint32_t *pBuf, uint16_t vLen)
{
    ir_loop_queue_data_out(&IR_RX_Queue, pBuf, vLen);
}

/**
  * @brief  copy data from loop queue.
  * @param  pBuf: point to address of buffer.
  * @param  vLen: data length.
  * @retval None.
  */
void ir_trans_rx_copy_data(uint32_t *pBuf, uint16_t vLen)
{
    ir_loop_queue_data_copy(&IR_RX_Queue, pBuf, vLen);
}

/**
  * @brief  Find special data in loop queue.
  * @param  vData: Find data.
  * @param  vLen: data length in loop queue.
  * @retval actual data position in loop queue.
  */
uint16_t ir_trans_rx_find_data_pos(uint32_t vData, uint16_t vLen)
{
    uint16_t pos = IR_RX_Queue.OutQueueIndex;

    while (vLen--)
    {
        pos &= LOOP_QUEUE_CAPABILITY;

        if (vData == IR_RX_Queue.LoopQueueBuf[pos++])
        {
            return (pos + LOOP_QUEUE_BUF_SIZE_MAX - IR_RX_Queue.OutQueueIndex) & LOOP_QUEUE_CAPABILITY;
        }
    }

    return 0;
}

/**
  * @brief  Enables or disables the specified interrupt.
  * @param  NewState: new state of the specified interrupt.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None.
  */
void ir_trans_rx_int_config(FunctionalState state)
{
    int_ir_rx_config(state);
}

/**
  * @brief  Register callback function to send events from IR learn interrupt handle to application layer.
  * @param  pFunc: callback function.
  * @return void
*/
void ir_trans_rx_handler_cb(pFn_IR_RX_Handler_CB_t pFunc)
{
    ir_rx_handler_cb(pFunc);
}

#endif

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/
