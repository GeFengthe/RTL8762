/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      switch_swtimer.c
* @brief     source file of software timer implementation
* @details
* @author    elliot chen
* @date      2018-09-17
* @version   v1.0
* *********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "switch_swtimer.h"

/* Globals ------------------------------------------------------------------*/
static plt_timer_t unprov_timer = NULL;
static plt_timer_t change_scan_param_timer = NULL;

static void unprov_timeout_cb(void *ptimer)
{
    T_IO_MSG unprov_timeout_msg;
    unprov_timeout_msg.type     = IO_MSG_TYPE_TIMER;
    unprov_timeout_msg.subtype  = UNPROV_TIMEOUT;
    app_send_msg_to_apptask(&unprov_timeout_msg);
}

void unprov_timer_init(void)
{
    if (unprov_timer == NULL)
    {
        unprov_timer = plt_timer_create("unprov calc", UNPROV_TIME_OUT, false, 0, unprov_timeout_cb);
    }
    unprov_timer_start();
}

void unprov_timer_start(void)
{
    if (unprov_timer)
    {
        plt_timer_start(unprov_timer, 0);
        switch_unprov_ctrl_dlps(false);
    }
    else
    {
        APP_PRINT_INFO0("switch_swtimer->unprov_timer_start failure!");
    }
}
void unprov_timer_stop(void)
{
    if (unprov_timer)
    {
        plt_timer_stop(unprov_timer, 0);
        switch_unprov_ctrl_dlps(true);
    }
    else
    {
        APP_PRINT_INFO0("switch_swtimer->unprov_timer_stop failure!");
    }
}

static void change_scan_param_timeout_cb(void *ptimer)
{
    T_IO_MSG unprov_timeout_msg;
    unprov_timeout_msg.type     = IO_MSG_TYPE_TIMER;
    unprov_timeout_msg.subtype  = PROV_SUCCESS_TIMEOUT;
    app_send_msg_to_apptask(&unprov_timeout_msg);
}

void change_scan_param_timer_init(void)
{
    if (change_scan_param_timer == NULL)
    {
        change_scan_param_timer = plt_timer_create("change scan param", CHANGE_SCAN_PARAM_TIME_OUT, false,
                                                   0, change_scan_param_timeout_cb);
    }
    change_scan_param_timer_start();
}

void change_scan_param_timer_start(void)
{
    if (change_scan_param_timer)
    {
        plt_timer_start(change_scan_param_timer, 0);
    }
    else
    {
        APP_PRINT_INFO0("switch_swtimer->change_scan_param_timer_start failure!");
    }
}

void switch_handle_sw_timer_msg(T_IO_MSG *io_msg)
{
    switch (io_msg->subtype)
    {
    case UNPROV_TIMEOUT:
        {
            beacon_stop();
            switch_unprov_ctrl_dlps(true);
            break;
        }
    case PROV_SUCCESS_TIMEOUT:
        {
            uint16_t scan_interval = 0x320; //!< 500ms
            uint16_t scan_window = 0x30; //!< 30ms
            gap_sched_params_set(GAP_SCHED_PARAMS_SCAN_INTERVAL, &scan_interval, sizeof(scan_interval));
            gap_sched_params_set(GAP_SCHED_PARAMS_SCAN_WINDOW, &scan_window, sizeof(scan_window));
            break;
        }
    default:
        {
            break;
        }
    }
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

