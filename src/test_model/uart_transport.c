/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     uart_transport.c
* @brief    This file provides uart transport layer driver for rcu uart test.
* @details
* @author   chenjie_jin
* @date     2018-04-08
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <board.h>
#include <uart_transport.h>
#include <data_uart_test.h>
#include <data_uart_test.h>
#include <rtl876x_uart.h>
#include <rtl876x_pinmux.h>
#include <rtl876x_rcc.h>
#include <rtl876x_nvic.h>
#include <string.h>
#include <app_task.h>

#if MP_TEST_MODE_SUPPORT_DATA_UART_TEST

#define MP_TEST_UART_TX_PIN DATA_UART_TX_PIN  
#define MP_TEST_UART_RX_PIN DATA_UART_RX_PIN 
/* Defines -------------------------------------------------------------------*/


/* Protocol defines ------------------------------------------------------------*/
/* Protocol header define */
#define PACKET_HEAD                     ((uint8_t)0x87)
#define PACKET_HEAD_LEN                 ((uint8_t)0x01)
#define OPCODE_LEN                      ((uint8_t)0x02)
#define CRC_DATA_LEN                    ((uint16_t)0x02)
/* Configure data uart receive trigger level */
#define UART_RX_TRIGGER_LEVEL           UART_RX_FIFO_TRIGGER_LEVEL_4BYTE
#define UART_RX_TRIGGER_VALUE           4

/* Globals ------------------------------------------------------------------ */

/* Loop queue management data structure */
UartLoopQueue_TypeDef   UartLoopQueue;
/* UART packet data structure */
UART_PacketTypeDef UART_Packet;

/* For CRC check */
#define  FAR
typedef unsigned char    UCHAR, BYTE, * PUCHAR, * PBYTE;
typedef unsigned short   WORD, USHORT, * PUSHORT, * PWORD;
typedef unsigned char    FAR *LPBYTE;

/* CRC check function */
extern uint16_t btxfcs(WORD fcs, BYTE FAR *cp, WORD len);

void DataUartTestIntrHandler(void);

/**
  * @brief  Initializes loop queue to their default reset values.
  * @param  pPacket: point to loop queue data structure.
  * @retval None
  */
static void LoopQueueInit(UartLoopQueue_TypeDef *pQueueStruct)
{
    uint16_t i = 0;

    /* Initialize loop queue data structure */
    pQueueStruct->ReadIndex  = 0;
    pQueueStruct->WriteIndex = 0;
    pQueueStruct->OverFlow   = false;

    /* Initialize Queue buffer */
    for (i = 0; i <= UART_QUEUE_CAPABILITY; i++)
    {
        pQueueStruct->buf[i] = 0;
    }
}

/**
  * @brief  check loop queue if full or not.
  * @param  pQueueStruct: point to loop queue dta struct.
  * @retval The new state of loop queue (full:TRUE, not full:FALSE).
  */
static inline bool LoopQueueIsFull(UartLoopQueue_TypeDef *pQueueStruct, uint16_t write_size)
{
    if (((pQueueStruct->WriteIndex + write_size) & UART_QUEUE_CAPABILITY) == pQueueStruct->ReadIndex)
    {
        UART_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "buf is OverFlow!", 0);
        pQueueStruct->OverFlow = true;
        return true;
    }
    return false;
}

#if 0
/**
  * @brief  check loop queue if empty or not.
  * @param  pPacket: point to IR packet struct.
  * @retval The new state of loop queue (empty:true, not empty:false).
  */
static inline bool LoopQueueIsEmpty(UartLoopQueue_TypeDef *pQueueStruct)
{
    return (pQueueStruct->WriteIndex == pQueueStruct->ReadIndex);
}
#endif

/**
  * @brief  Initializes UART packet data structure.
  * @param  pPacket: point to UART packet structure.
  * @retval None
  */
static void UARTPacketStructInit(UART_PacketTypeDef *pPacket)
{
    /* Initialize UART packet data structure */
    pPacket->BufIndex   = 0;
    pPacket->CRCLen     = 0;
    pPacket->PayloadLen = 0;
    pPacket->Status     = WaitHeader;
}

/**
  * @brief  Initializes UART peripheral.
  * @param  None.
  * @retval None.
  */
