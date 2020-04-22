/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      ir_send_app.h
* @brief
* @details
* @author    elliot chen
* @date      2018-07-23
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __IR_SEND_APP_H
#define __IR_SEND_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_trans.h"
#include "string.h"
#include "app_msg.h"
#include "trace.h"

void Board_LED_Init(void);
void Driver_LED_Init(void);
void IR_Send_Init(void);
void IR_Raw_Packet_Send(uint32_t *pBuf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /*__IR_SEND_APP_H*/


/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

