/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    uart demo polling tx and rx.
* @details
* @author   yuan
* @date     2018-06-28
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_uart.h"
#include "rtl876x_gdma.h"
#include "trace.h"
#include "app_msg.h"

/* Defines ------------------------------------------------------------------*/
#define UART_TX_PIN                P3_0
#define UART_RX_PIN                P3_1
/* Set 1 if Allow GDMA TX*/
#define NOT_ALLOW_DEINIT        1

#define DMA_RX_CHANNEL_NUM    4
#define DMA_RX_CHANNEL              GDMA_Channel4
#define DMA_RX_IRQ                      GDMA0_Channel4_IRQn

#define DMA_TX_CHANNEL_NUM      3
#define DMA_TX_CHANNEL              GDMA_Channel3
#define DMA_TX_IRQ                      GDMA0_Channel3_IRQn

#define GDMA_TRANSFER_SIZE              512
#define GDMA_BLOCK_SIZE                                 216
uint8_t GDMA_Rx_Buf[GDMA_TRANSFER_SIZE];
uint8_t GDMA_Tx_Buf[GDMA_TRANSFER_SIZE];

uint8_t receive_offset = 0;
uint8_t count = 0;

/* Globals ------------------------------------------------------------------*/
typedef struct
{
    uint16_t div;
    uint16_t ovsr;
    uint16_t ovsr_adj;
} UART_BaudRate_TypeDef;


const UART_BaudRate_TypeDef BaudRate_Table[11] =
{
    {271, 10, 0x24A}, // BAUD_RATE_9600
    {165, 7,  0x5AD}, // BAUD_RATE_19200
    {85,    7,  0x222}, // BAUD_RATE_38400
    {20,  12, 0x252}, // BAUD_RATE_115200
    {10,  12, 0x252}, // BAUD_RATE_230400
    {5,   12, 0x252}, // BAUD_RATE_460800
    {4,   5,  0x3F7}, // BAUD_RATE_921600
    {2,   5,  0},     // BAUD_RATE_2000000
    {1,   8,  0x492}, // BAUD_RATE_3000000
    {1,   5,  0},     // BAUD_RATE_4000000
    {1,   1,  0x36D}, // BAUD_RATE_6000000
};
bool receiveflg = false;

/****************************************************************************/
/* board init                                                               */
/****************************************************************************/
/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_uart_init(void)
{
    Pad_Config(UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);

    Pinmux_Config(UART_TX_PIN, UART0_TX);
    Pinmux_Config(UART_RX_PIN, UART0_RX);
}

/****************************************************************************/
/* UART driver init                                                         */
/****************************************************************************/
/**
  * @brief  Initialize uart peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_uart_init(void)
{
    UART_DeInit(UART);

    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);
    /* uart init */
    UART_InitTypeDef UART_InitStruct;
    UART_StructInit(&UART_InitStruct);

    /* Config uart baudrate */
    UART_InitStruct.div            = BaudRate_Table[BAUD_RATE_115200].div;
    UART_InitStruct.ovsr           = BaudRate_Table[BAUD_RATE_115200].ovsr;
    UART_InitStruct.ovsr_adj       = BaudRate_Table[BAUD_RATE_115200].ovsr_adj;

    UART_InitStruct.parity         = UART_PARITY_NO_PARTY;
    UART_InitStruct.stopBits       = UART_STOP_BITS_1;
    UART_InitStruct.wordLen        = UART_WROD_LENGTH_8BIT;
    UART_InitStruct.rxTriggerLevel = UART_RX_FIFO_TRIGGER_LEVEL_8BYTE;                      //1~29
    UART_InitStruct.idle_time      =
        UART_RX_IDLE_2BYTE;                                                                //idle interrupt wait time
    UART_InitStruct.TxWaterlevel   = 15;
    UART_InitStruct.RxWaterlevel   =
        1;                                                                                                     //Better to equal GDMA_MSize
    UART_InitStruct.TxDmaEn        = ENABLE;
    UART_InitStruct.RxDmaEn        = ENABLE;
    UART_InitStruct.dmaEn          = UART_DMA_ENABLE;
    UART_InitStruct.autoFlowCtrl   = UART_AUTO_FLOW_CTRL_EN;
    UART_Init(UART, &UART_InitStruct);
    UART_INTConfig(UART, UART_INT_IDLE | UART_INT_LINE_STS, ENABLE);
    UART_INTConfig(UART,  UART_INT_RD_AVA,  DISABLE);
    /*  Enable UART IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = UART0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 5;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    return;
}

/****************************************************************************/
/* GDMA_Channel3 driver init                                                */
/****************************************************************************/
void driver_gdma3_init(void)
{
    /* Initialize GDMA */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = DMA_TX_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = 0;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)GDMA_Tx_Buf;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(&(UART->RB_THR));
    GDMA_InitStruct.GDMA_DestHandshake           = GDMA_Handshake_UART0_TX;

    GDMA_Init(DMA_TX_CHANNEL, &GDMA_InitStruct);
    /* Enable transfer finish interrupt */
    GDMA_INTConfig(DMA_TX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    /* Configure NVIC of GDMA */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = DMA_TX_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 5;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

}

/****************************************************************************/
/* GDMA_Channel4 driver init                                                */
/****************************************************************************/
void driver_gdma4_init(void)
{
    /* Initialize GDMA */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = DMA_RX_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_PeripheralToMemory;
    GDMA_InitStruct.GDMA_BufferSize          = GDMA_BLOCK_SIZE;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)(&(UART->RB_THR));
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)GDMA_Rx_Buf;
    GDMA_InitStruct.GDMA_SourceHandshake     = GDMA_Handshake_UART0_RX;

    GDMA_Init(DMA_RX_CHANNEL, &GDMA_InitStruct);
    /* Enable transfer finish interrupt */
    GDMA_INTConfig(DMA_RX_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    /* Configure NVIC of GDMA */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GDMA0_Channel4_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    GDMA_Cmd(DMA_RX_CHANNEL_NUM, ENABLE);
}