void DataUARTInit(uint8_t buadrate_opt)
{
    /* pinmux configuration */
    UART_DeInit(UART);
    Pinmux_Config(MP_TEST_UART_TX_PIN, UART0_TX);
    Pinmux_Config(MP_TEST_UART_RX_PIN, UART0_RX);


    /* pad configuration */
    Pad_Config(MP_TEST_UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(MP_TEST_UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    /* turn on UART clock */
    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);

    /* uart init */
    UART_InitTypeDef uartInitStruct;
    UART_StructInit(&uartInitStruct);
    uartInitStruct.rxTriggerLevel = UART_RX_TRIGGER_LEVEL;

    if (buadrate_opt == CHANGE_BAUDRATE_OPTION_2M)
    {
        /* set baudrate to 2M */
        uartInitStruct.div = 2;
        uartInitStruct.ovsr = 5;
        uartInitStruct.ovsr_adj = 0;
        uartInitStruct.parity = UART_PARITY_NO_PARTY;
    }
    else
    {
        /* set baudrate to 115200 */
        uartInitStruct.div = 20;
        uartInitStruct.ovsr = 12;
        uartInitStruct.ovsr_adj = 0x252;
        uartInitStruct.parity = UART_PARITY_NO_PARTY;
    }

    UART_Init(UART, &uartInitStruct);

    /* NVIC init */
    UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_LINE_STS,  ENABLE);
    NVIC_ClearPendingIRQ(UART0_IRQn);

    /*  Enable UART IRQ  */
    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = UART0_IRQn;
    nvic_init_struct.NVIC_IRQChannelCmd      = ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 5;
    NVIC_Init(&nvic_init_struct);
}

/**
  * @brief  Initializes loop queue and UART packet data structure to their default reset values.
  * @param  pPacket: point to UART packet structure.
  * @retval None
  */
void UartTransport_Init(void)
{
    /* Initialize loop queue */
    LoopQueueInit(&UartLoopQueue);

    /* Initialize UART packet data structure */
    UARTPacketStructInit(&UART_Packet);

    /* Update Data UART interrupt handle */
    RamVectorTableUpdate(Uart0_VECTORn, DataUartTestIntrHandler);

    /* Initialize Data UART peripheral */
    DataUARTInit(CHANGE_BAUDRATE_OPTION_115200);
}

/**
  * @brief  decode uart packet.
  * @param  pPacket: point to UART packet struct.
  * @retval The decoded status of a entire packet.
  */
