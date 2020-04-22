/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     io_uart.c
* @brief    uart interrupt demo
* @details
* @author   yuan
* @date     2018-06-28
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "io_uart.h"

#include <string.h>

#include "app_task.h"

#include "trace.h"


/* Globals ------------------------------------------------------------------*/
uint8_t UART_RX_Buffer[256];
uint8_t UART_RX_Count = 0;

bool IO_UART_DLPS_Enter_Allowed = false;

/**
  * @brief  Initialize uart global data.
  * @param  No parameter.
  * @return void
  */
void global_data_uart_init(void)
{
    IO_UART_DLPS_Enter_Allowed = true;
    UART_RX_Count = 0;
    memset(UART_RX_Buffer, 0, sizeof(UART_RX_Buffer));
}

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
  * @brief  IO enter dlps call back function.
  * @param  No parameter.
  * @return void
  */
void io_uart_dlps_enter(void)
{
    /* Switch pad to Software mode */
    Pad_ControlSelectValue(UART_TX_PIN, PAD_SW_MODE);
    Pad_ControlSelectValue(UART_RX_PIN, PAD_SW_MODE);

    System_WakeUpPinEnable(UART_RX_PIN, PAD_WAKEUP_POL_LOW, 0);
}

/**
  * @brief  IO exit dlps call back function.
  * @param  No parameter.
  * @return void
  */
void io_uart_dlps_exit(void)
{
    /* Switch pad to Pinmux mode */
    Pad_ControlSelectValue(UART_TX_PIN, PAD_PINMUX_MODE);
    Pad_ControlSelectValue(UART_RX_PIN, PAD_PINMUX_MODE);

}

/**
  * @brief  IO enter dlps check function.
  * @param  No parameter.
  * @return void
  */
bool io_uart_dlps_check(void)
{
    return IO_UART_DLPS_Enter_Allowed;
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
  * @brief  Handle uart data function.
  * @param  No parameter.
  * @return void
  */
void io_uart_handle_msg(T_IO_MSG *io_uart_msg)
{
//    uint8_t *p_buf = io_uart_msg.u.buf;
    uint16_t subtype = io_uart_msg->subtype;

    if (IO_MSG_UART_RX == subtype)
    {
        uart_senddata_continuous(UART, UART_RX_Buffer, UART_RX_Count);
        global_data_uart_init();
        while (UART_GetFlagState(UART, UART_FLAG_THR_EMPTY) == 0) { IO_UART_DLPS_Enter_Allowed = true; }
    }
}

/**
  * @brief  Handle uart msg function.
  * @param  No parameter.
  * @return void
  */
void io_handle_uart_msg(T_IO_MSG *io_uart_msg)
{
    io_uart_handle_msg(io_uart_msg);
}

void UART0_Handler()
{
    uint16_t rx_len = 0;

    /* Read interrupt id */
    uint32_t int_status = UART_GetIID(UART);

    /* Disable interrupt */
    UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_LINE_STS, DISABLE);

    if (UART_GetFlagState(UART, UART_FLAG_RX_IDLE) == SET)
    {
        /* Clear flag */
        UART_INTConfig(UART, UART_INT_IDLE, DISABLE);

        /* Send msg to app task */
        T_IO_MSG int_uart_msg;

        int_uart_msg.type = IO_MSG_TYPE_UART;
        int_uart_msg.subtype = IO_MSG_UART_RX;
        UART_RX_Buffer[UART_RX_Count] = UART_RX_Count;
        int_uart_msg.u.buf = (void *)(&UART_RX_Buffer);
        if (false == app_send_msg_to_apptask(&int_uart_msg))
        {
            APP_PRINT_ERROR0("[io_uart] UART0_Handler: Send int_uart_msg failed!");
            //Add user code here!
            return;
        }
//        IO_UART_DLPS_Enter_Allowed = true;
        UART_INTConfig(UART, UART_INT_IDLE, ENABLE);
    }

    switch (int_status & 0x0E)
    {
    /* Rx time out(0x0C). */
    case UART_INT_ID_RX_TMEOUT:
        rx_len = UART_GetRxFIFOLen(UART);
        UART_ReceiveData(UART, &UART_RX_Buffer[UART_RX_Count], rx_len);
        UART_RX_Count += rx_len;
        break;

    /* Receive line status interrupt(0x06). */
    case UART_INT_ID_LINE_STATUS:
        break;

    /* Rx data valiable(0x04). */
    case UART_INT_ID_RX_LEVEL_REACH:
        rx_len = UART_GetRxFIFOLen(UART);
        UART_ReceiveData(UART, &UART_RX_Buffer[UART_RX_Count], rx_len);
        UART_RX_Count += rx_len;
        break;

    /* Tx fifo empty(0x02), not enable. */
    case UART_INT_ID_TX_EMPTY:
        /* Do nothing */
        break;
    default:
        break;
    }

    /* enable interrupt again */
    UART_INTConfig(UART, UART_INT_RD_AVA, ENABLE);
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
