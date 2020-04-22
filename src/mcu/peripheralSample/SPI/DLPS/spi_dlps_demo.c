/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     spi_dlps_demo.c
* @brief    spi+dlps demo code
            This file provides demo code of spi comunication in dlps situation.

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
#include "rtl876x_spi.h"
#include "rtl876x_tim.h"

#include "os_msg.h"
#include "os_queue.h"
#include "os_sched.h"
#include "os_task.h"
#include "os_timer.h"

#include "board.h"
#include "dlps.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#define SPI0_SCK_PIN                P3_2
#define SPI0_MOSI_PIN               P1_2
#define SPI0_MISO_PIN               P1_3
#define SPI0_CS_PIN                 P1_4

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
void board_spi_init(void)
{
    Pad_Config(SPI0_SCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MOSI_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MISO_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_CS_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Config(SPI0_SCK_PIN, SPI0_CLK_MASTER);
    Pinmux_Config(SPI0_MOSI_PIN, SPI0_MO_MASTER);
    Pinmux_Config(SPI0_MISO_PIN, SPI0_MI_MASTER);
    Pinmux_Config(SPI0_CS_PIN, SPI0_SS_N_0_MASTER);
}

/**
  * @brief  Initialize spi peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_spi_init(void)
{
    SPI_InitTypeDef  SPI_InitStruct;
    SPI_StructInit(&SPI_InitStruct);

    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);

    SPI_InitStruct.SPI_Direction   = SPI_Direction_FullDuplex;
    SPI_InitStruct.SPI_Mode        = SPI_Mode_Slave;
    SPI_InitStruct.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStruct.SPI_CPHA        = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_BaudRatePrescaler  = 100;
    SPI_InitStruct.SPI_RxThresholdLevel  =
        0;/* cause SPI_INT_RXF interrupt if data length in receive FIFO  >= SPI_RxThresholdLevel + 1*/
    SPI_InitStruct.SPI_FrameFormat = SPI_Frame_Motorola;

    SPI_Init(SPI0, &SPI_InitStruct);
    SPI_Cmd(SPI0, ENABLE);

}

///**
//  * @brief  Demo code of spi+dlps.
//  * @param  No parameter.
//  * @return void
//*/
//void spi_demo(void)
//{
//    uint8_t spiWriteBuf[16] = {0, 0x01, 0x02, 0x00};
//    uint8_t spiReadBuf[16] = {0, 0 , 0, 0};
//    uint8_t idx = 0;
//    uint8_t len = 0;

//    board_spi_init();
//    driver_spi_init();

//    /*---------------Read flash ID--------------*/
//    spiWriteBuf[0] = 0x9f;
//    SPI_SendBuffer(SPI0, spiWriteBuf, 4);

//    /* Waiting for SPI data transfer to end */
//    while (SPI_GetFlagState(SPI0, SPI_FLAG_BUSY));

//    /*Read ID number of flash GD25Q20*/
//    len = SPI_GetRxFIFOLen(SPI0);
//    for (idx = 0; idx < len; idx++)
//    {
//        spiReadBuf[idx] = SPI_ReceiveData(SPI0);
//        //DBG_BUFFER(MODULE_APP, LEVEL_INFO, "SPI_ReadBuf[%d] = 0x%x", 2, idx, SPI_ReadBuf[idx]);
//        DBG_DIRECT("SPI_ReadBuf[%d] = 0x%X",idx,spiReadBuf[idx]);
//    }

//}

/**
  * @brief  IO enter dlps call back function.
  * @param  No parameter.
  * @return void
*/
void io_dlps_enter(void)
{
    /* Switch pad to Software mode */
    Pad_ControlSelectValue(SPI0_SCK_PIN, PAD_SW_MODE);
    Pad_ControlSelectValue(SPI0_MOSI_PIN, PAD_SW_MODE);
    Pad_ControlSelectValue(SPI0_MISO_PIN, PAD_SW_MODE);
    Pad_ControlSelectValue(SPI0_CS_PIN, PAD_SW_MODE);

    System_WakeUpPinEnable(SPI0_SCK_PIN, PAD_WAKEUP_POL_LOW, 0);
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
    /* Switch pad to Pinmux mode */
    Pad_ControlSelectValue(SPI0_SCK_PIN, PAD_PINMUX_MODE);
    Pad_ControlSelectValue(SPI0_MOSI_PIN, PAD_PINMUX_MODE);
    Pad_ControlSelectValue(SPI0_MISO_PIN, PAD_PINMUX_MODE);
    Pad_ControlSelectValue(SPI0_CS_PIN, PAD_PINMUX_MODE);

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
    System_WakeUpPinEnable(SPI0_SCK_PIN, PAD_WAKEUP_POL_LOW, 0);
}