bool UARTPacket_Decode(UartLoopQueue_TypeDef *pQueueStruct, UART_PacketTypeDef *pPacket)
{
    bool PaketComplete = false;
    uint16_t opcode = 0;
    uint8_t param_len = 0;

    UART_DBG_BUFFER(MODULE_UART, LEVEL_INFO, "[UARTPacket_Decode] decode uart packet", 0);
    UART_DBG_BUFFER(MODULE_UART, LEVEL_INFO, "[UARTPacket_Decode] %d %d", 2, pQueueStruct->ReadIndex,
                    pQueueStruct->WriteIndex);

    while (pQueueStruct->ReadIndex != pQueueStruct->WriteIndex)
    {
        switch (pPacket->Status)
        {
        case WaitHeader:
            {
                /* Header: one byte */
                if (pQueueStruct->buf[pQueueStruct->ReadIndex] == PACKET_HEAD)
                {
                    pPacket->Buf[pPacket->BufIndex++] = pQueueStruct->buf[pQueueStruct->ReadIndex++];

                    pQueueStruct->ReadIndex &= UART_QUEUE_CAPABILITY;
                    pPacket->Status = WaitCMD;
                }
                else
                {
                    pQueueStruct->ReadIndex++;
                    UART_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "Error header = 0x%x!", 1,
                                    pQueueStruct->buf[pQueueStruct->ReadIndex]);
                    break;
                }
            }
        case WaitCMD:
            {
                while (pQueueStruct->ReadIndex != pQueueStruct->WriteIndex)
                {
                    /* Obtain command field data */
                    pPacket->Buf[pPacket->BufIndex++] = pQueueStruct->buf[pQueueStruct->ReadIndex++];
                    pQueueStruct->ReadIndex &= UART_QUEUE_CAPABILITY;

                    /* CMD: two bytes */
                    if (pPacket->BufIndex == (PACKET_HEAD_LEN + OPCODE_LEN))
                    {
                        opcode = (pPacket->Buf[pPacket->BufIndex - 1] << 8) + pPacket->Buf[pPacket->BufIndex - 2];

                        if ((opcode >= UART_FN_BEGIN) && (opcode < UART_FN_END))
                        {
                            if (uart_test_func_map[opcode & UART_FN_MASK].fncb != NULL)
                            {
                                param_len = uart_test_func_map[opcode & UART_FN_MASK].param_len;
                            }
                            else
                            {
                                param_len = 0;
                            }
                        }
                        else
                        {
                            param_len = 0;
                        }
                        pPacket->Status = WaitParams;
                        break;
                    }
                }
            }

        case WaitParams:
            {
                // Add application code here if have parameter
                while (param_len)
                {
                    /* Obtain parameter field data */
                    pPacket->Buf[pPacket->BufIndex++] = pQueueStruct->buf[pQueueStruct->ReadIndex++];
                    pQueueStruct->ReadIndex &= UART_QUEUE_CAPABILITY;
                    param_len--;
                }
                pPacket->Status = WaitCRC;
                break;
            }

        case WaitCRC:
            {
                while (pQueueStruct->ReadIndex != pQueueStruct->WriteIndex)
                {
                    pPacket->Buf[pPacket->BufIndex++] = pQueueStruct->buf[pQueueStruct->ReadIndex++];
                    pPacket->CRCLen++;

                    if (pPacket->CRCLen >= CRC_DATA_LEN)
                    {
                        break;
                    }
                }

                /* CRC check */
                if (btxfcs(0, pPacket->Buf, (pPacket->BufIndex) - CRC_DATA_LEN) == ((pPacket->Buf[pPacket->BufIndex
                                                                                     - 1] << 8) + pPacket->Buf[pPacket->BufIndex - 2]))
                {
                    PaketComplete = true;
                }
                else
                {
                    // CRC check error
                    UART_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "CRC check Error!", 0);
                }

                /* Reset decoding status */
                UARTPacketStructInit(pPacket);
                LoopQueueInit(&UartLoopQueue);
                return PaketComplete;
            }
        default:
            {
                break;
            }
        }
    }

    LoopQueueInit(&UartLoopQueue);
    return PaketComplete;
}

/**
  * @brief  Response of uart command.
  * @param cmd_index: command index.
  * @retval None.
  */
void UARTCmd_Response(uint16_t opcode, uint8_t status, uint8_t *pPayload, uint32_t payload_len)
{
    UART_PacketTypeDef RespPacket;
    UART_PacketTypeDef *pRespPacket = &RespPacket;

    pRespPacket->BufIndex = 0;
    pRespPacket->Buf[pRespPacket->BufIndex++] = PACKET_HEAD;
    pRespPacket->Buf[pRespPacket->BufIndex++] = opcode & 0xff;
    pRespPacket->Buf[pRespPacket->BufIndex++] = opcode >> 8;
    pRespPacket->Buf[pRespPacket->BufIndex++] = status;
    pRespPacket->Buf[pRespPacket->BufIndex++] = payload_len & 0xff;
    pRespPacket->Buf[pRespPacket->BufIndex++] = payload_len >> 8;
    pRespPacket->Buf[pRespPacket->BufIndex++] = payload_len >> 16;
    pRespPacket->Buf[pRespPacket->BufIndex++] = payload_len >> 24;

    while (payload_len--)
    {
        pRespPacket->Buf[pRespPacket->BufIndex++] = *pPayload++;
    }

    /* Add CRC data */
    uint16_t crc_calc = btxfcs(0, pRespPacket->Buf, pRespPacket->BufIndex);
    pRespPacket->Buf[pRespPacket->BufIndex++] = (uint8_t)(crc_calc & 0xff);
    pRespPacket->Buf[pRespPacket->BufIndex++] = (uint8_t)(crc_calc >> 8);

    /* send block bytes(16 bytes) */
    uint32_t i = 0;
    for (i = 0; i < (pRespPacket->BufIndex / UART_TX_FIFO_SIZE); i++)
    {
        UART_SendData(UART, &(pRespPacket->Buf[UART_TX_FIFO_SIZE * i]), UART_TX_FIFO_SIZE);
        /* wait tx fifo empty */
        while (UART_GetFlagState(UART, UART_FLAG_THR_TSR_EMPTY) != SET);
    }

    /* send left bytes */
    UART_SendData(UART, &(pRespPacket->Buf[UART_TX_FIFO_SIZE * i]),
                  pRespPacket->BufIndex % UART_TX_FIFO_SIZE);
    /* wait tx fifo empty */
    while (UART_GetFlagState(UART, UART_FLAG_THR_TSR_EMPTY) != SET);
}

