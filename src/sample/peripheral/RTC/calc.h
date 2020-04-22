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
#define RTC_PRESCALER_VALUE         0
#define ClOCK_RTC_COMPARATOR        0
#define SecondCountRTC              0
#define IO_MSG_RTC_CLOCK            0
#define IsLeapYear(yr) (!((yr) % 400) || (((yr) % 100) && !((yr) % 4)))
typedef struct
{
    uint16_t year;    // 2000+
    uint8_t month;    // 1-12
    uint8_t day;      // 1-31
    uint8_t seconds;  // 0-59
    uint8_t minutes;  // 0-59
    uint8_t hour;     // 0-23
} UTCTimeStruct;
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
void calcRTC(void);
void HandleRTCEvent(T_IO_MSG io_msg);
void Time_init(void);
uint8_t month_length_calc(uint8_t lpyr, uint8_t mon);
bool app_send_msg_to_apptask(T_IO_MSG *p_msg);
#ifdef __cplusplus
}
#endif

#endif
