/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rcu_swtimer.c
* @brief     source file of software timer implementation
* @details
* @author    elliot chen
* @date      2018-11-06
* @version   v1.0
* *********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rcu_swtimer.h"

/* Globals ------------------------------------------------------------------*/

void sw_timer_init(void)
{
    keyscan_init_timer();
    keyscan_long_press_detect_init_timer();
    keyscan_repeat_send_timer_init_timer();
#ifdef COMBINE_KEY_SCENARIO
    key_handle_comb_keys_init_timer();
#endif
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

