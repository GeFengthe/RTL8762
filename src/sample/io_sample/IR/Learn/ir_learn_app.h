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
#include "app_msg.h"

#include "ir_learn.h"
#include "ir_learn_config.h"

void ir_learn_timer_init(void);
void ir_learn_module_init(void);
bool ir_learn_handle_msg(T_IO_MSG *io_ir_msg);

#ifdef __cplusplus
}
#endif

#endif /*__IR_LEARN_APP_H_*/

/******************* (C) COPYRIGHT 2018Realtek Semiconductor Corporation *****END OF FILE****/

