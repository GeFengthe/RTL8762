/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     uart_interrupt_demo.c
* @brief    uart demo interrupt
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

/* Globals ------------------------------------------------------------------*/
uint8_t String_Buf[100];
uint8_t UART_Rev_Data[256];
uint8_t UART_Rev_Data_End_Flag = 0;
uint16_t UART_Rev_Data_Lenth = 0;
uint16_t UART_Rev_Data_Index = 0;

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
    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);

    /* uart init */
    UART_InitTypeDef UART_InitStruct;
    UART_StructInit(&UART_InitStruct);

    UART_InitStruct.parity         = UART_PARITY_NO_PARTY;
    UART_InitStruct.stopBits       = UART_STOP_BITS_1;
    UART_InitStruct.wordLen        = UART_WROD_LENGTH_8BIT;
    UART_InitStruct.rxTriggerLevel = 16;                      //1~29
    UART_InitStruct.idle_time      = UART_RX_IDLE_2BYTE;      //idle interrupt wait time
    UART_Init(UART, &UART_InitStruct);

    //enable rx interrupt and line status interrupt
    UART_INTConfig(UART, UART_INT_RD_AVA, ENABLE);
    UART_INTConfig(UART, UART_INT_IDLE, ENABLE);

    /*  Enable UART IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = UART0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
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
    uint16_t demoStrLen = 0;

    board_uart_init();
    driver_uart_init();

    char *demoStr = "### Uart demo--Auto Hardware Flow Contrl ###\r\n";
    demoStrLen = strlen(demoStr);
    memcpy(String_Buf, demoStr, demoStrLen);

    /* Send demo tips */
    uart_senddata_continuous(UART, String_Buf, demoStrLen);

    /* Loop rx and tx */
    while (1)
    {
        if (UART_Rev_Data_End_Flag == 1)
        {
            UART_Rev_Data_End_Flag = 0;
            uart_senddata_continuous(UART, UART_Rev_Data, UART_Rev_Data_Lenth);
            for (uint16_t i = 0; i < UART_Rev_Data_Lenth; i++)
            {
                UART_Rev_Data[i] = 0;
            }
        }
    }
}

void UART0_Handler()
{
    uint16_t lenth = 0;
//    uint8_t uart_rev_data[32];

    /* read interrupt id */
    uint32_t int_status = UART_GetIID(UART);

    /* disable interrupt */
    UART_INTConfig(UART, UART_INT_RD_AVA, DISABLE);

    if (UART_GetFlagState(UART, UART_FLAG_RX_IDLE) == SET)
    {
        UART_INTConfig(UART, UART_INT_IDLE, DISABLE);
        DBG_DIRECT("UART0_FLAG_RX_IDLE");
        UART_Rev_Data_End_Flag = 1;
        UART_INTConfig(UART, UART_INT_IDLE, ENABLE);
    }

    switch (int_status & 0x0E)
    {
    /*rx time out*/
    case 0x0C:
        DBG_DIRECT("UART0_Handler rx time out");
        lenth = UART_GetRxFIFOLen(UART);
        UART_ReceiveData(UART, &UART_Rev_Data[UART_Rev_Data_Index], lenth);
        UART_Rev_Data_Lenth += lenth;
        UART_Rev_Data_Index += lenth;
        for (uint8_t i = 0; i < lenth; i++)
        {
            DBG_DIRECT("data=0x%x", UART_Rev_Data[i]);
        }
//            uart_senddata_continuous(UART, uart_rev_data, lenth);
        break;

    /* receiver line status  */
    case 0x06:
        break;

    /* rx data valiable */
    case 0x04:
        DBG_DIRECT("UART0_Handler rx data valiable");
        lenth = UART_GetRxFIFOLen(UART);
        UART_ReceiveData(UART, &UART_Rev_Data[UART_Rev_Data_Index], lenth);
        UART_Rev_Data_Lenth += lenth;
        UART_Rev_Data_Index += lenth;
        for (uint8_t i = 0; i < lenth; i++)
        {
            DBG_DIRECT("data=0x%x", UART_Rev_Data[i]);
        }
//            uart_senddata_continuous(UART, uart_rev_data, lenth);
        break;

    /* tx fifo empty */
    case 0x02:
        /* do nothing */
        break;

    /* modem status */
    case 0x00:
        break;

    default:
        break;
    }

    /* enable interrupt again */
    UART_INTConfig(UART, UART_INT_RD_AVA, ENABLE);
}

