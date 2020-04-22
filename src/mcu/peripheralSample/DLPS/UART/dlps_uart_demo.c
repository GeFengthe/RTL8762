/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     dlps_uart_demo.c
* @brief    dlps+uart demo code
* @details
* @author   yuan
* @date     2018-06-21
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>

#include "rtl876x_io_dlps.h"
#include "rtl876x_ir.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_tim.h"
#include "rtl876x_uart.h"

#include "os_msg.h"
#include "os_queue.h"
#include "os_sched.h"
#include "os_task.h"
#include "os_timer.h"

#include "board.h"
#include "dlps.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#define UART_TX_PIN                     P2_4
#define UART_RX_PIN                     P2_5

/* Queue size & stack size & task priority */
#define IO_TASK_STACK_SIZE              1024
//#define IO_TASK_PRIORITY                       (tskIDLE_PRIORITY + 1)
#define IO_TASK_PRIORITY                (1)
#define IO_EVENT_QUEUE_SIZE             0x10
#define IO_MESSAGE_QUEUE_SIZE           0x10

#define IR_SEND_TASK_STACK_SIZE         1024
#define IR_SEND_TASK_PRIORITY           (1)

/* DLPS message type */
#define IO_EVENT_UART_RX                0x01
#define IO_EVENT_IR_TX_DONE             0x02

/* Globals ------------------------------------------------------------------*/
uint8_t String_Buf[100];
uint8_t UART_RX_Buffer[600];
uint32_t UART_RX_Count = 0;

bool  allowedSystemEnterDlps = false;

/* Task handle & queue handle */
void *IOTaskHandle;
void *IOEventQueueHandle;
void *IOMessageQueueHandle;


/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_uart_init(void)
{
    Pad_Config(UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);

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

    /* UART init */
    UART_InitTypeDef UART_InitStruct;
    UART_StructInit(&UART_InitStruct);

    /* Default baudrate is 115200 */
    UART_InitStruct.dmaEn = UART_DMA_ENABLE;
    UART_Init(UART, &UART_InitStruct);

    /* Enable rx interrupt and line status interrupt */
    UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_IDLE, ENABLE);

    /* Enable UART IRQ */
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
  * @brief  IO enter dlps call back function.
  * @param  No parameter.
  * @return void
*/
void io_dlps_enter(void)
{
    /* Switch pad to Software mode */
    Pad_ControlSelectValue(UART_TX_PIN, PAD_SW_MODE);
    Pad_ControlSelectValue(UART_RX_PIN, PAD_SW_MODE);

    System_WakeUpPinEnable(UART_RX_PIN, PAD_WAKEUP_POL_LOW, 0);
    /* To Debug */
    DBG_DIRECT("[DLPS] io_dlps_enter");
}

/**
  * @brief  IO exit dlps call back function.
  * @param  No parameter.
  * @return void
*/
void io_dlps_exit(void)
{
    /* switch pad to Pinmux mode */
    Pad_ControlSelectValue(UART_TX_PIN, PAD_PINMUX_MODE);
    Pad_ControlSelectValue(UART_RX_PIN, PAD_PINMUX_MODE);

    /* To Debug */
    DBG_DIRECT("[DLPS] io_dlps_exit");
//    DBG_BUFFER(TYPE_BEE2, SUBTYPE_FORMAT, MODULE_DLPS, LEVEL_INFO, "io_dlps_exit", 0);
}

/**
  * @brief  IO enter dlps check function.
  * @param  No parameter.
  * @return void
*/
bool io_dlps_check(void)
{
    return allowedSystemEnterDlps;
}

/**
  * @brief  The setting about power mode.
  * @param  No parameter.
  * @return void
*/
void pwr_mgr_init(void)
{
    if (false == dlps_check_cb_reg(io_dlps_check))
    {
        DBG_BUFFER(TYPE_BEE2, SUBTYPE_FORMAT, MODULE_DLPS, LEVEL_ERROR,
                   "dlps_check_cb_reg(io_dlps_check) failed!!", 0);
    }
    DLPS_IORegUserDlpsEnterCb(io_dlps_enter);
    DLPS_IORegUserDlpsExitCb(io_dlps_exit);
    DLPS_IORegister();
    lps_mode_set(LPM_DLPS_MODE);

    /* Config WakeUp pin */
    System_WakeUpPinEnable(UART_RX_PIN, PAD_WAKEUP_POL_LOW, 0);
}

