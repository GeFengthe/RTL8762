/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      fan_app.h
* @brief     ali fan application
* @details
* @author    bill
* @date      2019-3-26
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _FAN_APP_H
#define _FAN_APP_H

#include "ali_model.h"
#include "platform_types.h"
#include "generic_on_off.h"
BEGIN_DECLS
typedef struct
{
    uint32_t unix_time;
    uint8_t generic_on_off; //!< 0: off; 1: on
    uint8_t timezone;
} generic_data_t;

typedef struct
{
    uint8_t main_on_off;
    uint8_t bg_on_off;
} light_flash_vendor_light_state_t;

typedef struct
{
    uint16_t attr_type;
    uint8_t error_code;
    uint8_t index;
} error_data_t;

typedef struct
{
    ali_attr_t ali_attr;
    uint32_t opcode;
    uint8_t tid;
    uint8_t cnt;
} _PACKED_ arr_ali_attr_t;

typedef void (*msg_report_cb)(void *ptimer);

typedef struct
{
    arr_ali_attr_t arr_ali_attr[3];
    plt_timer_t msg_report_timer[3];
    msg_report_cb report_cb[3];
} _PACKED_ msg_info_t;

#define MSG_RPT_TIME_OUT (1000)  //1s
//extern timer_data_t *pTimer_adjust;
extern generic_data_t generic_data_current;
extern light_flash_vendor_light_state_t vendor_light_state;
/**
 * @addtogroup FAN_SERVER_APP
 * @{
 */

/**
 * @defgroup FAN_SERVER_EXPORTED_FUNCTIONS Exported Functions
 * @brief
 * @{
 */
/**
 * @brief initialize fan server models
 * @param[in] element_index: model element index
 */
void light_ctl_server_models_init(uint8_t element_index);

/**
 * @brief self subscribe
 */
void light_ctl_server_models_sub(void);
void light_ctl_server_models_pub(void);
void light_ctl_server_models_pub_start(void);
void send_update_request(void);
void clear_timer(uint8_t num);
uint16_t get_prd_unix_time(void);
uint8_t get_prd_schedule(void);
uint8_t get_prd_state(void);
bool is_prd_exist(void);
void power_on_message(void);
void handle_pub_evt(generic_on_off_t on_off);


/** @} */
/** @} */

END_DECLS

#endif /* _FAN_APP_H */

