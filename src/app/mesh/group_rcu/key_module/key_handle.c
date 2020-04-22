/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     key_handle.c
* @brief    This is the entry of user code which the key handle module resides in.
* @details
* @author   elliot chen
* @date     2018-11-06
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "key_handle.h"
#include <trace.h>
#include "app_msg.h"
#include "os_timer.h"
#include "rtl876x_wdg.h"
#include "rtl876x_keyscan.h"
#include "battery_driver.h"
#include "group.h"
#if LED_EN
#include "led_driver.h"
#endif

/* Defines ------------------------------------------------------------------*/
#define LIGHTNESS_STAGE        200
#define COLOR_TP_STAGE         1000

/* Local Variables ----------------------------------------------------------*/
bool key_handle_group_ctl_on_off(uint8_t group, void *param);
bool key_handle_group_transmitter_ctl_lightness(uint8_t group, void *param);
bool key_handle_group_transmitter_ctl_temperature(uint8_t group, void *param);

/* Key Mapping Table Definiton */
/* S2  S3   S1  S4
   S5  S6   S7  S8
   S9  S10  S11 S12
   S13 S14*/
static const T_KEY_INDEX_DEF KEY_MAPPING_TABLE[KEYPAD_ROW_SIZE][KEYPAD_COLUMN_SIZE] =
{
    {VK_COLOR_TP_UP,  VK_LIGHT_DOWN,   VK_COLOR_TP_DOWN, VK_LIGHT_UP},
    {VK_GROUP_4_ON,   VK_GROUP_1_ON,   VK_GROUP_3_ON,    VK_GROUP_2_ON},
    {VK_GROUP_4_OFF,  VK_GROUP_1_OFF,  VK_GROUP_3_OFF,   VK_GROUP_2_OFF},
    {VK_POWER_ON,     VK_POWER_OFF,    VK_NC,            VK_NC},
};

static const T_GROUP_CTL GROUP_CTL_func_map[KEY_INDEX_ENUM_GUAID] =
{
    {GROUP_INVALID, 0, NULL},                        /* VK_NC */
    {GROUP_ALL, 1, key_handle_group_ctl_on_off},     /* VK_POWER_ON */
    {GROUP_ALL, 0, key_handle_group_ctl_on_off},     /* VK_POWER_OFF */
    {GROUP_INVALID, LIGHTNESS_STAGE, key_handle_group_transmitter_ctl_lightness},  /* VK_LIGHT_UP */
    {GROUP_INVALID, -LIGHTNESS_STAGE, key_handle_group_transmitter_ctl_lightness}, /* VK_LIGHT_DOWN */
    {GROUP_INVALID, COLOR_TP_STAGE, key_handle_group_transmitter_ctl_temperature}, /* VK_COLOR_TP_UP */
    {GROUP_INVALID, -COLOR_TP_STAGE, key_handle_group_transmitter_ctl_temperature},/* VK_COLOR_TP_DOWN */
    {GROUP_1, 1, key_handle_group_ctl_on_off},       /* VK_GROUP_1_ON */
    {GROUP_1, 0, key_handle_group_ctl_on_off},       /* VK_GROUP_1_OFF */
    {GROUP_2, 1, key_handle_group_ctl_on_off},       /* VK_GROUP_2_ON */
    {GROUP_2, 0, key_handle_group_ctl_on_off},       /* VK_GROUP_2_OFF */
    {GROUP_3, 1, key_handle_group_ctl_on_off},       /* VK_GROUP_3_ON */
    {GROUP_3, 0, key_handle_group_ctl_on_off},       /* VK_GROUP_3_OFF */
    {GROUP_4, 1, key_handle_group_ctl_on_off},       /* VK_GROUP_4_ON */
    {GROUP_4, 0, key_handle_group_ctl_on_off},       /* VK_GROUP_4_OFF */
};

/* Global Variables ------------------------------------------------------------------*/
/* Value to indicate the reconnection key data */
T_KEY_HANDLE_GLOBAL_DATA key_handle_global_data;
plt_timer_t combine_keys_detection_timer = NULL;

