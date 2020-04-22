/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     light_dlps_ctrl.c
* @brief
* @details
* @author   Elliot Chen
* @date     2018-11-27
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "light_dlps_ctrl.h"
#include "string.h"

#if LIGHT_DLPS_EN
/* Globals ------------------------------------------------------------------*/
DLPS_Ctrl_Status light_dlps_ctrl;

void light_dlps_ctrl_init(void)
{
    memset(&light_dlps_ctrl, 0, sizeof(DLPS_Ctrl_Status));
}

void light_io_ctrl_dlps(bool allow_enter_dlps)
{
    if (!allow_enter_dlps)
    {
        light_dlps_ctrl.dlps_bit.io_dlps_bit = 1;
    }
    else
    {
        light_dlps_ctrl.dlps_bit.io_dlps_bit = 0;
    }
}

void light_unprov_ctrl_dlps(bool allow_enter_dlps)
{
    if (!allow_enter_dlps)
    {
        light_dlps_ctrl.dlps_bit.unprov_dlps_bit = 1;
    }
    else
    {
        light_dlps_ctrl.dlps_bit.unprov_dlps_bit = 0;
    }
}

bool light_check_dlps(void)
{
    if (light_dlps_ctrl.dlps_ctrl == 0)
    {
        return true;
    }
    return false;
}
#endif
/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
