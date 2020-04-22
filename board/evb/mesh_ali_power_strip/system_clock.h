/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      system_clock.h
* @brief     Head file of all MACRO and functions used to compute system time
* @details
* @author    Astor
* @date      2019-04-26
* @version   v1.0
* *********************************************************************************************************
*/

#include "stdint.h"

#define DAY                         86400
#define HOUR                        3600
#define MINUTE                        60

#define BASE_YEAR                     2019
#define BASE_MONTH                    1
#define BASE_DAY                    1
#define BASE_HOUR                     0
#define BASE_MIN                    0
#define BASE_SEC                    0

#define BASE_UNIX_TIME                    0x5C2A3D00

typedef enum
{
    JAN = 31,
    FEB = 28,
    MAR = 31,
    APR = 30,
    MAY = 31,
    JUN = 30,
    JUL = 31,
    AUG = 31,
    SEP = 30,
    OCT = 31,
    NOV = 30,
    DEC = 31,
} enum_month_t;

typedef enum
{
    MON = 0x01,
    TUE = 0x02,
    WED = 0x04,
    THU = 0x08,
    FRI = 0x10,
    SAT = 0x20,
    SUN = 0x40,
} enum_weekday_t;

typedef struct
{
    volatile uint16_t year;    // 2019+
    volatile uint8_t month;    // 0-11
    volatile uint8_t day;      // 0-30
    volatile uint8_t seconds;  // 0-59
    volatile uint8_t minutes;  // 0-59
    volatile uint8_t hour;     // 0-23
    volatile uint8_t weekday;
} UTCTimeStruct;

extern UTCTimeStruct local_time;
extern uint32_t sys_clk_update;

void unix2UTC(uint32_t unix_time);
