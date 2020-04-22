/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     battery_driver.h
* @details
* @author   Yuyin_zhang
* @date     2018-11-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef _BATTERY_DRIVER_
#define _BATTERY_DRIVER_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "rtl876x_lpc.h"
#include "bee2_adc_lib.h"

/* Defines -------------------------------------------------------------------*/
#define ADC_CHANNEL_INDEX               1
#define ADC_SCHEDULE_INDEX              0

typedef enum
{
    NORMAL_MODE = 0,
    LOW_POWER_MODE,
} BAT_MODE;

typedef struct
{
    bool       is_working;                 /* indicate if adc working*/
    uint8_t    vbat_mode;
    uint8_t    vbat_level;
    uint16_t   vbat_value;
    uint16_t   vbat_low_power_threshold;
} vbat_stg;


/*============================================================================*
 *                       Interface Functions
 *============================================================================*/

void bat_module_init(void);
void battery_get_value(uint16_t *p_level, uint16_t *p_value);
bool is_vbat_working(void);
void battery_msg_handle(uint16_t msg_sub_type);
void bat_nvic_config(void);
BAT_MODE get_bat_mode(void);

#ifdef BAT_EN
#define rcu_bat_init()                    bat_module_init()
#define rcu_get_bat_value(level, value)   battery_get_value(level,value)
#define bat_allow_enter_dlps()            is_vbat_working()
#define rcu_bat_handle(type)              battery_msg_handle(type)
#define rcu_bat_nvic_config()             bat_nvic_config()
#define rcu_get_bat_mode()                get_bat_mode()
#else
#define rcu_bat_init()
#define rcu_get_bat_value(level, value)
#define bat_allow_enter_dlps()
#define rcu_bat_handle(type)
#define rcu_bat_nvic_config()
#define rcu_get_bat_mode()
#endif

#ifdef __cplusplus
}
#endif

#endif /*_BATTERY_DRIVER_*/

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

