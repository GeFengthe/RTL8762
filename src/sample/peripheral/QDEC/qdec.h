/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      mouse_qdecoder.h
* @brief
* @details
* @author    parker
* @date      2018-4-28
* @version   v1.0
* *********************************************************************************************************
*/


#ifndef _MOUSE_QDECODER_H
#define _MOUSE_QDECODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rtl876x.h>
#include <app_msg.h>


typedef struct _qdec_ctx_t
{
    int16_t pre_ct;     //previous counter value
    int16_t cur_ct;     //current counter value
    uint16_t dir;        //1--up; 0-- down
} qdec_ctx_t;

extern bool allowed_qdecoder;


//export functions
void qdecoder_pinmux_config(void);
void qdecoder_pad_config(void);
void qdecoder_pad_enter_dlps_config(void);
void qdecoder_pad_exit_dlps_config(void);
void qdecoder_init_status_read(void);
void driver_qdec_init(void);
void qdecoder_dlps_timer_init(void);
void handle_qdecoder_wakeup(void);
void handle_qdecoder_event(T_IO_MSG qdecoder_msg);
bool app_send_msg_to_apptask(T_IO_MSG *p_msg);
#ifdef __cplusplus
}
#endif

#endif /* _MOUSE_QDECODER_H */
