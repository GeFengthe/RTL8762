/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      keyboard.h
* @brief     Header file of keyboard demo.
* @details
* @author    yuan
* @date      2018-05-29
* @version   v0.1
* *********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>

#include "rtl876x_keyscan.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_uart.h"

#include "os_msg.h"
#include "os_queue.h"
#include "os_sched.h"
#include "os_task.h"
#include "os_timer.h"

#include "trace.h"

/* Defines ------------------------------------------------------------------*/
/* Pin define of keypad row and column */
#define KEYBOARD_ROW_SIZE                   2
#define KEYBOARD_COLUMN_SIZE                2
#define KEYBOARD_ROW_0                      P2_3
#define KEYBOARD_ROW_1                      P2_4
#define KEYBOARD_COLUMN_0                   P4_0
#define KEYBOARD_COLUMN_1                   P4_1

/* Pin define of UART peripheral */
#define UART_TX_PIN                         P3_2
#define UART_RX_PIN                         P3_3

/* Queue size & stack size & task priority */
#define IO_TASK_STACK_SIZE                  1024
//#define IO_DEMO_TASK_PRIORITY               (tskIDLE_PRIORITY + 1)
#define IO_TASK_PRIORITY                    (1)
#define IO_MESSAGE_QUEUE_SIZE               0x10

/* Keyscan message type */
#define IO_EVENT_KEY_VALUE_UPDATE           0x01
#define IO_EVENT_KEY_RELEASE                0x02

#define KEYSCAN_TIMEOUT                     (30)        //30ms

/* Typedefs ------------------------------------------------------------------*/
typedef struct
{
    uint32_t length;
    struct
    {
        uint16_t column: 5;   /**< keyscan column buffer data   */
        uint16_t row: 4;      /**< keyscan raw buffer data      */
    } key[26];
} KeyScan_DataStruct, *PKeyScan_DataStruct;

/* keyscan message type */
typedef struct
{
    uint16_t msgType;
    union
    {
        uint32_t parm;
        void *pBuf;
    };
} KeyScan_MsgStruct;

/* Globals ------------------------------------------------------------------*/
uint8_t String_Buf[100];
uint8_t Repeat_Report = 0;

KeyScan_DataStruct Current_KeyData;
KeyScan_DataStruct Pre_KeyData;

/* Task handle & queue handle */
void *KeyScanTimerHandle = NULL;
void *IOMessageQueueHandle;
void *IOTaskHandle;

