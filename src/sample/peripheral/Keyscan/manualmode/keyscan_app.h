/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      scatternet_app.h
   * @brief     This file handles BLE scatternet application routines.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

#ifndef _KEYSCAN_APP_H_
#define _KEYSCAN_APP_H_

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <gap.h>
#include <gap_le.h>
#include <gap_msg.h>
#include <app_msg.h>
/*============================================================================*
 *                              Variables
 *============================================================================*/
#define KeyScan_Debounce_Enable              ((uint32_t)0x1 << 31)
#define KeyScan_Debounce_Disable             ((uint32_t)0x0 << 31)
#define KEYSCAN_INTERVAL      (50)        //50ms
#define MSG_KEYSCAN_PRESS             0
#define MSG_KEYSCAN_RELEASE           1
#define REPEATCODE              1
typedef void *TimerHandle_t;
typedef struct _KEYSCAN_DATA_STRUCT_
{
    uint32_t Length;            /**< Keyscan state register        */
    struct
    {
        uint8_t column: 5;      /**< Keyscan raw buffer data       */
        uint8_t row: 3;         /**< Keyscan raw buffer data       */
        uint8_t reserved;
    } key[8];
} KEYSCAN_DATA_STRUCT, *PKEYSCAN_DATA_STRUCT;
/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg);
void swTimerInit(void);
void HandleKeyscanEvent(T_IO_MSG io_msg);
void keyscan_timer_Release_handler(TimerHandle_t pxTimer);
void keyscan_timer_Reset_handler(TimerHandle_t pxTimer);
void Key_HandleRelease(void);
bool app_send_msg_to_apptask(T_IO_MSG *p_msg);
#ifdef __cplusplus
}
#endif

#endif
