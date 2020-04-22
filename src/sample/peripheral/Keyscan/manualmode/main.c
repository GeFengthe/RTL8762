/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of KEYSCAN.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "rtl876x_keyscan.h"
#include "rtl876x_nvic.h"
#include "trace.h"
#include "os_sched.h"
#include "os_timer.h"
#include "string.h"
#include "board.h"
#include "app_msg.h"
#include "app_task.h"
#include "keyscan_app.h"
/**
 * @brief    pinmux configuration
 * @return   void
 */
void PINMUX_Configuration(void)
{

    /* keypad pinmux config */
    Pinmux_Config(ROW0, KEY_ROW_0);
    Pinmux_Config(ROW1, KEY_ROW_1);
    Pinmux_Config(COLUMN0, KEY_COL_0);
    Pinmux_Config(COLUMN1, KEY_COL_1);
    Pinmux_Config(LED0, DWGPIO);
    Pinmux_Config(LED1, DWGPIO);
    return;
}
/**
 * @brief    pad configuration
 * @return   void
 */
void PAD_Configuration(void)
{
    /* Keypad pad config */
    Pad_Config(ROW0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(ROW1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(COLUMN0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(COLUMN1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(LED0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(LED1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    return;
}

/**
 * @brief    rcc configuration
 * @return   void
 */
void RCC_Configuration(void)
{
    /* Enable KEYSCAN clock */
    RCC_PeriphClockCmd(APBPeriph_KEYSCAN, APBPeriph_KEYSCAN_CLOCK, ENABLE);
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
    return;
}

/**
 * @brief    Contains the initialization of pinmux settings and pad settings
 * @note     All the pinmux settings and pad settings shall be initiated in this function,
 *           but if legacy driver is used, the initialization of pinmux setting and pad setting
 *           should be peformed with the IO initializing.
 * @return   void
 */
void board_init(void)
{
    RCC_Configuration();
    PINMUX_Configuration();
    PAD_Configuration();
}

/**
  * @brief  Initialize GPIO peripheral.
  * @param   No parameter.
  * @return  void
  */
void KeyScanInit(bool isDebounce)
{
    KEYSCAN_InitTypeDef  KeyScan_InitStruct;
    KeyScan_StructInit(&KeyScan_InitStruct);
    KeyScan_InitStruct.colSize         = KEYPAD_COLUMN_SIZE;
    KeyScan_InitStruct.rowSize         = KEYPAD_ROW_SIZE;
    KeyScan_InitStruct.scanmode        = KeyScan_Manual_Scan_Mode;
    KeyScan_InitStruct.clockdiv         = 0x26;      //128kHz = 5MHz/(clockdiv+1)
    KeyScan_InitStruct.delayclk         = 0x0f;       //8kHz = 5MHz/(clockdiv+1)/(delayclk+1)
    KeyScan_InitStruct.debounceEn       = isDebounce;
    KeyScan_InitStruct.scantimerEn      = KeyScan_ScanInterval_Disable;
    KeyScan_InitStruct.detecttimerEn    = KeyScan_Release_Detect_Disable;
    KeyScan_InitStruct.scanInterval     = 0x190;  //50ms = scanInterval/8kHz
    KeyScan_InitStruct.debouncecnt      = 0x40;   //8ms = debouncecnt/8kHz
    KeyScan_InitStruct.releasecnt       = 0x01;   //0.125ms = releasecnt/8kHz

    KeyScan_Init(KEYSCAN, &KeyScan_InitStruct);
    KeyScan_INTConfig(KEYSCAN, KEYSCAN_INT_SCAN_END, ENABLE);
    KeyScan_Cmd(KEYSCAN, ENABLE);
    NVIC_InitTypeDef NVIC_InitStruct;

    /* Keyscan IRQ */
    NVIC_InitStruct.NVIC_IRQChannel = KeyScan_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStruct);
}
void driver_init(void)
{
    GPIO_InitTypeDef Gpio_Struct;
    GPIO_StructInit(&Gpio_Struct);
    Gpio_Struct.GPIO_Pin = GPIO_GetPin(LED0) | GPIO_GetPin(LED1);
    Gpio_Struct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(&Gpio_Struct);

}
int main(void)
{
    board_init();
    KeyScanInit(KeyScan_Debounce_Enable);
    swTimerInit();
    app_task_init();
    os_sched_start();

}