//IRQ_FUN UserIrqFunTable[32 + 17];

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
*/
void board_keyboard_init(void)
{
    /* Keypad pad config */
    Pad_Config(KEYBOARD_ROW_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(KEYBOARD_ROW_1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(KEYBOARD_COLUMN_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pad_Config(KEYBOARD_COLUMN_1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);

    /* keypad pinmux config */
    Pinmux_Config(KEYBOARD_ROW_0, KEY_ROW_0);
    Pinmux_Config(KEYBOARD_ROW_1, KEY_ROW_1);
    Pinmux_Config(KEYBOARD_COLUMN_0, KEY_COL_0);
    Pinmux_Config(KEYBOARD_COLUMN_1, KEY_COL_1);
}

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
*/
void board_uart_init(void)
{
    Pad_Config(UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);

    Pinmux_Config(UART_TX_PIN, UART0_TX);
    Pinmux_Config(UART_RX_PIN, UART0_RX);
}

/**
  * @brief  Initialize keyboard peripheral.
  * @param  No parameter.
  * @return Void
 */
void driver_keyboard_init(void)
{
    /* data struct init */
    memset(&Current_KeyData, 0, sizeof(KeyScan_DataStruct));
    memset(&Pre_KeyData, 0, sizeof(KeyScan_DataStruct));
    Repeat_Report = false;

    /* turn on Keyscan clock */
    RCC_PeriphClockCmd(APBPeriph_KEYSCAN, APBPeriph_KEYSCAN_CLOCK, ENABLE);

    KEYSCAN_InitTypeDef KeyScan_InitStruct;

    KeyScan_StructInit(&KeyScan_InitStruct);
//    KeyScan_InitStruct.debounceTime    = (16 * 32); //16ms
    KeyScan_InitStruct.rowSize = KEYBOARD_ROW_SIZE;
    KeyScan_InitStruct.colSize = KEYBOARD_COLUMN_SIZE;
//    KeyScan_InitStruct.scanmode = KeyScan_Manual_Scan_Mode;
    KeyScan_InitStruct.scanInterval    = 0x80;
    KeyScan_Init(KEYSCAN, &KeyScan_InitStruct);

    KeyScan_INTConfig(KEYSCAN, KEYSCAN_INT_SCAN_END | KEYSCAN_INT_ALL_RELEASE, ENABLE);
    KeyScan_Cmd(KEYSCAN, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    /* Keyscan IRQ */
    NVIC_InitStruct.NVIC_IRQChannel = KeyScan_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Initialize uart peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_uart_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);

    /* UART init */
    UART_InitTypeDef UART_InitStruct;

    UART_StructInit(&UART_InitStruct);
    /* Default baudrate is 115200 */
    UART_Init(UART, &UART_InitStruct);

    /* Enable rx interrupt and line status interrupt */
//    UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_LINE_STS, ENABLE);
    UART_INTConfig(UART, UART_INT_RD_AVA, ENABLE);

    /* Enable UART IRQ */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = UART0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  UART send data continuous.
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

/* Hex to Ascii */
uint8_t HexToAscii(uint8_t hex)
{
    if (hex <= 9)
    {
        return (hex - 0x00 + 0x30);
    }
    else if (hex >= 0x0A && hex <= 0x0F)
    {
        return (hex - 0x0A + 0x41);
    }
    else
    {
        /* wrong */
        return 0xFF;
    }
}

void keyscan_task(void *param)
{
    uint8_t demoStrLen = 0;
//    uint8_t event = 0;
    KeyScan_MsgStruct msg;

    char *demoTipStr = "### Bee keyboard demo ###\r\n";
    char *demoOneKeyStr = "## One key press:(0, 0) ##\r\n";
    char *demoTwoKeyStr = "## Two keys press:(0, 0),(0, 0) ##\r\n";
    char *demoKeyReleaseStr = "## Key Release ##\r\n";

    os_msg_queue_create(&IO_Message_Queue_Handle, IO_MESSAGE_QUEUE_SIZE, sizeof(KeyScan_MsgStruct));

    board_keyboard_init();
    board_uart_init();

    driver_uart_init();
    driver_keyboard_init();

    /********************send demo tips through uart *************/
    demoStrLen = strlen(demoTipStr);
    memcpy(String_Buf, demoTipStr, demoStrLen);
    uart_senddata_continuous(UART, String_Buf, demoStrLen);


    /************************** start task while loop **********************/
    while (1)
    {
        /* key value update message */
        while (os_msg_recv(IO_Message_Queue_Handle, &msg, 0xffffffffUL) == true)
        {
            if (msg.msgType == IO_EVENT_KEY_VALUE_UPDATE)
            {
                PKeyScan_DataStruct pKeyData = (PKeyScan_DataStruct)msg.pBuf;
                /* signal key press */
                if (pKeyData->length == 1)
                {
                    demoStrLen = strlen(demoOneKeyStr);
                    memcpy(String_Buf, demoOneKeyStr, demoStrLen);
                    String_Buf[18] = HexToAscii(pKeyData->key[0].row);
                    String_Buf[21] = HexToAscii(pKeyData->key[0].column);
                    DBG_DIRECT("--->signal press: position: (%d, %d)", pKeyData->key[0].row, pKeyData->key[0].column);
//                    DBG_BUFFER(MODULE_APP, LEVEL_INFO, "--->signal press: position: (%d, %d)", 2, pKeyData->key[0].row, pKeyData->key[0].column);
                }

                /* two keys press */
                if (pKeyData->length == 2)
                {
                    demoStrLen = strlen(demoTwoKeyStr);
                    memcpy(String_Buf, demoTwoKeyStr, demoStrLen);
                    String_Buf[19] = HexToAscii(pKeyData->key[0].row);
                    String_Buf[22] = HexToAscii(pKeyData->key[0].column);
                    String_Buf[26] = HexToAscii(pKeyData->key[1].row);
                    String_Buf[29] = HexToAscii(pKeyData->key[1].column);
                    DBG_DIRECT("--->double key press: position: (%d, %d), (%d, %d)", \
                               pKeyData->key[0].row, pKeyData->key[0].column, \
                               pKeyData->key[1].row, pKeyData->key[1].column);
//                    DBG_BUFFER(MODULE_APP, LEVEL_INFO, "--->double key press: position: (%d, %d), (%d, %d)", 4,
//                               pKeyData->key[0].row, pKeyData->key[0].column,
//                               pKeyData->key[1].row, pKeyData->key[1].column);

                }
            }
            else if (msg.msgType == IO_EVENT_KEY_RELEASE)
            {
                if (msg.msgType == IO_EVENT_KEY_RELEASE)
                {
                    demoStrLen = strlen(demoKeyReleaseStr);
                    memcpy(String_Buf, demoKeyReleaseStr, demoStrLen);
                    DBG_DIRECT("--->Key Release!!");
//                    DBG_BUFFER(MODULE_APP, LEVEL_INFO, "--->Key Release!!", 0);
                }
            }
            else
            {
                DBG_DIRECT("--->wrong key event!!");
//                DBG_BUFFER(MODULE_APP, LEVEL_INFO, "--->wrong key event!!", 0);
            }

            /* send key value through uart */
//            uart_senddata_continuous(UART, String_Buf, demoStrLen);
        }
    }
}

void keyscan_timer_callback(void *xTimer)
{
    memset(&Current_KeyData, 0, sizeof(KeyScan_DataStruct));
    memset(&Pre_KeyData, 0, sizeof(KeyScan_DataStruct));

    KeyScan_MsgStruct msg;
    msg.msgType = IO_EVENT_KEY_RELEASE;
    msg.pBuf = NULL;

//    void  os_queue_in(T_OS_QUEUE *p_queue, void *p_elem);
    os_queue_in(IO_Message_Queue_Handle, &msg);

    return;
}

void sw_timer_init(void)
{
    bool retval ;
    retval = os_timer_create(&KeyScan_Timer_Handle,
                             "KeyScanTimer",                 /* Just a text name, not used by the kernel.    */
                             1,                              /* Assign each timer a unique id equal to its array index.  */
                             KEYSCAN_TIMEOUT,                /* The timer period in ticks. KeyTimeoutPeriod = Interval_time +Debounce_time +2ms  */
                             false,                          /* The timers will auto-reload themselves when they expire. */
                             keyscan_timer_callback          /* Each timer calls the same callback when it expires.  */
                            );
    if (!retval)
    {
        APP_PRINT_INFO1("KeyScanTimer retval is %d", retval);
    }
}

void app_task_init(void)
{
    os_task_create(&IOTaskHandle, "keyscan", keyscan_task, 0, IO_TASK_STACK_SIZE, IO_TASK_PRIORITY);
}


void keyscan_demo(void)
{
//    sw_timer_init();
    app_task_init();
    os_sched_start();
}

//void io_assert_failed(uint8_t *file, uint32_t line)
//{
//    DBG_DIRECT("IO driver parameters error! file_name: %s, line: %d", file, line);

//    for (;;);
//}

/**
  * @brief  Keyscan interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void Keyscan_Handler(void)
{
    uint32_t fifo_length;
    PKeyScan_DataStruct pKeyData = &Current_KeyData;
    KeyScan_MsgStruct msg;
    //uint32_t xTimerPeriod=100;
//    long xHigherPriorityTaskWoken = false;

    if (KeyScan_GetFlagState(KEYSCAN, KEYSCAN_INT_FLAG_ALL_RELEASE) == SET)
    {
        /* Mask keyscan interrupt */
        memset(&Pre_KeyData, 0, sizeof(KeyScan_DataStruct));
        KeyScan_INTMask(KEYSCAN, KEYSCAN_INT_ALL_RELEASE, ENABLE);

        msg.msgType = IO_EVENT_KEY_RELEASE;
        msg.pBuf = (void *)pKeyData;
        os_msg_send(IO_Message_Queue_Handle, &msg, 0);

        /* clear & Unmask keyscan interrupt */
        KeyScan_ClearINTPendingBit(KEYSCAN, KEYSCAN_INT_ALL_RELEASE);
        KeyScan_INTMask(KEYSCAN, KEYSCAN_INT_ALL_RELEASE, DISABLE);
    }
    if (KeyScan_GetFlagState(KEYSCAN, KEYSCAN_INT_FLAG_SCAN_END) == SET)
    {
        /* Read current keyscan interrupt status and mask interrupt */
        KeyScan_INTMask(KEYSCAN, KEYSCAN_INT_SCAN_END, ENABLE);
        KeyScan_ClearINTPendingBit(KEYSCAN, KEYSCAN_INT_SCAN_END);

        /* KeyScan fifo not empty */
        if (KeyScan_GetFlagState(KEYSCAN, KEYSCAN_FLAG_EMPTY) != SET)
        {
            memset(pKeyData, 0, sizeof(KeyScan_DataStruct));
            fifo_length = (uint32_t)KeyScan_GetFifoDataNum(KEYSCAN);
            KeyScan_Read(KEYSCAN, (uint16_t *)&pKeyData->key[0], fifo_length);
            pKeyData->length = fifo_length;

            if (!Repeat_Report)
            {
                if (!memcmp(pKeyData, &Pre_KeyData, sizeof(KeyScan_DataStruct)))
                {
                    goto UNMASK_INT;
                }
                else
                {
                    memcpy(&Pre_KeyData, pKeyData, sizeof(KeyScan_DataStruct));
                }
            }

            /* Send event to app task */
            msg.msgType = IO_EVENT_KEY_VALUE_UPDATE;
            msg.pBuf = (void *)pKeyData;

            //send msg
            os_msg_send(IO_Message_Queue_Handle, &msg, 0);
        }

UNMASK_INT:
        /* clear & Unmask keyscan interrupt */
        KeyScan_ClearINTPendingBit(KEYSCAN, KEYSCAN_INT_SCAN_END);
        KeyScan_INTMask(KEYSCAN, KEYSCAN_INT_SCAN_END, DISABLE);
    }

}

