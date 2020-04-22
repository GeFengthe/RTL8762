/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
* @file      app_task.h
* @brief     Routines to create App task and handle events & messages
* @author    jane
* @date      2017-06-02
* @version   v1.0
**************************************************************************************
* @attention
* <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
**************************************************************************************
*/
#ifndef _APP_TASK_H_
#define _APP_TASK_H_

#include <string.h>

#include "os_msg.h"
#include "os_queue.h"
#include "os_sched.h"
#include "os_task.h"
#include "os_timer.h"

#include "app_msg.h"

#include "trace.h"

//#include "io_gpio.h"


/**
  * @brief      Send msg to app task.
  * @param[in]  p_handle: The handle to the message queue being peeked.
  * @return     The status of the message queue peek.
  * @retval     true:Message queue was peeked successfully.
  * @retval     false:Message queue was failed to peek.
  */
bool app_send_msg_to_apptask(T_IO_MSG *p_msg);

/**
  * @brief  Initialize app task
  * @return void
  */
void app_task_init(void);

#endif

