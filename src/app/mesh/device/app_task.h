/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */
#ifndef _APPLICATION_H_
#define _APPLICATION_H_
#include "app_msg.h"
#include "stdbool.h"
/**
 * @brief  Initialize App task
 * @return void
 */
void app_task_init(void);
bool app_send_msg_to_apptask(T_IO_MSG *p_msg);
void uart_init(void);
#endif

