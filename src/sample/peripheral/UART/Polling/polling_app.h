/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      scatternet_app.h
   * @brief     This file handles BLE scatternet application routines.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

#ifndef _KEYSCAN_APP_H_
#define _KEYSCAN_APP_H_

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <gap.h>
#include <gap_le.h>
#include <gap_msg.h>
#include <app_msg.h>
#include "local_struct.h"
/*============================================================================*
 *                              Variables
 *============================================================================*/
#define   MSG_UART  0
#define   LOOP_BUFFER_MAX_SIZE            (512)
/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg);
void LOOP_BUFFER_Init(void);
void HandleForLoopBuf(LoopBuf_TypeDef *pRecvLoopBuf);
void HandleUARTEvent(T_IO_MSG io_msg);
void UART_Send(uint8_t *Str, uint16_t strLen);
bool app_send_msg_to_apptask(T_IO_MSG *p_msg);
#ifdef __cplusplus
}
#endif

#endif
