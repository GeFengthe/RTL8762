/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     app_timer.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __APP_TIMER_H
#define __APP_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif


#include <string.h>

//#include "os_msg.h"
//#include "os_queue.h"
//#include "os_sched.h"
//#include "os_task.h"
#include "os_timer.h"

#include "app_msg.h"

#include "trace.h"

#include "io_keyscan.h"

//void timer_keyscan_callback(void *xTimer);
void sw_timer_init(void);

#ifdef __cplusplus
}
#endif

#endif
