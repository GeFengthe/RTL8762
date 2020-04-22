/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     ir_trans_rx.h
* @brief    data transmission layer driver
* @details
* @author   yuan
* @date     2019-01-23
* @version  v1.0
* *********************************************************************************************************
*/
#ifndef __IR_TRANS_RX_H
#define __IR_TRANS_RX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_driver_rx.h"
#include "ir_rx_loop_queue.h"


void ir_trans_rx_deinit(void);
void ir_trans_rx_init(void);
void ir_trans_rx_queue_init(void);
bool ir_trans_rx_queue_is_empty(void);
uint16_t ir_trans_rx_get_data_len(void);
void ir_trans_rx_read_data(uint32_t *pBuf, uint16_t vLen);
void ir_trans_rx_copy_data(uint32_t *pBuf, uint16_t vLen);
uint16_t ir_trans_rx_find_data_pos(uint32_t vData, uint16_t vLen);
void ir_trans_rx_int_config(FunctionalState state);
void ir_trans_rx_handler_cb(pFn_IR_RX_Handler_CB_t pFunc);

#ifdef __cplusplus
}
#endif

#endif /*__IR_TRANS_RX_H*/

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/

