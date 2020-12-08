/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      uart_transport.h
* @brief
* @details
* @author    chenjie jin
* @date      2018-04-08
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __UART_TRANSPORT_H
#define __UART_TRANSPORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include <stdint.h>
#include <stdbool.h>
#include "trace.h"

/* Defines ------------------------------------------------------------------*/

/* Enable print log or not */
#define UART_PRINT_LOG

/* Configure loop queue parameters */
#define UART_LOOP_QUEUE_MAX_SIZE             (512)
#define UART_QUEUE_CAPABILITY                (UART_LOOP_QUEUE_MAX_SIZE-1)
/* Configure UART packet buffer length */
#define CMD_SIZE                        50

#ifdef UART_PRINT_LOG
#define UART_DBG_BUFFER(MODULE, LEVEL, fmt, para_num,...) DBG_BUFFER_##LEVEL(TYPE_BEE2, SUBTYPE_FORMAT, MODULE, fmt, para_num, ##__VA_ARGS__)
#else
#define UART_DBG_BUFFER(MODULE, LEVEL, fmt, para_num,...) ((void)0)
#endif

/* Loop queue data struct */
typedef struct
{
    volatile uint16_t   ReadIndex;          /* index of read queue */
    volatile uint16_t   WriteIndex;         /* index of write queue */
    volatile bool       OverFlow;           /* loop queue overflow or not */
    uint8_t buf[UART_LOOP_QUEUE_MAX_SIZE];       /* Buffer for loop queue */
} UartLoopQueue_TypeDef;

/* Packet decode status */
typedef enum
{
    WaitHeader,
    WaitCMD,
    WaitParams,
    WaitCRC,
} WaitState;

/* UART packet data structure */
typedef struct _UART_PacketTypeDef
{
    uint8_t     Buf[CMD_SIZE];  /* command buffer */
    uint16_t    BufIndex;       /* index of buffer */
    uint16_t    PayloadLen;     /* length of decoder payload */
    uint16_t    CRCLen;         /* index of CRC */
    WaitState   Status;         /* status of decoding */

} UART_PacketTypeDef;

void DataUARTInit(uint8_t buadrate_opt);
void UartTransport_Init(void);
bool Packet_Decode(UART_PacketTypeDef *pPacket);
void UARTCmd_Response(uint16_t opcode, uint8_t status, uint8_t *pPayload, uint32_t payload_len);

#ifdef __cplusplus
}
#endif

#endif /*__UART_TRANSPORT_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

