/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      scatternet_app.c
   * @brief     This file handles BLE scatternet application routines.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <trace.h>
#include <string.h>
#include <gap.h>
#include <gap_adv.h>
#include <gap_bond_le.h>
#include <gap_msg.h>
#include <app_msg.h>
#include <gap_conn_le.h>
#include <os_timer.h>
#include "polling_app.h"
#include "rtl876x_gpio.h"
#include "rtl876x_uart.h"
#include "local_struct.h"
#include "board.h"
#include "mem_types.h"
#include "os_mem.h"
/*============================================================================*
 *                              Variables
 *============================================================================*/
uint8_t *pLoopBufContent = NULL;
LoopBuf_TypeDef RecLoopBuf;
uint8_t DemoStrBuffer[100];
/*============================================================================*
 *                              Functions
 *============================================================================*/
void app_handle_gap_msg(T_IO_MSG  *p_gap_msg);

/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg)
{
    uint16_t msg_type = io_msg.type;

    switch (msg_type)
    {
    case IO_MSG_TYPE_BT_STATUS:
        break;
    case IO_MSG_TYPE_GPIO:
        {
            HandleUARTEvent(io_msg);
        }
        break;
    default:
        break;
    }
}

/**
 * @brief    HandleButtonEvent for UART event
 * @note     Event for keyscan.
 * @param[in] io_msg
 * @return   void
 */
void HandleUARTEvent(T_IO_MSG io_msg)
{
    uint8_t keytype = io_msg.subtype ;

    if (keytype == MSG_UART)
    {
        HandleForLoopBuf(&RecLoopBuf);
    }
}
void HandleForLoopBuf(LoopBuf_TypeDef *pRecvLoopBuf)
{
    uint16_t currentloopbuflen = 0;
    currentloopbuflen = GetLoopBufLength(pRecvLoopBuf);
    if (PopLoopBuf(pRecvLoopBuf, currentloopbuflen, DemoStrBuffer) == false)
    {
        APP_PRINT_INFO1("Pop Loop Buffer error! line = %d", __LINE__);
    }
    UART_Send(DemoStrBuffer, currentloopbuflen);
}
/****************************************************************************/
/* UART interrupt                                                           */
/****************************************************************************/
void UART0_Handler(void)
{
    uint8_t RxByte[UART_RX_FIFO_TRIGGER_LEVEL_14BYTE];
    /* Read interrupt type */
    uint32_t int_status = UART_GetIID(UART);
    /* Disable interrupt */
    UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_LINE_STS, DISABLE);
    /* Interrupt handle */
    if (int_status == UART_INT_ID_RX_LEVEL_REACH)
    {
        UART_ReceiveData(UART, &RxByte[0], UART_RX_FIFO_TRIGGER_LEVEL_14BYTE);
        if (PushLoopBuf(&RecLoopBuf, UART_RX_FIFO_TRIGGER_LEVEL_14BYTE, RxByte) == false)
        {
            APP_PRINT_INFO0("push Loop Buffer error!");
        }
    }
    else if (int_status == UART_INT_ID_RX_TMEOUT)
    {
        while (UART_GetFlagState(UART, UART_FLAG_RX_DATA_RDY) == SET)
        {
            RxByte[0] = UART_ReceiveByte(UART);
            if (PushLoopBuf(&RecLoopBuf, 1, RxByte) == false)
            {
                APP_PRINT_INFO0("push Loop Buffer error!");
            }
        }
    }
    else if (int_status == UART_INT_ID_TX_EMPTY)
    {
        /* Tx FIFO empty, not enable */
        return;
    }
    else
    {
        /* Receive line status interrupt */
        if (int_status == UART_INT_ID_LINE_STATUS)
        {
            uint8_t line_status = UART->LSR;
            UART_GetFlagState(UART, UART_FLAG_RX_OVERRUN);
            UART_ClearRxFifo(UART);
            APP_PRINT_INFO1("line status error =0x%x!", line_status);
            /* Enable interrupt */
            UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_LINE_STS, ENABLE);
            return ;
        }
    }
    T_IO_MSG bee_io_msg;
    bee_io_msg.type = IO_MSG_TYPE_GPIO;
    bee_io_msg.subtype = MSG_UART;
    app_send_msg_to_apptask(&bee_io_msg);
    /* Enable interrupt */
    UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_LINE_STS, ENABLE);
}
void LOOP_BUFFER_Init()
{
    if (pLoopBufContent == NULL)
    {
        pLoopBufContent = os_mem_alloc(RAM_TYPE_DATA_ON,
                                       LOOP_BUFFER_MAX_SIZE); //TODO NEED vPortFree
    }
    else
    {
        os_mem_free(pLoopBufContent);
    }
    if (pLoopBufContent == NULL)
    {
        APP_PRINT_INFO0("lerror!");
    }
    InitLoopBuf(&RecLoopBuf, pLoopBufContent, LOOP_BUFFER_MAX_SIZE);
}
void UART_Send(uint8_t *Str, uint16_t strLen)
{
    uint8_t blkcount = 0;
    uint8_t remainder = 0;
    uint8_t i = 0;
    memcpy(DemoStrBuffer, Str, strLen);
    /* send demo tips */
    blkcount = strLen / UART_TX_FIFO_SIZE;
    remainder = strLen % UART_TX_FIFO_SIZE;

    /* send block bytes(16 bytes) */
    for (i = 0; i < blkcount; i++)
    {
        UART_SendData(UART, &DemoStrBuffer[UART_TX_FIFO_SIZE * i], UART_TX_FIFO_SIZE);
        /* wait tx fifo empty */
        while (UART_GetFlagState(UART, UART_FLAG_THR_TSR_EMPTY) != SET);
    }

    /* send left bytes */
    UART_SendData(UART, &DemoStrBuffer[UART_TX_FIFO_SIZE * i], remainder);
    /* wait tx fifo empty */
    while (UART_GetFlagState(UART, UART_FLAG_THR_TSR_EMPTY) != SET);

}