/****************************************************************************/
/* UART Handler                                                             */
/****************************************************************************/
void UART0_Handler(void)
{
    uint8_t tmp;
    uint32_t int_status = UART_GetIID(UART);
    if (UART_GetFlagState(UART, UART_FLAG_RX_IDLE) == SET)
    {
        DBG_DIRECT("UART_FLAG_RX_IDLE");

        /*  Suspend GDMA_Channel4   */
        GDMA_SuspendCmd(DMA_RX_CHANNEL, ENABLE);
        UART_INTConfig(UART, UART_INT_IDLE, DISABLE);
        for (uint32_t i = 0; i < (DMA_RX_CHANNEL->DAR - (uint32_t)GDMA_Rx_Buf); i++)
        {
            DBG_DIRECT("value is 0x%x", GDMA_Rx_Buf[i]);
        }

        if (DMA_RX_CHANNEL->DAR - (uint32_t)GDMA_Rx_Buf)
        {
            receive_offset += DMA_RX_CHANNEL->DAR - (uint32_t)GDMA_Rx_Buf;
            memcpy(GDMA_Tx_Buf + GDMA_BLOCK_SIZE * count, GDMA_Rx_Buf,
                   DMA_RX_CHANNEL->DAR - (uint32_t)GDMA_Rx_Buf);

#if NOT_ALLOW_DEINIT
            uint32_t time_out = 0x1f;
            while ((RESET == GDMA_GetSuspendChannelStatus(DMA_RX_CHANNEL)) && time_out)
            {
                time_out--;
            }
            time_out = 0x0f;
            while ((RESET == GDMA_GetSuspendCmdStatus(DMA_RX_CHANNEL)) && time_out)
            {
                time_out--;
            }
            GDMA_Cmd(4, DISABLE);
            GDMA_SuspendCmd(DMA_RX_CHANNEL, DISABLE);
#else
            GDMA_DeInit();
#endif

            driver_gdma4_init();
            /* GDMA TX flag */
            receiveflg = true;
        }
        /* Run here if data length = N * GDMA_BLOCK_SIZE,  */
        else
        {
            GDMA_SuspendCmd(DMA_RX_CHANNEL, DISABLE);
            receiveflg = true;
        }

        switch (int_status & 0x0E)
        {
        case UART_INT_ID_LINE_STATUS:
            {
                DBG_DIRECT("Line status error!=%x", UART->LSR);
                break;
            }
        case UART_INT_ID_RX_LEVEL_REACH:
            {
                DBG_DIRECT("UART_INT_ID_RX_LEVEL_REACH");
                break;
            }
        case UART_INT_ID_RX_TMEOUT:
            {
                DBG_DIRECT("UART_INT_ID_RX_TMEOUT.");
                while (UART_GetFlagState(UART, UART_FLAG_RX_DATA_RDY) == SET)
                {
                    UART_ReceiveData(UART, &tmp, 1);
                }
                break;
            }
        default:
            {
                break;
            }
        }
        UART_INTConfig(UART, UART_INT_IDLE, ENABLE);
    }
}

/****************************************************************************/
/* GDMA Handler                                                             */
/****************************************************************************/
void GDMA0_Channel4_Handler(void)
{
    DBG_DIRECT("GDMA0_Channel4_Handler");
    /*  Clear interrupt */
    GDMA_Cmd(DMA_RX_CHANNEL_NUM, DISABLE);
    GDMA_ClearAllTypeINT(DMA_RX_CHANNEL_NUM);
    receive_offset += GDMA_BLOCK_SIZE;
    count += 1;

    /*print information*/
    for (uint32_t i = 0; i < GDMA_BLOCK_SIZE; i++)
    {
        DBG_DIRECT("Rxvalue is 0x%x", GDMA_Rx_Buf[i]);
    }
    memcpy(GDMA_Tx_Buf + GDMA_BLOCK_SIZE * (count - 1), GDMA_Rx_Buf, GDMA_BLOCK_SIZE);

    GDMA_ClearINTPendingBit(DMA_RX_CHANNEL_NUM, GDMA_INT_Transfer);
    /*  reset gdma param    */
    GDMA_SetDestinationAddress(DMA_RX_CHANNEL, (uint32_t)GDMA_Rx_Buf);
    GDMA_Cmd(DMA_RX_CHANNEL_NUM, ENABLE);

}

/****************************************************************************/
/* main                                                                     */
/****************************************************************************/
/**
  * @brief    Entry of app code
  * @return   int (To avoid compile warning)
  */
int main(void)
{
    __enable_irq();
    board_uart_init();
    driver_uart_init();
    /* GDMA Channel For Rx*/
    driver_gdma4_init();
    /* GDMA Channel For Tx*/
    driver_gdma3_init();
    while (1)
    {
        __nop();
        __nop();

        if (receiveflg)
        {
            /* Print information */
            DBG_DIRECT("receive_offset = %d", receive_offset);
            GDMA_SetBufferSize(DMA_TX_CHANNEL, receive_offset);
            GDMA_Cmd(DMA_TX_CHANNEL_NUM, ENABLE);
            receive_offset = 0;
            count = 0;
            receiveflg = false;
        }
        __nop();
        __nop();
    }
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
