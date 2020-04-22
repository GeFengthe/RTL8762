/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     led_driver.c
* @brief    this file provides led module function driver for users.
* @details  multi prio, multi event, multi led support.
* @author   Yuyin_zhang
* @date     2018-04-28
* @version  v1.0
* @note     the more led number and led event, the more time cpu spend on tick handle
*           For example: led_num = 8, led_event_num = 10, max time is 112us.
*********************************************************************************************************
*/

#include "board.h"
#include "trace.h"
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "os_timer.h"
#include "led_driver.h"

#if LED_EN

#define LED_TIMER_STATE(state)   os_timer_state_get(&led_ctrl_timer, &state)
#define LED_TIMER_START(n)       os_timer_restart(&led_ctrl_timer, n)
#define LED_TIMER_STOP()         os_timer_stop(&led_ctrl_timer)

#if (LED_ON_LEVEL_TRIG == LED_ON_LEVEL_HIGH)
#define led_on(index)            Pad_OutputControlValue(index, PAD_OUT_HIGH)
#define led_off(index)           Pad_OutputControlValue(index, PAD_OUT_LOW)
#elif (LED_ON_LEVEL_TRIG == LED_ON_LEVEL_LOW)
#define led_on(index)            Pad_OutputControlValue(index, PAD_OUT_LOW)//Pad_Config(index, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW)
#define led_off(index)           Pad_OutputControlValue(index, PAD_OUT_HIGH)//Pad_Config(index, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH)
#endif

/*============================================================================*
 *                              External Variables
 *============================================================================*/

typedef void *TimerHandle_t;


/*============================================================================*
 *                              Local Variables
 *============================================================================*/
uint32_t  led_tick_cnt = 0;
led_data_stg led_arr[LED_NUM_MAX] = {0};
TimerHandle_t led_ctrl_timer;
const led_event_stg led_event_arr[LED_TYPE_MAX] =
{
    {LED_TYPE_IDLE,                   LED_LOOP_BITS_IDLE,             LED_BIT_MAP_INVALID},
    {LED_TYPE_BLINK_LOW_POWER,        LED_LOOP_BITS_LOW_POWER,        LED_BIT_MAP_LOW_POWER},
    {LED_TYPE_BLINK_IR_LEARN_SUCCESS, LED_LOOP_BITS_IR_LEARN_SUCCESS, LED_BIT_MAP_IR_LEARN_SUCCESS},
    {LED_TYPE_BLINK_IR_LEARN_WAITING, LED_LOOP_BITS_IR_LEARN_WAITING, LED_BIT_MAP_IR_LEARN_WAITING},
    {LED_TYPE_BLINK_IR_LEARN_MODE,    LED_LOOP_BITS_IR_LEARN_MODE,    LED_BIT_MAP_IR_LEARN_MODE},
    {LED_TYPE_BLINK_OTA_FAIL,         LED_LOOP_BITS_OTA_FAIL,         LED_BIT_MAP_OTA_FAIL},
    {LED_TYPE_BLINK_PAIR_SUCCESS,     LED_LOOP_BITS_PAIR_SUCCESS,     LED_BIT_MAP_PAIR_SUCCESS},
    {LED_TYPE_ON,                     LED_LOOP_BITS_ON,               LED_BIT_MAP_ON},
};
/*============================================================================*
 *                              External Functions
 *============================================================================*/



/*============================================================================*
 *                              Loacal Functions
 *============================================================================*/

LED_RET_CAUSE led_blink_exit(uint16_t index, LED_TYPE type);
uint32_t led_get_next_event(const uint32_t bitmap);
LED_RET_CAUSE led_blink_start(uint16_t index, LED_TYPE type, uint8_t cnt);
void led_ctrl_timer_cb(TimerHandle_t pxTimer);
void led_tick_handle(uint8_t index);

