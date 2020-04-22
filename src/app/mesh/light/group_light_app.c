/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     group_light_app.c
  * @brief    Source file for group light.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2018-11-09
  * @version  v1.0
  * *************************************************************************************
  */

/* Add Includes here */
#include <string.h>
#include "platform_os.h"
#include "group_light_app.h"
#include "light_cwrgb_app.h"
#include "light_storage_app.h"
#include "light_controller_app.h"

#define GROUP_LIGHT_TIMER_CFG_TO_PERIOD         5000 //!< ms
#define GROUP_LIGHT_TIMER_GOOD_NIGHT_PERIOD     60000 //!< ms

/** 0:CWRGB, 2:RGB */
#define GENERIC_ON_OFF_OFFSET                   0

plt_timer_t group_light_timer;
bool group_light_timer_type;

void group_light_timeout_cb(void *timer)
{
    plt_timer_delete(group_light_timer, 0);
    group_light_timer = NULL;
    group_receiver_state_set(GROUP_RECEIVER_STATE_NORMAL);
    if (group_light_timer_type)
    {
        bool generic_on_off = GROUP_CTL_OFF;
        if ((light_get_cold()->lightness) ||
            (light_get_warm()->lightness))
        {
            generic_on_off = GROUP_CTL_ON;
        }

        if (generic_on_off == GROUP_CTL_ON)
        {
            light_cw_turn_on();
            light_state_store();
        }
    }
}

void group_light_receive_ctl_msg(T_LE_SCAN_INFO *ple_scan_info, uint8_t *pdata, uint8_t len)
{
    group_ctl_t *pmsg = (group_ctl_t *)pdata;
    switch (pmsg->opcode)
    {
    case GROUP_CTL_OPCODE_ON_OFF:
        {
            bool generic_on_off = GROUP_CTL_OFF;
            if ((light_get_cold()->lightness) ||
                (light_get_warm()->lightness))
            {
                generic_on_off = GROUP_CTL_ON;
            }

            if (pmsg->on_off == GROUP_CTL_OFF && generic_on_off == GROUP_CTL_ON)
            {
                light_cw_turn_off();
            }
            else if (pmsg->on_off == GROUP_CTL_ON && generic_on_off == GROUP_CTL_OFF)
            {
                light_cw_turn_on();
                light_state_store();
            }
        }
        break;
    case GROUP_CTL_OPCODE_LIGHTNESS:
        {
            light_ctl_t ctl = light_get_ctl();
            int32_t lightness = ctl.lightness + pmsg->lightness;
            if (lightness < 0)
            {
                ctl.lightness = 0;
            }
            else if (lightness > 0xffff)
            {
                ctl.lightness = 0xffff;
            }
            else
            {
                ctl.lightness = lightness;
            }
            light_set_ctl(ctl);
            light_state_store();
        }
        break;
    case GROUP_CTL_OPCODE_TEMPERATURE:
        {
            light_ctl_t ctl = light_get_ctl();
            int32_t temperature = ctl.temperature + pmsg->temperature;
            if (temperature < 0x0320)
            {
                ctl.temperature = 0x0320;
            }
            else if (temperature > 0x4E20)
            {
                ctl.temperature = 0x4E20;
            }
            else
            {
                ctl.temperature = temperature;
            }
            light_set_ctl(ctl);
            light_state_store();
        }
        break;
    case GROUP_CTL_OPCODE_NIGHT_LIGHT:
        {
            light_ctl_t ctl = light_get_ctl();
            ctl.lightness = 0x3ff;
            light_set_ctl(ctl);
            light_state_store();
        }
        break;
    case GROUP_CTL_OPCODE_GOOD_NIGHT:
        {
            if (group_light_timer)
            {
                plt_timer_change_period(group_light_timer, GROUP_LIGHT_TIMER_GOOD_NIGHT_PERIOD, 0);
            }
            else
            {
                group_light_timer = plt_timer_create("gl", GROUP_LIGHT_TIMER_GOOD_NIGHT_PERIOD, 0, 0,
                                                     group_light_timeout_cb);
                if (group_light_timer)
                {
                    plt_timer_start(group_light_timer, 0);
                }
            }
            group_light_timer_type = true;
            light_rgb_t rgb = {0, 0, 0};
            light_set_rgb_lightness(rgb);
            light_breath(light_get_cold(), 0, 0xffff, 1000, 50, 3, TRUE, NULL);
            light_breath(light_get_warm(), 0, 0xffff, 1000, 50, 3, TRUE, NULL);
        }
        break;
    default:
        break;
    }
}

void group_light_receive_cfg_msg(T_LE_SCAN_INFO *ple_scan_info, uint8_t *pdata, uint8_t len)
{
    light_rgb_t rgb = {0, 0, 0};
    light_set_rgb_lightness(rgb);
    light_breath(light_get_cold(), 0, 0xffff, 1000, 50, 3, TRUE, NULL);
    light_breath(light_get_warm(), 0, 0xffff, 1000, 50, 3, TRUE, NULL);
}

void group_light_receive_sync_msg(T_LE_SCAN_INFO *ple_scan_info, uint8_t *pdata, uint8_t len)
{
    group_sync_t *pmsg = (group_sync_t *)pdata;
    uint16_t cwrgb[5];
    for (uint8_t channel = 0; channel < 5; channel++)
    {
        cwrgb[channel] = pmsg->sync_data[channel] * 65535 / 255;
    }
    light_set_cwrgb(cwrgb);
    light_state_store();
}

void group_light_sync(void)
{
    group_sync_t sync;
    plt_rand((uint8_t *)&sync.weight, 2);
    light_get_cwrgb(sync.sync_data);
    group_receiver_sync(&sync, 7);
}

void group_light_init(void)
{
    group_receiver_init();
    group_receiver_state_set(GROUP_RECEIVER_STATE_CFG);
    group_receiver_reg_cb(GROUP_MSG_TYPE_CFG, group_light_receive_cfg_msg);
    group_receiver_reg_cb(GROUP_MSG_TYPE_CTL, group_light_receive_ctl_msg);
    group_receiver_reg_cb(GROUP_MSG_TYPE_SYNC, group_light_receive_sync_msg);
    group_light_timer = plt_timer_create("gl", GROUP_LIGHT_TIMER_CFG_TO_PERIOD, 0, 0,
                                         group_light_timeout_cb);
    plt_timer_start(group_light_timer, 0);
}
