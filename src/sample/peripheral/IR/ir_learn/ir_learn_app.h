/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file         ir_learn_app.h
* @brief
* @details
* @author       elliot chen
* @date         2018-7-24
* @version      v1.0
* *********************************************************************************************************
*/

#ifndef __IR_LEARN_APP_H_
#define __IR_LEARN_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "ir_learn_config.h"
#include "app_section.h"
#include "mem_config.h"
#include "ir_learn.h"
#include "app_msg.h"
#include "trace.h"
#include "app_msg.h"
#include "os_timer.h"

void ir_learn_init_timer(void);
void ir_learn_module_init(void);
bool ir_learn_msg_proc(uint16_t msg_sub_type);

#ifdef __cplusplus
}
#endif

#endif /*__IR_LEARN_APP_H_*/

/******************* (C) COPYRIGHT 2018Realtek Semiconductor Corporation *****END OF FILE****/

