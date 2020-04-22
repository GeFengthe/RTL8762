/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      switch_swtimer.h
* @brief     header file of software timer implementation
* @details
* @author    elliot chen
* @date      2018-09-17
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _SWITCH_SWTIMER_
#define _SWITCH_SWTIMER_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "platform_os.h"
#include "app_msg.h"
#include "app_task.h"
#include "mesh_beacon.h"
#include "gap_scheduler.h"
#include "switch_dlps_ctrl.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#define UNPROV_TIME_OUT                     (10*60*1000)
#define CHANGE_SCAN_PARAM_TIME_OUT          (10*1000)

typedef enum
{
    UNPROV_TIMEOUT,
    PROV_SUCCESS_TIMEOUT
} SW_TIMER_MSG_TYPE;

void unprov_timer_init(void);
void unprov_timer_start(void);
void unprov_timer_stop(void);
void change_scan_param_timer_init(void);
void change_scan_param_timer_start(void);
void switch_handle_sw_timer_msg(T_IO_MSG *io_msg);
void power_on_detect_timer_init(void);

#ifdef __cplusplus
}
#endif

#endif /*_SWITCH_SWTIMER_*/

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