bool key_handle_group_ctl_on_off(uint8_t group, void *param)
{
    return group_transmitter_ctl_on_off(group, *((uint8_t *)param));
}

bool key_handle_group_transmitter_ctl_lightness(uint8_t group, void *param)
{
    return group_transmitter_ctl_lightness(group, *((int16_t *)param));
}

bool key_handle_group_transmitter_ctl_temperature(uint8_t group, void *param)
{
    return group_transmitter_ctl_temperature(group, *((int16_t *)param));
}

/**
 * @brief handle one key pressed scenario
 * @param key_index - pressed key index
 * @return none
 * @retval void
 */
static bool key_handle_one_key_scenario(T_KEY_HANDLE_GLOBAL_DATA *p_key_handle_global_data)
{
    uint8_t  group_temp = GROUP_CTL_func_map[p_key_handle_global_data->cur_press_key_index].group;

    /* normal key code */
    if (p_key_handle_global_data->cur_press_key_index != VK_NC)
    {
        if (GROUP_INVALID == group_temp)
        {
            group_temp = GROUP_CTL_func_map[p_key_handle_global_data->last_pressed_key_index].group;
            if (GROUP_INVALID == group_temp)
            {
                APP_PRINT_INFO1("[key_handle_one_key_scenario] invalid group index, curr key index = 0x%x!", \
                                p_key_handle_global_data->cur_press_key_index);
                return false;
            }
        }
        APP_PRINT_INFO2("group index = %d, type = %d", group_temp,
                        GROUP_CTL_func_map[p_key_handle_global_data->cur_press_key_index].type);
        return GROUP_CTL_func_map[p_key_handle_global_data->cur_press_key_index].fncb(\
                                                                                      group_temp, (void *)(&GROUP_CTL_func_map[p_key_handle_global_data->cur_press_key_index].type));
    }
    else
    {
        return false;
    }
}

#ifdef COMBINE_KEY_SCENARIO
/**
 * @brief handle two keys pressed scenario1
 * @param key_index - pressed key index
 * @return none
 * @retval void
 */
static void key_handle_two_keys_scenario(T_KEY_INDEX_DEF key_index_1, T_KEY_INDEX_DEF key_index_2)
{
    switch (key_index_1)
    {
    case VK_LIGHT_UP:
        {
            switch (key_index_2)
            {
            case VK_GROUP_1_ON:
            case VK_GROUP_2_ON:
            case VK_GROUP_3_ON:
            case VK_GROUP_4_ON:
                {
                    key_handle_global_data.combine_keys_status = ADV_COMBINE_KEYS_BIT_MASK;
                    key_handle_global_data.group_key = GROUP_CTL_func_map[key_index_2].group;
                    break;
                }
            default:
                {
                    break;
                }
            }
            break;
        }
    case VK_GROUP_1_ON:
    case VK_GROUP_2_ON:
    case VK_GROUP_3_ON:
    case VK_GROUP_4_ON:
        {
            if (key_index_2 == VK_LIGHT_UP)
            {
                key_handle_global_data.combine_keys_status = ADV_COMBINE_KEYS_BIT_MASK;
                key_handle_global_data.group_key = GROUP_CTL_func_map[key_index_1].group;
            }
            break;
        }
    default:
        {
            break;
        }
    }

    APP_PRINT_INFO1("[key_handle_two_keys_scenario] combine_keys_status is 0x%04X",
                    key_handle_global_data.combine_keys_status);

    if (key_handle_global_data.combine_keys_status != INVALID_COMBINE_KEYS_BIT_MASK)
    {
        /* start combine keys dectecion timer */
        os_timer_restart(&combine_keys_detection_timer, COMBINE_KEYS_DETECT_TIMEOUT);
    }
}

/**
 * @brief    key handle combine keys detected timer callback
 * @param    p_timer - point of timer
 * @return   none
 * @retval   void
 * Caution   do NOT excute time consumption functions in timer callback
 */
