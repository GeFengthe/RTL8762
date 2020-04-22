/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      ali_app.h
* @brief     ali application
* @details
* @author    bill
* @date      2019-8-5
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _ALI_APP_H_
#define _ALI_APP_H_

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
 * @brief initialize ali light server models
 * @param[in] element_index: model element index
 */
void ali_server_models_init(uint8_t element_index);

/**
 * @brief self subscribe
 */
void ali_server_models_sub(void);

/** @} */
/** @} */

END_DECLS

#endif /* _ALI_APP_H_ */