/**
  * @brief  IO demo task Handle.
  * @param  No parameter.
  * @return void
*/
void dlps_uart_task(void *param)
{
    uint8_t demoStrLen = 0;
    uint8_t event = 0;
    uint16_t index = 0;

    /* Create event queue and message queue */
    os_msg_queue_create(&IOEventQueueHandle, IO_EVENT_QUEUE_SIZE, sizeof(uint8_t));
    os_msg_queue_create(&IOMessageQueueHandle, IO_MESSAGE_QUEUE_SIZE, sizeof(uint16_t));

    /* Pinmux & Pad Config */
    board_uart_init();
    /* Power Setting */
    pwr_mgr_init();

    /* Initialize UART peripheral */
    driver_uart_init();

    /* Send demo string */
    char *demoStr = "### Welcome to use RealTek Bumblebee ###\r\n";
    demoStrLen = strlen(demoStr);
    memcpy(String_Buf, demoStr, demoStrLen);
    uart_senddata_continuous(UART, String_Buf, demoStrLen);

    while (1)
    {
        if (os_msg_recv(IOEventQueueHandle, &event, 0xffffffffUL) == true)
        {
            if (event == IO_EVENT_UART_RX)
            {
                uart_senddata_continuous(UART, UART_RX_Buffer, UART_RX_Count);

                for (index = 0; index < 500; index++)
                {
                    UART_RX_Buffer[index] = 0;
                }
                UART_RX_Count = 0;
            }

        }
    }
}

/**
  * @brief  Initialize peripheral Task.
  * @param  No parameter.
  * @return void
*/
void app_task_init(void)
{
    os_task_create(&IOTaskHandle, "dlps_uart", dlps_uart_task, 0, IO_TASK_STACK_SIZE, IO_TASK_PRIORITY);
}

/**
  * @brief  Demo code of dlps+uart.
  * @param  No parameter.
  * @return void
*/
void dlps_uart_demo(void)
{
    app_task_init();
    os_sched_start();
}

/**
  * @brief  Uart interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void UART0_Handler(void)
{
    uint8_t event = IO_EVENT_UART_RX;
    uint8_t rxFifoCnt = 0;
    uint32_t intStatus = 0;
    intStatus = UART_GetIID(UART);

    /* disable interrupt */
    UART_INTConfig(UART, UART_INT_RD_AVA, DISABLE);

    if (UART_GetFlagState(UART, UART_FLAG_RX_IDLE) == SET)
    {
        //clear Flag
        UART_INTConfig(UART, UART_INT_IDLE, DISABLE);
        os_msg_send(IOEventQueueHandle, &event, 0);
        allowedSystemEnterDlps = true;
        UART_INTConfig(UART, UART_INT_IDLE, ENABLE);
    }

    switch (intStatus)
    {
    /* Tx fifo empty, not enable */
    case UART_INT_ID_TX_EMPTY:
        break;

    /* Rx data valiable */
    case UART_INT_ID_RX_LEVEL_REACH:
        rxFifoCnt = UART_GetRxFIFOLen(UART);
        UART_ReceiveData(UART, &UART_RX_Buffer[UART_RX_Count], rxFifoCnt);
        UART_RX_Count += rxFifoCnt;
        break;

    case UART_INT_ID_RX_TMEOUT:
        rxFifoCnt = UART_GetRxFIFOLen(UART);
        UART_ReceiveData(UART, &UART_RX_Buffer[UART_RX_Count], rxFifoCnt);
        UART_RX_Count += rxFifoCnt;
        break;

    /* Receive line status interrupt */
    case UART_INT_ID_LINE_STATUS:
        {
            //DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "Line status error!!!!\n", 0);
            DBG_BUFFER(TYPE_BEE2, SUBTYPE_FORMAT, MODULE_UART, LEVEL_WARN, "Line status error!!!!", 0);
        }
        break;

    default:
        break;
    }
    /* Enable interrupt again */
    UART_INTConfig(UART, UART_INT_RD_AVA, ENABLE);

    return;
}

/**
  * @brief  System interrupt handler function, for wakeup pin.
  * @param  No parameter.
  * @return void
*/
void System_Handler(void)
{
    DBG_BUFFER(TYPE_BEE2, SUBTYPE_FORMAT, MODULE_DLPS, LEVEL_INFO, "System_Handler", 0);
    if (System_WakeUpInterruptValue(P2_5) == SET)
    {
        Pad_ClearWakeupINTPendingBit(P2_5);
        System_WakeUpPinDisable(P2_5);
        allowedSystemEnterDlps = false;
    }
}

/**
  * @brief  IO parameter check fail.
  * @param  No parameter.
  * @return void
*/
void io_assert_failed(uint8_t *file, uint32_t line)
{
    DBG_DIRECT("IO driver parameters error! File_name: %s, line: %d", file, line);

    for (;;);
}