static void key_handle_comb_keys_timer_cb(plt_timer_t p_timer)
{
    /* check combine keys status */
    APP_PRINT_INFO1("[key_handle_comb_keys_timer_cb] combine_keys_status is 0x%04X",
                    key_handle_global_data.combine_keys_status);

    if (key_handle_global_data.combine_keys_status == ADV_COMBINE_KEYS_BIT_MASK)
    {
        // Send mesh configuration packet!
        group_transmitter_cfg_group(key_handle_global_data.group_key);
        /* start combine keys repeat send */
        os_timer_restart(&combine_keys_detection_timer, COMBINE_KEYS_REPEAT_SEND_TIMEOUT);
    }
}

/**
 * @brief key handler init timer
 */
void key_handle_comb_keys_init_timer(void)
{
    APP_PRINT_INFO0("[key_handle_comb_keys_init_timer] init timer");

    /* combine_keys_detection_timer is used to detect combine keys after timeout */
    if (false == os_timer_create(&combine_keys_detection_timer, "combine_keys_detection_timer",  1, \
                                 COMBINE_KEYS_DETECT_TIMEOUT, false, key_handle_comb_keys_timer_cb))
    {
        APP_PRINT_ERROR0("[key_handle_comb_keys_init_timer] combine_keys_detection_timer creat failed!");
    }
}
#endif

/*============================================================================*
 *                              Global Functions
 *============================================================================*/
/**
* @brief  Initialize key handle data
*/
void key_handle_init_data(void)
{
    APP_PRINT_INFO0("[key_handle_init_data] init data");
    memset(&key_handle_global_data, 0, sizeof(key_handle_global_data));
}

/**
* @brief   handle key pressed event
* @param   p_keyscan_fifo_data - point of keyscan FIFO data
* @return  void
*/
void key_handle_pressed_event(T_KEYSCAN_FIFIO_DATA *p_keyscan_fifo_data)
{
    APP_PRINT_INFO1("[key_handle_pressed_event] keyscan FIFO length is %d", p_keyscan_fifo_data->len);

    /* LED display */
    LED_ON(LED_1);

    for (uint8_t index = 0; index < (p_keyscan_fifo_data->len); index++)
    {
        APP_PRINT_INFO4("[key_handle_pressed_event] keyscan data[%d]: row - %d, column - %d, value - %d", \
                        index, p_keyscan_fifo_data->key[index].row, p_keyscan_fifo_data->key[index].column,
                        KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[index].row][p_keyscan_fifo_data->key[index].column]);
    }

    T_KEY_INDEX_DEF key_index_1 =
        KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[0].row][p_keyscan_fifo_data->key[0].column];

#ifdef COMBINE_KEY_SCENARIO
    T_KEY_INDEX_DEF key_index_2 =
        KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[1].row][p_keyscan_fifo_data->key[1].column];

    /* check if need to reissue key release event */
    if ((key_handle_global_data.cur_press_key_index != VK_NC) &&
        ((p_keyscan_fifo_data->len != 1) || (key_handle_global_data.cur_press_key_index != key_index_1)))
    {
        key_handle_release_event(p_keyscan_fifo_data);
    }
#endif

#ifdef BAT_EN
    if (LOW_POWER_MODE == rcu_get_bat_mode())
    {
        /* Update battery status */
        rcu_bat_handle(IO_MSG_TYPE_BAT_DETECT);

        /* Check current battery status */
        if (LOW_POWER_MODE == rcu_get_bat_mode())
        {
            return;
        }
    }
#endif

    if (p_keyscan_fifo_data->len == 0)
    {
        APP_PRINT_WARN0("[key_handle_pressed_event] FIFO length is 0!");
    }
    else if (p_keyscan_fifo_data->len == 1)
    {
        key_handle_global_data.cur_press_key_index = key_index_1;

        /* lightness and clor temperature control */
        if (!keyscan_long_press_timer_status_get())
        {
            key_handle_one_key_scenario(&key_handle_global_data);
        }
    }
    else if (p_keyscan_fifo_data->len == 2)
    {
#ifdef COMBINE_KEY_SCENARIO
        key_handle_two_keys_scenario(key_index_1, key_index_2);
#endif
        APP_PRINT_WARN0("[key_handle_pressed_event] FIFO length is 2!");
    }
    else
    {
        /* more than two keys are pressed, just ignore this scenario.
           If need to use three or more keys as combined keys, need
           to caution ghost keys!
        */
    }
}

