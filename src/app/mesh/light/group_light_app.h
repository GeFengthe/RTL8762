/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     group_light_app.h
  * @brief    Head file for group related.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2018-11-06
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _GROUP_LIGHT_APP_H
#define _GROUP_LIGHT_APP_H

/* Add Includes here */
#include "platform_misc.h"
#include "group.h"

BEGIN_DECLS

/**
 * @addtogroup Group_Light
 * @{
 */

/** @defgroup Group_Light_Exported_Macros Exported Macros
  * @brief
  * @{
  */

#define GROUP_LIGHT_SUPPORTED           1
/** @} */

/** @defgroup Group_Light_Exported_Functions Exported Functions
  * @brief
  * @{
  */
void group_light_init(void);
void group_light_sync(void);
/** @} */
/** @} */

END_DECLS

#endif /* _GROUP_LIGHT_APP_H */
