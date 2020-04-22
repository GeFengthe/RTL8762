/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      strip_timer.c
* @brief     System time update & timer init
* @details
* @author    Astor
* @date      2019-04-26
* @version   v1.0
* *********************************************************************************************************
*/
// reference: https://www.aligenie.com/doc/357554/iv5it7
#include "vendor_timer.h"
#include "trace.h"
#include "ali_ctl_light_app.h"
#include "rtl876x_rtc.h"
#include "system_clock.h"
#include "rtl876x_nvic.h"
#include "app_msg.h"
#include "app_task.h"

#define FLASH_SAVE_ADDR 0x00830000
#define RTC_PRESCALER (3200-1)
#define RTC_COMP_INDEX          1
#define RTC_INT_CMP_NUM         RTC_INT_CMP1
//
#define RTC_COMP_VALUE          (600)

UTCTimeStruct local_time;
timer_data_t timer_list[40];

void *xTimerUpdate;
void *xTimerReport;

/**
 * @brief callback function of timer to requeset local time update
*/
void vTimerUpdateCallback(void *pxTimer)
{
    temp_tid++;
    if (temp_tid == 192)
    {
        temp_tid = 0x80;
    }
    send_update_request();
}


/**
 * @brief callback function of timer to publish current device state
*/
void vTimerReportCallback(void *pxTimer)
{
    T_IO_MSG bee_io_msg;
    bee_io_msg.type = IO_MSG_TYPE_GPIO;
    bee_io_msg.subtype = generic_data_current.generic_on_off;
    app_send_msg_to_apptask(&bee_io_msg);
}

void swTimerInit()
{
    bool retval ;


    retval = os_timer_create(&xTimerUpdate, "xTimerUpdate", 1, 10000, false, vTimerUpdateCallback);

    if (retval)
    {
        os_timer_start(&xTimerUpdate);
    }
    else
    {
        APP_PRINT_INFO1("xTimerUpdate retval is %d", retval);
    }

    retval = os_timer_create(&xTimerReport, "xTimerReport", 1, 180000, true, vTimerReportCallback);
    if (!retval)
    {
        APP_PRINT_INFO1("xTimerReport retval is %d", retval);
    }
    else
    {
        os_timer_start(&xTimerReport);
    }
}

