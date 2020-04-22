/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      switch_dlps_ctrl.h
* @brief
* @details
* @author    Elliot Chen
* @date      2018-9-13
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _SWITCH_DLPS_CTRL_
#define _SWITCH_DLPS_CTRL_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"

typedef union
{
    uint32_t dlps_ctrl;
    struct
    {
        uint32_t io_dlps_bit: 1;
        uint32_t unprov_dlps_bit: 1;
        uint32_t rsvd: 30;
    } dlps_bit;
} DLPS_Ctrl_Status;

void switch_dlps_ctrl_init(void);
void switch_io_ctrl_dlps(bool allow_enter_dlps);
void switch_unprov_ctrl_dlps(bool allow_enter_dlps);
bool switch_check_dlps(void);

#ifdef __cplusplus
}
#endif

#endif

/* _SWITCH_DLPS_CTRL_ */

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

