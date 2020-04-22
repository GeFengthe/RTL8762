/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      switch_server_app.c
* @brief     Smart mesh switch server application file
* @details
* @author    elliot chen
* @date      2018-9-14
* @version   v1.0
* *********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "switch_server_app.h"
#include "generic_on_off.h"
#include "generic_default_transition_time.h"
#include "switch_io.h"

/* Defines ------------------------------------------------------------------*/

/* Globals ------------------------------------------------------------------*/
static mesh_model_info_t generic_on_off_server;

static generic_on_off_t generic_on_off_get_switch_status(void)
{
    generic_on_off_t current_on_off = GENERIC_OFF;
    SWITCH_STATUS *p_switch_cur_status = switch_get_status();

    if (p_switch_cur_status->switch_status_bit.left_switch_status_bit)
    {
        current_on_off = GENERIC_ON;
    }
    else
    {
        current_on_off = GENERIC_OFF;
    }

    return current_on_off;
}

static int32_t generic_on_off_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                          void *pargs)
{
    UNUSED(pmodel_info);

    switch (type)
    {
    case GENERIC_ON_OFF_SERVER_GET:
        {
            generic_on_off_server_get_t *pdata = pargs;
            pdata->on_off = generic_on_off_get_switch_status();
        }
        break;
    case GENERIC_ON_OFF_SERVER_SET:
        {
            generic_on_off_server_set_t *pdata = pargs;
            if (pdata->on_off != GENERIC_OFF)
            {
                switch_light_cmd(true);
            }
            else
            {
                switch_light_cmd(false);
            }
        }
        break;
    default:
        break;
    }

    return 0;
}

void switch_server_models_init(void)
{
    generic_on_off_server.model_data_cb = generic_on_off_server_data;
    generic_on_off_server_reg(0, &generic_on_off_server);
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
