/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      strip_app.h
* @brief     Head file of ali strip application
* @details
* @author    astor
* @date      2019-4-10
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _POWER_STRIP_APP_H
#define _POWER_STRIP_APP_H

#include "platform_types.h"
#include "generic_on_off.h"
BEGIN_DECLS



typedef struct
{
    uint32_t unix_time;
    uint8_t strip_on_off; //!< 0: off; 1: on
    uint8_t timezone;
    uint16_t time_sync_period;
    uint8_t time_sync_retry_delay;
    uint8_t time_sync_retry_times;
    bool has_updated;
    bool need_retry;
} strip_data_t;



//extern timer_data_t *pTimer_adjust;
extern strip_data_t strip_data_current;

/**
 * @addtogroup STRIP_SERVER_APP
 * @{
 */

/**
 * @defgroup STRIP_SERVER_EXPORTED_FUNCTIONS Exported Functions
 * @brief
 * @{
 */
/**
 * @brief initialize strip server models
 * @param[in] element_index: model element index
 */
void strip_server_models_init(uint8_t element_index);

/**
 * @brief self subscribe
 */
void strip_server_models_sub(void);

/** @} */
/** @} */

void handle_pub_evt(generic_on_off_t on_off);
void send_update_request(void);
void clear_timer(uint8_t num);
void clear_period(uint8_t num);
uint16_t get_prd_unix_time(void);
uint8_t get_prd_schedule(void);
uint8_t get_prd_state(void);
bool is_prd_exist(void);

END_DECLS

#endif /* _STRIP_APP_H */

