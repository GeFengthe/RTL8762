/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      switch_flash_mgr.c
* @brief     source file of flash management implementation
* @details
* @author    elliot chen
* @date      2018-09-17
* @version   v1.0
* *********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "switch_flash_mgr.h"
#include "switch_swtimer.h"

/* Globals ------------------------------------------------------------------*/
/* light factory restore */
uint8_t power_on_count = 0;
static plt_timer_t power_on_detect_timer;

void power_on_detect_timeout_cb(void *timer)
{
    plt_timer_delete(power_on_detect_timer, 0);
    power_on_detect_timer = NULL;
    switch_flash_store(FLASH_SWITCH_PARAM_TYPE_POWER_ON_COUNT, false);
}

void power_on_detect_timer_init(void)
{
    power_on_detect_timer = plt_timer_create("PO", SWITCH_POWER_ON_TIME_OUT, 0, 0,
                                             power_on_detect_timeout_cb);

    if (power_on_detect_timer)
    {
        plt_timer_start(power_on_detect_timer, 0);
    }
    else
    {
        APP_PRINT_INFO0("switch_swtimer->power_on_detect_timer_init failure!");
    }
}

void switch_flash_store(flash_switch_param_type_t type, uint8_t used)
{
    uint32_t ret;
    switch (type)
    {
    case FLASH_SWITCH_PARAM_TYPE_SWITCH_STATE:
        {
            break;
        }
    case FLASH_SWITCH_PARAM_TYPE_POWER_ON_COUNT:
        {
            flash_switch_power_on_count_t flash_switch_power_on_count;
            flash_switch_power_on_count.count = power_on_count;
            flash_switch_power_on_count.used = used;
            ret = ftl_save((void *)&flash_switch_power_on_count,
                           FLASH_PARAMS_APP_SWITCH_OFFSET + MEMBER_OFFSET(flash_switch_param_t, power_on_count),
                           sizeof(flash_switch_power_on_count_t));
            break;
        }
    default:
        {
            ret = 1;
            break;
        }
    }

    if (ret != 0)
    {
        APP_PRINT_ERROR2("switch_flash_mgr->flash store failed, type = %d, cause = %d", type, ret);
    }
}

bool switch_flash_restore(void)
{
    uint32_t ret;
    flash_switch_power_on_count_t flash_switch_power_on_count;
    ret = ftl_load((void *)&flash_switch_power_on_count,
                   FLASH_PARAMS_APP_SWITCH_OFFSET + + MEMBER_OFFSET(flash_switch_param_t, power_on_count),
                   sizeof(flash_switch_power_on_count_t));
    if ((ret == 0) && (TRUE == flash_switch_power_on_count.used))
    {
        power_on_count = flash_switch_power_on_count.count;
    }

    /* restore factory setting */
    if (power_on_count >= SWITCH_POWER_ON_COUNT)
    {
        power_on_count = 0;
        switch_flash_store(FLASH_SWITCH_PARAM_TYPE_POWER_ON_COUNT, false);
        mesh_node_clear();
        return true;
    }
    else
    {
        power_on_count++;
        switch_flash_store(FLASH_SWITCH_PARAM_TYPE_POWER_ON_COUNT, true);
        power_on_detect_timer_init();

        return false;
    }
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

