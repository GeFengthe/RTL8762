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
#include "calc.h"
#include "rtl876x_gpio.h"
#include "rtl876x_RTC.h"
#include "board.h"
#include "mem_types.h"
#include "os_mem.h"
/*============================================================================*
 *                              Variables
 *============================================================================*/
UTCTimeStruct Global_Time;
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
    case IO_MSG_TYPE_TIMER:
        {
            HandleRTCEvent(io_msg);
        }
        break;
    default:
        break;
    }
}

/**
 * @brief    HandleButtonEvent for RTC event
 * @note     Event for keyscan.
 * @param[in] io_msg
 * @return   void
 */
void HandleRTCEvent(T_IO_MSG io_msg)
{
    uint8_t keytype = io_msg.subtype ;

    if (keytype == IO_MSG_RTC_CLOCK)
    {
        calcRTC();
    }
}
void calcRTC()
{
    uint8_t leapYear = IsLeapYear(Global_Time.year);
    Global_Time.seconds++;
    if (Global_Time.seconds >= 60)
    {
        Global_Time.seconds = 0;
        Global_Time.minutes++;
    }
    if (Global_Time.minutes >= 60)
    {
        Global_Time.minutes = 0;
        Global_Time.hour++;
    }
    if (Global_Time.hour >= 24)
    {
        Global_Time.hour = 0;
        Global_Time.day++;
    }
    if (Global_Time.day > month_length_calc(leapYear, Global_Time.month))
    {
        Global_Time.day = 1;
        Global_Time.month++;
    }
    if (Global_Time.month > 12)
    {
        Global_Time.month = 1;
        Global_Time.year++;
    }
    APP_PRINT_INFO6("%d/%d/%d (%d:%d:%d)",
                    Global_Time.year, Global_Time.month, Global_Time.day,
                    Global_Time.hour, Global_Time.minutes, Global_Time.seconds);
}
uint8_t month_length_calc(uint8_t lpyr, uint8_t mon)
{
    uint8_t days = 31;

    if (mon == 2)   // feb
    {
        days = (28 + lpyr);
    }
    else
    {
        if (mon > 7)   // aug-dec
        {
            mon--;
        }

        if ((mon & 1) == 0)
        {
            days = 30;
        }
    }
    return (days);
}
void Time_init()
{
    Global_Time.year = 2000;
    Global_Time.month = 1;
    Global_Time.day = 1;
    Global_Time.hour = 0;
    Global_Time.minutes = 0;
    Global_Time.seconds = 0;
}
/**
  * @brief  RTC interrupt handle function.
  * @param  None.
  * @return None.
  */
void RTC_Handler(void)
{
    uint32_t CompareValue;
    T_IO_MSG rtc_clock_msg;
    /* RTC overflow interrupt handle */
    if (RTC_GetINTStatus(RTC_INT_CMP0) == SET)
    {
        CompareValue = RTC_GetCounter() + (32000 / (RTC_PRESCALER_VALUE + 1));
        RTC_SetComp(ClOCK_RTC_COMPARATOR, CompareValue & 0xFFFFFF);
        rtc_clock_msg.type = IO_MSG_TYPE_TIMER;
        rtc_clock_msg.subtype = IO_MSG_RTC_CLOCK;
        app_send_msg_to_apptask(&rtc_clock_msg);
        // Add application code here
        RTC_ClearCompINT(ClOCK_RTC_COMPARATOR);
    }
}
