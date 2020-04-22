/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      scatternet_app.c
   * @brief     This file handles BLE scatternet application routines.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <trace.h>
#include <string.h>
#include <gap.h>
#include <gap_adv.h>
#include <gap_bond_le.h>
#include <gap_msg.h>
#include <app_msg.h>
#include <gap_conn_le.h>
#include <os_timer.h>
#include "keyscan_app.h"
#include "rtl876x_gpio.h"
#include "rtl876x_keyscan.h"
#include "board.h"
/*============================================================================*
 *                              Variables
 *============================================================================*/
KEYSCAN_DATA_STRUCT PreKeyData;
KEYSCAN_DATA_STRUCT CurKeyData;
TimerHandle_t xTimersKeyScanRelease;
TimerHandle_t xTimersKeyScanReset;
/*============================================================================*
 *                              Functions
 *============================================================================*/
void app_handle_gap_msg(T_IO_MSG  *p_gap_msg);
void KeyScanInit(bool isDebounce);
/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg)
{
    uint16_t msg_type = io_msg.type;

    switch (msg_type)
    {
    case IO_MSG_TYPE_BT_STATUS:
        break;
    case IO_MSG_TYPE_GPIO:
        {
            HandleKeyscanEvent(io_msg);
        }
        break;
    default:
        break;
    }
}
void Key_HandlePressed(PKEYSCAN_DATA_STRUCT pKeyData)
{
    if (pKeyData->Length == 1)
    {
        APP_PRINT_INFO2(" Singlekey row - %d, column - %d\n", pKeyData->key[0].column,
                        pKeyData->key[0].row);
        GPIO_SetBits(GPIO_GetPin(LED0));
    }
    if (pKeyData->Length == 2)
    {
        APP_PRINT_INFO2(" CombKey row - %d, column - %d\n", pKeyData->key[0].column, pKeyData->key[0].row);
        APP_PRINT_INFO2(" CombKey row - %d, column - %d\n", pKeyData->key[1].column, pKeyData->key[1].row);
        GPIO_SetBits(GPIO_GetPin(LED0));
        GPIO_SetBits(GPIO_GetPin(LED1));
    }
    else
    {

    }
    return;
}
/**
 * @brief    HandleButtonEvent for keyscan event
 * @note     Event for keyscan.
 * @param[in] io_msg
 * @return   void
 */
void HandleKeyscanEvent(T_IO_MSG io_msg)
{
    uint8_t keytype = io_msg.subtype ;

    if (keytype == MSG_KEYSCAN_PRESS)
    {
        Key_HandlePressed(&PreKeyData);
    }
    else if (keytype == MSG_KEYSCAN_RELEASE)
    {
        Key_HandleRelease();
        GPIO_ResetBits(GPIO_GetPin(LED0));
        GPIO_ResetBits(GPIO_GetPin(LED1));
    }
}
void Keyscan_Handler(void)
{
    T_IO_MSG bee_io_msg;
    PKEYSCAN_DATA_STRUCT pKeyData = &CurKeyData;
    uint32_t len = 0;
    if (KeyScan_GetFlagState(KEYSCAN, KEYSCAN_INT_FLAG_SCAN_END) == SET)   // scan finish
    {

        /* Mask keyscan interrupt */
        KeyScan_INTMask(KEYSCAN, KEYSCAN_INT_SCAN_END, ENABLE);
        os_timer_restart(&xTimersKeyScanReset, KEYSCAN_INTERVAL);
        os_timer_stop(&xTimersKeyScanRelease);
        /* KeyScan fifo not empty */
        if (KeyScan_GetFlagState(KEYSCAN, KEYSCAN_FLAG_EMPTY) != SET)
        {
            /* Read fifo data */
            len = KeyScan_GetFifoDataNum(KEYSCAN);
            pKeyData->Length = len;
            KeyScan_Read(KEYSCAN, (uint16_t *)&pKeyData->key[0], len);
#if REPEATCODE
            if (!memcmp(pKeyData, &PreKeyData, sizeof(KEYSCAN_DATA_STRUCT)))
            {

            }
            else
            {
#endif
                bee_io_msg.type = IO_MSG_TYPE_GPIO;
                bee_io_msg.subtype = MSG_KEYSCAN_PRESS;
                app_send_msg_to_apptask(&bee_io_msg);
                memcpy(&PreKeyData, pKeyData, sizeof(KEYSCAN_DATA_STRUCT));
#if REPEATCODE
            }
#endif
        }
        /* clear & Unmask keyscan interrupt */
        KeyScan_ClearINTPendingBit(KEYSCAN, KEYSCAN_INT_SCAN_END);
        KeyScan_INTMask(KEYSCAN, KEYSCAN_INT_SCAN_END, DISABLE);
    }
}

void keyscan_timer_Release_handler(TimerHandle_t pxTimer)
{
    T_IO_MSG bee_io_msg;
    bee_io_msg.type = IO_MSG_TYPE_GPIO;
    bee_io_msg.subtype = MSG_KEYSCAN_RELEASE;
    app_send_msg_to_apptask(&bee_io_msg);
}
void keyscan_timer_Reset_handler(TimerHandle_t pxTimer)
{
    KeyScanInit(KeyScan_Debounce_Enable);
    os_timer_start(&xTimersKeyScanRelease);
    KeyScan_ClearINTPendingBit(KEYSCAN, KEYSCAN_INT_SCAN_END);
    KeyScan_INTMask(KEYSCAN, KEYSCAN_INT_SCAN_END, DISABLE);
}
void swTimerInit()
{
    os_timer_create(&xTimersKeyScanRelease, "keyscan_timer",  1, \
                    KEYSCAN_INTERVAL, false, keyscan_timer_Release_handler);
    os_timer_create(&xTimersKeyScanReset, "keyscan_timer",  1, \
                    KEYSCAN_INTERVAL, false, keyscan_timer_Reset_handler);
}
void Key_HandleRelease()
{
    memset(&PreKeyData, 0, sizeof(KEYSCAN_DATA_STRUCT));
    APP_PRINT_INFO0(" Release");
    return;
}
