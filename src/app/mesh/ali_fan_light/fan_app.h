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

#include "platform_types.h"

BEGIN_DECLS

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
void fan_server_models_init(uint8_t element_index);

/**
 * @brief self subscribe
 */
void fan_server_models_sub(void);

/** @} */
/** @} */

END_DECLS

#endif /* _FAN_APP_H */