/**
* @brief   handle key release event
* @param   none
* @return  void
*/
void key_handle_release_event(T_KEYSCAN_FIFIO_DATA *p_keyscan_fifo_data)
{
    LED_OFF(LED_1);

#ifdef COMBINE_KEY_SCENARIO
    /* one key scenario */
    if (key_handle_global_data.cur_press_key_index != VK_NC)
    {
        APP_PRINT_INFO0("[key_handle_release_event] key release event");
        //If need, please send release message to peer device!
        if (GROUP_INVALID != GROUP_CTL_func_map[key_handle_global_data.cur_press_key_index].group)
        {
            key_handle_global_data.last_pressed_key_index = key_handle_global_data.cur_press_key_index;
        }
        key_handle_global_data.cur_press_key_index = VK_NC;
    }

    /* Two keys scenario */
    if (key_handle_global_data.combine_keys_status == ADV_COMBINE_KEYS_BIT_MASK)
    {
        os_timer_stop(&combine_keys_detection_timer);
    }
    keyscan_init_data();
#else
    for (uint8_t index = 0; index < (p_keyscan_fifo_data->len); index++)
    {
        APP_PRINT_INFO4("[key_handle_release_event] keyscan data[%d]: row - %d, column - %d, value - %d", \
                        index, p_keyscan_fifo_data->key[index].row, p_keyscan_fifo_data->key[index].column,
                        KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[index].row][p_keyscan_fifo_data->key[index].column]);
    }

    T_KEY_INDEX_DEF key_index_1 =
        KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[0].row][p_keyscan_fifo_data->key[0].column];
    key_handle_global_data.cur_press_key_index = key_index_1;

    DBG_DIRECT("release->key_index_1: %d, long press:%d", key_index_1,
               key_handle_global_data.long_press_status);

    if (!key_handle_global_data.long_press_status)
    {
        /* Send press key packet */
        key_handle_one_key_scenario(&key_handle_global_data);

        if (GROUP_INVALID != GROUP_CTL_func_map[key_handle_global_data.cur_press_key_index].group)
        {
            key_handle_global_data.last_pressed_key_index = key_handle_global_data.cur_press_key_index;
        }
        key_handle_global_data.cur_press_key_index = VK_NC;
    }

    key_handle_global_data.long_press_status = false;
    keyscan_init_data();
#endif
}

/**
* @brief   handle key long pressed event
* @param   p_keyscan_fifo_data - point of keyscan FIFO data
* @return  void
*/
void key_handle_long_pressed_event(T_KEYSCAN_FIFIO_DATA *p_keyscan_fifo_data)
{
    T_KEY_INDEX_DEF key_index_1 =
        KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[0].row][p_keyscan_fifo_data->key[0].column];
    key_handle_global_data.long_press_status = true;

    key_handle_global_data.group_key = GROUP_CTL_func_map[key_index_1].group;

    if (key_index_1 == VK_POWER_OFF)
    {
        /* Clear group information */
        group_transmitter_cfg_group(GROUP_INVALID);
    }
    else if (key_index_1 == VK_GROUP_1_OFF)
    {
        /* Enter night mode */
        group_transmitter_ctl_night_light(key_handle_global_data.group_key);
    }
    else if (key_index_1 == VK_GROUP_2_OFF)
    {
        /* Enter good night mode */
        group_transmitter_ctl_good_night(key_handle_global_data.group_key);
    }
    else
    {
        /* Enter good night mode */
        group_transmitter_cfg_group(key_handle_global_data.group_key);
    }

}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

