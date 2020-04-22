/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     dimmable_light.c
* @brief    This file provides demo code for the operation of dimmable light.
* @details
* @author   bill
* @date     2017-01-07
* @version  v1.0
*********************************************************************************************************
*/

#include "dimmable_light.h"
#include "mesh_api.h"
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_tim.h"
#include "ftl.h"
#include "light_config.h"
#if MESH_ALI_CERTIFICATION
#include "light_effect_app.h"
#endif

#define LED_PWM_FREQ        200 //!< Hz
#define LED_PWM_COUNT       (40000000/LED_PWM_FREQ)


static light_flash_power_on_count_t light_power_on_count;
static plt_timer_t power_on_timer;

void light_driver_init(void)
{
    /* turn on timer clock */
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);
    *((volatile uint32_t *)0x40000360UL) &= ~(1 << 10);
}

void light_pin_config(const light_t *light)
{
    if (PIN_INVALID == light->pin_num)
    {
        return ;
    }
    /* pad & pinmux */
    Pad_Config(light->pin_num, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pinmux_Config(light->pin_num, light->pin_func);
    /* TIM */
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);
    TIM_InitStruct.TIM_PWM_En = PWM_ENABLE;
    /*<! PWM output freqency = 40M/(TIM_PWM_High_Count + TIM_PWM_Low_Count) */
    /*<! PWM duty cycle = TIM_PWM_High_Count/(TIM_PWM_High_Count + TIM_PWM_Low_Count) */
    uint32_t high_count;
    if (0xffff == light->lightness)
    {
        high_count = LED_PWM_COUNT;
    }
    else
    {
        high_count = (LED_PWM_COUNT / 65535.0) * light->lightness;
    }

    if (light->pin_high_on)
    {
        TIM_InitStruct.TIM_PWM_High_Count = high_count;
        TIM_InitStruct.TIM_PWM_Low_Count = LED_PWM_COUNT - high_count;
    }
    else
    {
        TIM_InitStruct.TIM_PWM_High_Count = LED_PWM_COUNT - high_count;
        TIM_InitStruct.TIM_PWM_Low_Count = high_count;
    }

    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_SOURCE_DIV = TIM_CLOCK_DIVIDER_1;
    TIM_TimeBaseInit(light->tim_id, &TIM_InitStruct);
    /* Enable PWM output */
    TIM_Cmd(light->tim_id, ENABLE);
}

void light_pin_dlps_config(const light_t *light)
{
    if (PIN_INVALID == light->pin_num)
    {
        return ;
    }
    PAD_OUTPUT_VAL val;
    if (light->pin_high_on)
    {
        val = PAD_OUT_LOW;
    }
    else
    {
        val = PAD_OUT_HIGH;
    }
    /* pad & pinmux */
    Pad_Config(light->pin_num, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               val);
}

void light_lighten(light_t *light, uint16_t lightness)
{
    if (PIN_INVALID == light->pin_num)
    {
        return ;
    }
    uint32_t high_count;
    if (0xffff == lightness)
    {
        high_count = LED_PWM_COUNT;
    }
    else
    {
        high_count = (LED_PWM_COUNT / 65535.0) * lightness;
    }

    if (light->pin_high_on)
    {
        TIM_PWMChangeFreqAndDuty(light->tim_id, high_count, LED_PWM_COUNT - high_count);
    }
    else
    {
        TIM_PWMChangeFreqAndDuty(light->tim_id, LED_PWM_COUNT - high_count, high_count);
    }

    light->lightness = lightness;
}

void light_set_lightness(light_t *light, uint16_t lightness)
{
    if (PIN_INVALID == light->pin_num)
    {
        return ;
    }
    light_lighten(light, lightness);
    light->lightness_last = lightness;
}

void light_blink_infinite(light_t *light, uint32_t hz_numerator, uint32_t hz_denominator,
                          uint8_t duty)
{
    if (PIN_INVALID == light->pin_num)
    {
        return ;
    }
    if (hz_numerator == 0 || hz_denominator == 0 || duty > 100)
    {
        return;
    }

    uint32_t high_count = (40000000 / 100) * hz_denominator / hz_numerator * duty;
    uint32_t low_count = (40000000 / 100) * hz_denominator / hz_numerator * (100 - duty);

    if (light->pin_high_on)
    {
        TIM_PWMChangeFreqAndDuty(light->tim_id, low_count, high_count);
    }
    else
    {
        TIM_PWMChangeFreqAndDuty(light->tim_id, high_count, low_count);
    }
}

