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
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#define UART_TX_PIN                P4_0
#define UART_RX_PIN                P4_1


/** @brief  UART_BaudRate_Table
  *         div ovsr ovsr_adj :These three parameters set the baud rate calibration parameters of UART.
    baudrate    |   div     |   ovsr    |   ovsr_adj
--------------------------------------------------------
    1200Hz      |   2589    |   7       |   0x7F7
    9600Hz      |   271     |   10      |   0x24A
    14400Hz     |   271     |   5       |   0x222
    19200Hz     |   165     |   7       |   0x5AD
    28800Hz     |   110     |   7       |   0x5AD
    38400Hz     |   85      |   7       |   0x222
    57600Hz     |   55      |   7       |   0x5AD
    76800Hz     |   35      |   9       |   0x7EF
    115200Hz    |   20      |   12      |   0x252
    128000Hz    |   25      |   7       |   0x555
    153600Hz    |   15      |   12      |   0x252
    230400Hz    |   10      |   12      |   0x252
    460800Hz    |   5       |   12      |   0x252
    500000Hz    |   8       |   5       |   0
    921600Hz    |   4       |   5       |   0x3F7
    1000000Hz   |   4       |   5       |   0
    1382400Hz   |   2       |   9       |   0x2AA
    1444400Hz   |   2       |   8       |   0x5F7
    1500000Hz   |   2       |   8       |   0x492
    1843200Hz   |   2       |   5       |   0x3F7
    2000000Hz   |   2       |   5       |   0
    2100000Hz   |   1       |   14      |   0x400
    2764800Hz   |   1       |   9       |   0x2AA
    3000000Hz   |   1       |   8       |   0x492
    3250000Hz   |   1       |   7       |   0x112
    3692300Hz   |   1       |   5       |   0x5F7
    3750000Hz   |   1       |   5       |   0x36D
    4000000Hz   |   1       |   5       |   0
    6000000Hz   |   1       |   1       |   0x36D
-----------------------------------------------------
*/ /* End of UART_BaudRate_Table */

/* Globals ------------------------------------------------------------------*/
typedef struct
{
    uint16_t div;
    uint16_t ovsr;
    uint16_t ovsr_adj;
} UART_BaudRate_TypeDef;


const UART_BaudRate_TypeDef BaudRate_Table[10] =
{
    {271, 10, 0x24A}, // BAUD_RATE_9600
    {165, 7,  0x5AD}, // BAUD_RATE_19200
    {20,  12, 0x252}, // BAUD_RATE_115200
    {10,  12, 0x252}, // BAUD_RATE_230400
    {5,   12, 0x252}, // BAUD_RATE_460800
    {4,   5,  0x3F7}, // BAUD_RATE_921600
    {2,   5,  0},     // BAUD_RATE_2000000
    {1,   8,  0x492}, // BAUD_RATE_3000000
    {1,   5,  0},     // BAUD_RATE_4000000
    {1,   1,  0x36D}, // BAUD_RATE_6000000
};


uint8_t String_Buf[100];

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
    UART_InitStruct.rxTriggerLevel = 16;                      //1~29
    UART_InitStruct.idle_time      = UART_RX_IDLE_2BYTE;      //idle interrupt wait time

    UART_Init(UART, &UART_InitStruct);
}

/**
  * @brief  UARt send data continuous.
  * @param  No parameter.
  * @return void
*/
void uart_senddata_continuous(UART_TypeDef *UARTx, const uint8_t *pSend_Buf, uint16_t vCount)
{
    uint8_t count;

    while (vCount / UART_TX_FIFO_SIZE > 0)
    {
        while (UART_GetFlagState(UARTx, UART_FLAG_THR_EMPTY) == 0);
        for (count = UART_TX_FIFO_SIZE; count > 0; count--)
        {
            UARTx->RB_THR = *pSend_Buf++;
        }
        vCount -= UART_TX_FIFO_SIZE;
    }

    while (UART_GetFlagState(UARTx, UART_FLAG_THR_EMPTY) == 0);
    while (vCount--)
    {
        UARTx->RB_THR = *pSend_Buf++;
    }
}

/**
  * @brief  Demo code of uart.
  * @param  No parameter.
  * @return void
*/
void uart_demo(void)
{
    uint16_t demo_str_len = 0;
    uint8_t rx_byte = 0;

    board_uart_init();
    driver_uart_init();

    char *demo_str = "### Uart demo polling read uart data ###\r\n";
    demo_str_len = strlen(demo_str);
    memcpy(String_Buf, demo_str, demo_str_len);

    /* Send demo tips */
    uart_senddata_continuous(UART, String_Buf, demo_str_len);

    /* Loop rx and tx */
    while (1)
    {
        if (UART_GetFlagState(UART, UART_FLAG_RX_DATA_RDY) == SET)
        {
            rx_byte = UART_ReceiveByte(UART);
            UART_SendByte(UART, rx_byte);
        }
    }
}

/**
  * @brief    Entry of app code
  * @return   int (To avoid compile warning)
  */
int main(void)
{
    uart_demo();

    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();

    }
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