/**
 * @brief determine if the very year is leap year
 * @return boolean: if the very year is leap year
*/
bool is_leap_year(uint16_t year)
{
    if (((year % 4) == 0) && ((year % 100) != 0))
    {
        return true;
    }
    else if ((year % 400) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief erase flash with specific type and address
 * @param unix_time  latest received system time
 * @note after received a new system time, all timers will also be updated
*/
void unix2UTC(uint32_t unix_time)
{
    RTC_RunCmd(DISABLE);
    RTC_ResetCounter();
    RTC_init();
    uint8_t month_days[12] = {JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC};
    uint32_t sec_diff = unix_time - BASE_UNIX_TIME;
    uint16_t day_diff = sec_diff / DAY + BASE_DAY;
    local_time.weekday = day_diff % 7;
    sec_diff = sec_diff % DAY;
    local_time.hour = sec_diff / HOUR;
    sec_diff = sec_diff % HOUR;
    local_time.minutes = sec_diff / MINU;
    local_time.seconds = sec_diff = sec_diff % MINU;
    local_time.year = BASE_YEAR;
    uint8_t year_diff = day_diff / 365;
    for (int i = 2019; i < year_diff + 2019; i++)
    {
        if (is_leap_year(i))
        {
            day_diff = day_diff - 366;
        }
        else
        {
            day_diff = day_diff - 365;
        }
        local_time.year++;
    }
    if (is_leap_year(local_time.year))
    {
        month_days[1] = 29;
    }
    local_time.month = BASE_MONTH;
    uint8_t cnt = 1;
    while (day_diff > 0)
    {
        if ((day_diff) <= month_days[cnt - 1])
        {
            local_time.month = cnt;
            local_time.day = day_diff;
            day_diff = 0;
        }
        else
        {
            day_diff -= month_days[cnt - 1];
            cnt = (cnt + 1) % 12;
        }
    }
    check_timer_valid();
    RTC_RunCmd(ENABLE);
}

/**
 * @brief load/initialize timer list from flash
*/
void timer_list_init(void)
{
//      for (int i = 0; i <= TIMER_MAXIMUM; i++)
//      {
//              timer_list[i].index = 0xFF;
//              timer_list[i].unix_time = 0xFFFFFFFF;
//              timer_list[i].on_off = GENERIC_OFF;
//      }
    flash_read_locked(FLASH_SAVE_ADDR, sizeof(timer_data_t) * 40, (uint8_t *) timer_list);
}

/**
 * @brief sort the timer list(ascending sort)
 * @note each time this function is invoked, timer list will be rewrited into falsh
*/
void timer_sort(void)
{
    uint8_t i, j, min;
    timer_data_t temp_timer;
    for (i = 0; i < TIMER_MAXIMUM; i++)
    {
        min = i;
        for (j = i + 1; j < TIMER_MAXIMUM; j++)
        {
            if (timer_list[min].unix_time > timer_list[j].unix_time)
            {
                min = j;
            }
        }
        if (min != i)
        {
            temp_timer = timer_list[i];
            timer_list[i] = timer_list[min];
            timer_list[min] = temp_timer;
        }
    }
    if (!flash_erase_locked(FLASH_ERASE_SECTOR, 0x00830000))
    {
        DBG_DIRECT("Erase failed");
    }
    flash_auto_write_buffer_locked(FLASH_SAVE_ADDR, (uint32_t *)timer_list, sizeof(timer_data_t) * 40);
}

/**
 * @brief add the timer to timer list
 * @param time_data  the new timer to be added
 * @return error type of adding operation
 * @note timer will not be added successfully if:
                 1. Unix time of this timer is smaller than unix timer of system
                 2. Index of timer is already existed in list
                 3. Timer list is full
*/
ERROR_TYPE is_in_list(timer_data_t time_data)
{
    if (time_data.unix_time < generic_data_current.unix_time)
    {
        return NOT_REACHABLE_TIME;
    }
    if (generic_data_current.unix_time == 0xFFFFFFFF)
    {
        return UNIX_TIME_NOT_SET;
    }

    for (int i = 0; i < 40; i++)
    {
        if (timer_list[i].unix_time == time_data.unix_time && timer_list[i].on_off == time_data.on_off)
        {
            return NOT_REACHABLE_TIME;
        }
        else if (timer_list[i].unix_time == time_data.unix_time && timer_list[i].on_off != time_data.on_off)
        {
            timer_list[i] = time_data;
            if (!flash_erase_locked(FLASH_ERASE_SECTOR, 0x00830000))
            {
                DBG_DIRECT("Erase failed");
            }
            flash_auto_write_buffer_locked(FLASH_SAVE_ADDR, (uint32_t *)timer_list, sizeof(timer_data_t) * 40);
            return ADD_SUCCESS;
        }
        if (timer_list[i].unix_time == 0xFFFFFFFF && timer_list[i].index == 0xFF)
        {
            timer_list[i].index = time_data.index;
            timer_list[i].unix_time = time_data.unix_time;
            timer_list[i].on_off = time_data.on_off;
            timer_sort();
            return ADD_SUCCESS;
        }
    }
    return TIMER_LIST_FULL;
}

/**
 * @brief confirm the new timer is valid
 * @note timer will not be added successfully if:
                 1. Unix time of this timer is smaller than unix timer of system
                 2. Index of timer is already existed in list
                 3. Timer list is full
*/
void check_timer_valid(void)
{
    bool timer_invalid = false;
    for (int i = 0; i < TIMER_MAXIMUM; i++)
    {
        if (timer_list[i].unix_time <= generic_data_current.unix_time)
        {
            timer_list[i].index = 0xFF;
            timer_list[i].unix_time = 0xFFFFFFFF;
            timer_list[i].on_off = GENERIC_OFF;
            timer_invalid = true;
        }
    }
    if (timer_invalid)
    {
        if (!flash_erase_locked(FLASH_ERASE_SECTOR, 0x00830000))
        {
            DBG_DIRECT("Erase failed");
        }
        flash_auto_write_buffer_locked(FLASH_SAVE_ADDR, (uint32_t *)timer_list, sizeof(timer_data_t) * 40);
    }
}

/**
 * @brief delete timer with specified index
 * @param index  index of timer
*/
void remove_timer(uint8_t index)
{
    for (int i = 0; i < TIMER_MAXIMUM; i++)
    {
        if (timer_list[i].index == index)
        {
            timer_list[i].index = 0xFF;
            timer_list[i].on_off = GENERIC_OFF;
            timer_list[i].unix_time = 0xFFFFFFFF;
            timer_sort();
            return;
        }
    }
}

void RTC_init(void)
{

    RTC_SetPrescaler(RTC_PRESCALER);

    RTC_SetComp(RTC_COMP_INDEX, RTC_COMP_VALUE);
    RTC_MaskINTConfig(RTC_INT_CMP_NUM, DISABLE);
    RTC_CompINTConfig(RTC_INT_CMP_NUM, ENABLE);

    /* Config RTC interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /* Start RTC */
    //RTC_RunCmd(ENABLE);
}

/**
 * @brief Handle RTC interrupt
 * @note RTC counter will be reset at 16777200 to simplify UTC time compute
*/
void RTC_Handler(void)
{
    if (RTC_GetINTStatus(RTC_INT_CMP_NUM) == SET)
    {
        /* Notes: DBG_DIRECT function is only used for debugging demonstrations, not for application projects.*/
        T_IO_MSG minute_msg;
        minute_msg.type = IO_MSG_TYPE_RTC;
        minute_msg.subtype = IO_MSG_TIMER_RWS;
        app_send_msg_to_apptask(&minute_msg);
        if ((RTC_GetCounter() + RTC_COMP_VALUE) >= 16777200)
        {
            RTC_ResetCounter();
            RTC_SetComp(RTC_COMP_INDEX, RTC_COMP_VALUE);
        }
        else
        {
            RTC_SetComp(RTC_COMP_INDEX, RTC_GetCounter() + RTC_COMP_VALUE);
        }
        RTC_ClearCompINT(RTC_COMP_INDEX);

    }
}
