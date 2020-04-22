
/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2018 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */
#ifndef _KEYSCAN_DRIVER_H_
#define _KEYSCAN_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <rtl876x.h>
#include "platform_os.h"

/*============================================================================*
 *                         Macros
 *============================================================================*/
//#define KEYSCAN_PRINT_LOG  /* Enable print log or not */

#ifdef KEYSCAN_PRINT_LOG
#define KEYSCAN_DBG_BUFFER(MODULE, LEVEL, fmt, para_num,...) DBG_BUFFER_##LEVEL(TYPE_BEE2, SUBTYPE_FORMAT, MODULE, fmt, para_num, ##__VA_ARGS__)
#else
#define KEYSCAN_DBG_BUFFER(MODULE, LEVEL, fmt, para_num,...) ((void)0)
#endif

#define KEYSCAN_SW_INTERVAL     50  /* 50ms */
#define KEYSCAN_SW_RELEASE_TIMEOUT 10 /* 10ms */

#define KEYSCAN_LONG_PRESS_DETECT_TIMEOUT     1000
#define KEYSCAN_REPEAT_SEND_INTERVAL          300

/*============================================================================*
 *                         Types
 *============================================================================*/
/**
 * @brief  KeyScan FIFO data struct definition.
 */
typedef struct
{
    uint32_t len;               /**< Keyscan state register        */
    struct
    {
        uint8_t column: 5;      /**< Keyscan raw buffer data       */
        uint8_t row: 3;         /**< Keyscan raw buffer data       */
        uint8_t reserved;
    } key[8];
} T_KEYSCAN_FIFIO_DATA;

/**
 * @brief  KeyScan global data struct definition.
 */
typedef struct
{
    bool is_allowed_to_enter_dlps;  /* to indicate whether to allow to enter dlps or not */
    bool is_key_pressed;  /* to indicate whether any key is pressed or not */
    bool is_all_key_released;  /* to indicate whether all keys are released or not */
    T_KEYSCAN_FIFIO_DATA pre_fifo_data;  /* to indicate the previous keyscan FIFO data */
    T_KEYSCAN_FIFIO_DATA cur_fifo_data;  /* to indicate the current keyscan FIFO data */
} T_KEYSCAN_GLOBAL_DATA;

/*============================================================================*
*                        Export Global Variables
*============================================================================*/
extern T_KEYSCAN_GLOBAL_DATA keyscan_global_data;
extern plt_timer_t keyscan_timer;

/*============================================================================*
 *                         Functions
 *============================================================================*/
void keyscan_init_data(void);
void keyscan_init_driver(uint32_t is_debounce);
void keyscan_pinmux_config(void);
void keyscan_init_pad_config(void);
void keyscan_nvic_config(void);
void keyscan_enter_dlps_config(void);
void keyscan_exit_dlps_config(void);
void keyscan_init_timer(void);
void keyscan_timer_callback(plt_timer_t pxTimer);
void keyscan_long_press_detect_init_timer(void);
bool keyscan_long_press_timer_status_get(void);
void keyscan_long_press_timer_stop(void);
void keyscan_repeat_send_timer_init_timer(void);
void keyscan_repeat_send_timer_stop(void);

#ifdef __cplusplus
}
#endif

#endif

