/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rcu_swtimer.h
* @brief     header file of software timer implementation
* @details
* @author    elliot chen
* @date      2018-11-06
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _RCU_SWTIMER_
#define _RCU_SWTIMER_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "platform_os.h"
#include "app_msg.h"
#include "app_task.h"
#include "trace.h"
#include "keyscan_driver.h"
#include "key_handle.h"

void sw_timer_init(void);

#ifdef __cplusplus
}
#endif

#endif /*_RCU_SWTIMER_*/

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