bool light_flash_write(light_flash_param_type_t type, uint16_t len, void *pdata)
{
    uint32_t ret = 0;
    switch (type)
    {
    case LIGHT_FLASH_PARAM_TYPE_POWER_ON_COUNT:
        {
            ret = ftl_save(pdata, LIGHT_FLASH_PARAMS_APP_OFFSET + MEMBER_OFFSET(light_flash_param_t,
                                                                                power_on_count),
                           len);
        }
        break;
    case LIGHT_FLASH_PARAM_TYPE_LIGHT_STATE:
        {
            ret = ftl_save(pdata, LIGHT_FLASH_PARAMS_APP_OFFSET + MEMBER_OFFSET(light_flash_param_t,
                                                                                light_state),
                           len);
        }
        break;
    case LIGHT_FLASH_PARAM_TYPE_USER_DATA:
        {
            ret = ftl_save(pdata, LIGHT_FLASH_PARAMS_APP_OFFSET + MEMBER_OFFSET(light_flash_param_t, user_data),
                           MIN(len, FTL_MAP_DIMMABLE_LIGHT_SIZE - MEMBER_OFFSET(light_flash_param_t, user_data)));
        }
        break;
    default:
        break;
    }

    if (0 != ret)
    {
        printe("light_flash_write: failed, type = %d, cause = %d", type, ret);
    }

    return (0 == ret);
}

bool light_flash_read(light_flash_param_type_t type, uint16_t len, void *pdata)
{
    uint32_t ret = 0;
    switch (type)
    {
    case LIGHT_FLASH_PARAM_TYPE_POWER_ON_COUNT:
        ret = ftl_load((void *)pdata,
                       LIGHT_FLASH_PARAMS_APP_OFFSET + MEMBER_OFFSET(light_flash_param_t, power_on_count),
                       len);
        break;
    case LIGHT_FLASH_PARAM_TYPE_LIGHT_STATE:
        ret = ftl_load((void *)pdata,
                       LIGHT_FLASH_PARAMS_APP_OFFSET + MEMBER_OFFSET(light_flash_param_t, light_state),
                       len);
        break;
    case LIGHT_FLASH_PARAM_TYPE_USER_DATA:
        ret = ftl_load((void *)pdata,
                       LIGHT_FLASH_PARAMS_APP_OFFSET + MEMBER_OFFSET(light_flash_param_t, user_data),
                       len);
        break;
    default:
        break;
    }
    if (0 != ret)
    {
        printe("light_flash_read: restore data from flash failed");
    }

    return (0 == ret);
}

static void light_power_on_timeout_cb(void *timer)
{
    plt_timer_delete(power_on_timer, 0);
    power_on_timer = NULL;
    light_power_on_count.count = 0;
    light_flash_write(LIGHT_FLASH_PARAM_TYPE_POWER_ON_COUNT, sizeof(light_flash_power_on_count_t),
                      &light_power_on_count);
}

bool light_power_on_count_check(uint8_t count)
{
    if (count >= LIGHT_POWER_ON_COUNT)
    {
        mesh_node_clear();
        light_power_on_count.count = 0;
        light_flash_write(LIGHT_FLASH_PARAM_TYPE_POWER_ON_COUNT, sizeof(light_flash_power_on_count_t),
                          &light_power_on_count);
#if MESH_ALI_CERTIFICATION
        light_set_unprov_effect(TRUE);
#endif
        return TRUE;
    }
    else
    {
        light_power_on_count.count = count + 1;
        power_on_timer = plt_timer_create("PO", LIGHT_POWER_ON_TIME, 0, 0, light_power_on_timeout_cb);
        plt_timer_start(power_on_timer, 0);
        light_flash_write(LIGHT_FLASH_PARAM_TYPE_POWER_ON_COUNT, sizeof(light_flash_power_on_count_t),
                          &light_power_on_count);
        return FALSE;
    }
}