/**
  * @brief  decode uart packet.
  * @param  pPacket: point to UART packet struct.
  * @retval The decoded status of a entire packet.
  */
bool Packet_Decode(UART_PacketTypeDef *pPacket)
{
    return UARTPacket_Decode(&UartLoopQueue, pPacket);
}

/**
  * @brief  Data UART interrupt handle.
  * @param  None.
  * @retval None.
  */
void DataUartTestIntrHandler(void)
{
    uint16_t len = 0;

    /* read interrupt id */
    uint32_t int_status = UART_GetIID(UART);

    UART_DBG_BUFFER(MODULE_UART, LEVEL_INFO, "[DataUartTestIntrHandler] int_status = %d", 1,
                    int_status);

    /* disable interrupt */
    UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_LINE_STS, DISABLE);

    switch (int_status)
    {
    /* tx fifo empty, not enable */
    case UART_INT_ID_TX_EMPTY:
        {
            /* do nothing */
            break;
        }

    /* rx data valiable */
    case UART_INT_ID_RX_LEVEL_REACH:
        {
            if (!LoopQueueIsFull(&UartLoopQueue, UART_RX_TRIGGER_VALUE))
            {
                UartLoopQueue.WriteIndex &= UART_QUEUE_CAPABILITY;

                if (UartLoopQueue.WriteIndex + UART_RX_TRIGGER_VALUE <= UART_LOOP_QUEUE_MAX_SIZE)
                {
                    UART_ReceiveData(UART, &UartLoopQueue.buf[UartLoopQueue.WriteIndex], UART_RX_TRIGGER_VALUE);
                    UartLoopQueue.WriteIndex = (UartLoopQueue.WriteIndex + UART_RX_TRIGGER_VALUE) &
                                               UART_QUEUE_CAPABILITY;
                }
                else
                {
                    len = UART_LOOP_QUEUE_MAX_SIZE - UartLoopQueue.WriteIndex;
                    UART_ReceiveData(UART, &UartLoopQueue.buf[UartLoopQueue.WriteIndex], len);

                    UartLoopQueue.WriteIndex = 0;
                    UART_ReceiveData(UART, &UartLoopQueue.buf[UartLoopQueue.WriteIndex], UART_RX_TRIGGER_VALUE - len);
                    UartLoopQueue.WriteIndex += UART_RX_TRIGGER_VALUE - len;
                }
            }
            else
            {
                /* Discard data */
                uint8_t Data[14];
                UART_ReceiveData(UART, (uint8_t *)(&Data), UART_RX_TRIGGER_VALUE);
            }
            /* enable interrupt again */
            UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_LINE_STS, ENABLE);
            return;
        }

    /* rx time out */
    case UART_INT_ID_RX_TMEOUT:
        {
            while (UART_GetFlagState(UART, UART_FLAG_RX_DATA_RDY) == SET)
            {

                if (!LoopQueueIsFull(&UartLoopQueue, 1))
                {
                    UartLoopQueue.WriteIndex &= UART_QUEUE_CAPABILITY;
                    UART_ReceiveData(UART, &UartLoopQueue.buf[UartLoopQueue.WriteIndex++], 1);
                }
                else
                {
                    /* Discard data */
                    UART->RB_THR;
                }
            }
            break;
        }

    /* receive line status interrupt */
    case UART_INT_ID_LINE_STATUS:
        {
            UART_GetFlagState(UART, UART_FLAG_RX_OVERRUN);
            UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "line status error!", 0);
            break;
        }

    default:
        break;
    }
    /* Send Msg to App task */
    T_IO_MSG uart_test_msg;
    uart_test_msg.type  = IO_MSG_TYPE_UART;
    uart_test_msg.subtype = IO_MSG_UART_RX;
    uart_test_msg.u.buf    = (void *)(&UART_Packet);

    app_send_msg_to_apptask(&uart_test_msg);

    /* enable interrupt again */
    UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_LINE_STS, ENABLE);
}
#endif

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

