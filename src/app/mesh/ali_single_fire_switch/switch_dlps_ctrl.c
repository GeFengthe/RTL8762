/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     switch_dps_check.c
* @brief
* @details
* @author   Elliot Chen
* @date     2018-9-13
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "switch_dlps_ctrl.h"
#include "string.h"

/* Globals ------------------------------------------------------------------*/
DLPS_Ctrl_Status switch_dlps_ctrl;

void switch_dlps_ctrl_init(void)
{
    memset(&switch_dlps_ctrl, 0, sizeof(DLPS_Ctrl_Status));
}

void switch_io_ctrl_dlps(bool allow_enter_dlps)
{
    if (!allow_enter_dlps)
    {
        switch_dlps_ctrl.dlps_bit.io_dlps_bit = 1;
    }
    else
    {
        switch_dlps_ctrl.dlps_bit.io_dlps_bit = 0;
    }
}

void switch_unprov_ctrl_dlps(bool allow_enter_dlps)
{
    if (!allow_enter_dlps)
    {
        switch_dlps_ctrl.dlps_bit.unprov_dlps_bit = 1;
    }
    else
    {
        switch_dlps_ctrl.dlps_bit.unprov_dlps_bit = 0;
    }
}

bool switch_check_dlps(void)
{
    if (switch_dlps_ctrl.dlps_ctrl == 0)
    {
        return true;
    }
    return false;
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
