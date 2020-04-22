/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      strip_timer.h
* @brief     head file of timers on ali power strip
* @details
* @author    Astor
* @date      2019-04-26
* @version   v1.0
* *********************************************************************************************************
*/

#include "os_timer.h"
#include "generic_on_off.h"

#define TIMER_MAXIMUM    40

typedef enum
{
    TIMER_LIST_FULL = 0x86,
    NOT_REACHABLE_TIME = 0x84,
    UNIX_TIME_NOT_SET = 0x80,
    ADD_SUCCESS = 0x0,
} ERROR_TYPE;

typedef struct
{
    uint8_t index;
    uint32_t unix_time;
    uint16_t attr_type;
    uint8_t on_off;
} timer_data_t;

typedef struct
{
    uint8_t index;
    uint16_t relative_time;
    uint8_t schedule;
    uint16_t attr_type;
    uint8_t on_off;
} periodic_data_t;

void swTimerInit(void);
void RTC_init(void);
void timer_list_init(void);
void period_list_init(void);
ERROR_TYPE is_in_timer_list(timer_data_t time_data);
ERROR_TYPE is_in_period_list(periodic_data_t time_data);
void timer_sort(void);
void check_timer_valid(void);
void remove_timer(uint8_t index);

extern uint8_t temp_tid;
extern timer_data_t timer_list[40];
extern periodic_data_t period_list[40];