/**
 * @brief  led module init.
 *@param   void.
 * @return     void.
 * @note       none.
**/
void led_module_init(void)
{
    bool retval = false;

    /*init led default state*/
#if (LED_ON_LEVEL_TRIG == LED_ON_LEVEL_HIGH)
    Pad_Config(LED_1 & 0xFF, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
#elif (LED_ON_LEVEL_TRIG == LED_ON_LEVEL_LOW)
    Pad_Config(LED_1 & 0xFF, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
#endif

    /*create led tick timer*/
    retval = os_timer_create(&led_ctrl_timer, "led_ctrl_timer", 1, LED_PERIOD, false,
                             led_ctrl_timer_cb);
    if (!retval)
    {
        APP_PRINT_INFO1("[Led] led_ctrl_timer ret is %d", retval);
    }
}
/**
 * @brief  start led event.
 *@param[in]   led_index led pin index.
 * @param[in]  type led type to set.
 * @param[in]  cnt  led blink count.
 * @return     void.
 * @retval     none
 * @note       none.
**/
LED_RET_CAUSE led_blink_start(uint16_t led_index, LED_TYPE type, uint8_t cnt)
{
    uint32_t state = 0;
    if ((LED_TYPE_IDLE == type) || (LED_TYPE_MAX <= type))
    {
        return LED_ERR_TYPE;
    }

    if (LED_NUM_MAX <= (led_index >> 8))
    {
        return LED_ERR_INDEX;
    }

    /*set led new type*/
    led_off(led_index & 0xFF);

    /*set led event map flag*/
    led_arr[led_index >> 8].led_map |= (1 << type);
    /*record led pin*/
    led_arr[led_index >> 8].led_pin = led_index & 0xFF;

    if (cnt > 0)
    {
        led_arr[led_index >> 8].led_cnt_arr[type].led_loop_cnt = cnt;
        led_arr[led_index >> 8].led_cnt_arr[type].cur_tick_cnt = led_tick_cnt %
                                                                 led_event_arr[type].led_loop_cnt;
    }
    else
    {
        led_on(led_index & 0xFF);
        return LED_SUCCESS;
    }

    /*start led tick*/
    LED_TIMER_STATE(state);
    if (!state)
    {
        LED_TIMER_START(LED_PERIOD);
    }

#if LED_DEBUG
    APP_PRINT_INFO2("[Led] led_BlinkStart, type = %x, led_index = %x", type, led_index >> 8);
#endif

    return LED_SUCCESS;

}

#if 1
/**
 * @brief  handle tick msg for each led event.
 *
 * @param[in]  led_index led index to handle.
 * @return     void.
 * @retval     none.
 * @note       none.
**/
void led_tick_handle(uint8_t led_index)
{
    uint8_t index = 0;

    if (0 == led_arr[led_index].led_map)
    {
        return;
    }

    for (index = 0; index < LED_TYPE_MAX; index++)
    {
        /*update led state, when new tick come*/
        uint32_t mask = (1 << index);

        if (0 == (mask & led_arr[led_index].led_map))
        {
            continue;
        }

        if ((led_arr[led_index].led_cnt_arr[index].led_loop_cnt > 0)
            && (led_arr[led_index].led_cnt_arr[index].cur_tick_cnt == (led_tick_cnt %
                                                                       led_event_arr[index].led_loop_cnt)))
        {
            led_arr[led_index].led_cnt_arr[index].led_loop_cnt --;
            if (0 == led_arr[led_index].led_cnt_arr[index].led_loop_cnt)
            {
                /*clear event bit*/
                led_arr[led_index].led_map &= ~(1 << index);
                /*turn off led*/
                //led_off(led_arr[led_index].led_pin);
            }
        }
    }
}
#endif
/**
 * @brief  this function terminate led blink according to the led type.
 *
 * @param[in]  index led index.
 * @param[in]  type  led blink type.
 * @return     void.
 * @retval     none
 * @note       none.
**/
LED_RET_CAUSE led_blink_exit(uint16_t index, LED_TYPE type)
{
    if ((LED_TYPE_IDLE == type) || (LED_TYPE_MAX <= type))
    {
        return LED_ERR_TYPE;
    }

    if (LED_NUM_MAX <= (index >> 8))
    {
        return LED_ERR_INDEX;
    }

    /*clear current led type bit*/
    led_arr[index >> 8].led_map &= ~(1 << type);

    /*start led*/
    if ((LED_TYPE_IDLE == led_arr[index >> 8].led_map))
    {
#if LED_DEBUG
        APP_PRINT_INFO1("[Led] led_blink_exit, led_index = 0x%x ", index);
#endif
        led_off(led_arr[index >> 8].led_pin);
        return LED_SUCCESS;
    }
    return LED_SUCCESS;
}

/**
 * @brief  get next led blink event.
 *
 * @param[in]  bitmap  led event bit map, indicate which led event bit map to get.
 * @return     return led event index.
 * @retval     none
 * @note       none.
**/
uint32_t led_get_next_event(uint32_t bitmap)
{
    for (uint32_t index = 0; index < LED_TYPE_MAX; index++)
    {
        uint32_t bit_mask = 0;
        bit_mask |= (1 << index);
        if (bit_mask & bitmap)
        {
            return index;
        }
#if LED_DEBUG
        APP_PRINT_INFO1("[Led] index = %d", index);
#endif
    }
    return LED_TYPE_IDLE;
}

/**
 * @brief  callback led timer timeout.
 *
 * @param[in]  pxTimer.
 * @return     void.
 * @retval     none
 * @note       none.
**/
void led_ctrl_timer_cb(TimerHandle_t pxTimer)
{
#if LED_DEBUG
    APP_PRINT_INFO1("[Led] led_ctrl_timer_cb, led_blink_type = %x", led_tick_cnt);
#endif

    /*led timer restart*/
    LED_TIMER_START(LED_PERIOD);

    /**update led blink tick*/
    led_tick_cnt ++;

    /*update led state, when new tick come*/
    uint32_t mask = 0;
    bool event_flag = false;

    for (uint8_t index = 0; index < LED_NUM_MAX; index++)
    {
        uint32_t map = 0;
        uint32_t type = 0;

        /*1. get highest prio event type*/
        if (led_arr[index].led_map != 0)
        {
            type = led_get_next_event(led_arr[index].led_map);
            event_flag = true;
        }
        else
        {
            continue;
        }

        /*2. get highest event mask*/
        mask = (1 << (led_tick_cnt % led_event_arr[type].led_loop_cnt));

        /*3. get highest prio bit map*/
        if (led_event_arr[type].led_type_index == type)
        {
            map = led_event_arr[type].led_bit_map;
        }

        /*4. set led state according to bit map*/
        if (mask & map)
        {
            led_on(led_arr[index].led_pin);
        }
        else
        {
            led_off(led_arr[index].led_pin);
        }

        /*led tick msg handle, update left events state*/
        led_tick_handle(index);
    }

    if (event_flag == false)
    {
        LED_TIMER_STOP();
    }

}

#endif