/**
  * @brief  IO demo task Handle.
  * @param  No parameter.
  * @return void
*/
void dlps_spi_task(void *param)
{
//    uint8_t demoStrLen = 0;
    uint8_t event = 0;
    uint16_t index = 0;

    /* Create event queue and message queue */
    os_msg_queue_create(&IOEventQueueHandle, IO_EVENT_QUEUE_SIZE, sizeof(uint8_t));
    os_msg_queue_create(&IOMessageQueueHandle, IO_MESSAGE_QUEUE_SIZE, sizeof(uint16_t));

    /* Pinmux & Pad Config */
    board_spi_init();
//    board_uart_init();
    /* Power Setting */
    pwr_mgr_init();

    /* Initialize UART peripheral */
    driver_spi_init();
//    driver_uart_init();

//    /* Send demo string */
//    char *demoStr = "### Welcome to use RealTek Bumblebee ###\r\n";
//    demoStrLen = strlen(demoStr);
//    memcpy(String_Buf, demoStr, demoStrLen);
//    uart_senddata_continuous(UART, String_Buf, demoStrLen);

    while (1)
    {
        if (os_msg_recv(IOEventQueueHandle, &event, 0xffffffffUL) == true)
        {
            if (event == IO_EVENT_UART_RX)
            {
//                uart_senddata_continuous(UART, UART_RX_Buffer, UART_RX_Count);

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
    os_task_create(&IOTaskHandle, "dlps_spi", dlps_spi_task, 0, IO_TASK_STACK_SIZE, IO_TASK_PRIORITY);
}

/**
  * @brief  Demo code of dlps+uart.
  * @param  No parameter.
  * @return void
*/
void dlps_spi_demo(void)
{
    app_task_init();
    os_sched_start();
}

/**
  * @brief  Demo code of spi.
  * @param  No parameter.
  * @return void
*/
void spi_demo(void)
{
    dlps_spi_demo();
}

/**
  * @brief  SPI0 interrupt handle function.
  * @param  None.
  * @return None.
  */
void SPI0_Handler(void)
{
    volatile uint8_t len = 0;
    volatile uint8_t idx = 0;
    volatile uint8_t SPI_ReadINTBuf[70] = {0};
    DBG_DIRECT("SPI0_Handler!");

    if (SPI_GetINTStatus(SPI0, SPI_INT_RXF) == SET)
    {
        SPI_ClearINTPendingBit(SPI0, SPI_INT_RXF);
        DBG_DIRECT("SPI_INT_RXF");
        len = SPI_GetRxFIFOLen(SPI0);
        for (idx = 0; idx < len; idx++)
        {
            /* must read all data in receive FIFO , otherwise cause SPI_INT_RXF interrupt again */
            SPI_ReadINTBuf[idx] = SPI_ReceiveData(SPI0);
        }
    }
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
///*--------------------------SPI demo code----------------------------------*/
//void SPI_DemoCode(void)
//{
//    uint8_t SPI_WriteBuf[16] = {0x01, 0x02, 0x00};
//    uint8_t SPI_ReadBuf[16] = {0, 0, 0, 0};
//    uint8_t idx = 0;
//    uint8_t len = 0;

//    /*---------------read flash ID--------------*/
//    SPI_WriteBuf[0] = 0x9f;
//    SPI_SendBuffer(SPI0, SPI_WriteBuf, 4);

//    /*Waiting for SPI data transfer to end*/
//    while (SPI_GetFlagState(SPI0, SPI_FLAG_BUSY));

//    /*read ID number of flash GD25Q20*/
//    len = SPI_GetRxFIFOLen(SPI0);
//    for (idx = 0; idx < len; idx++)
//    {
//        SPI_ReadBuf[idx] = SPI_ReceiveData(SPI0);
//        DBG_BUFFER(TYPE_BEE2, SUBTYPE_FORMAT, MODULE_SPI, LEVEL_INFO,
//                   "SPI_ReadBuf[%d] = 0x%x", 2, idx, SPI_ReadBuf[idx]);
//    }
//}

