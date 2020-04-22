/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      app_flags.h
   * @brief     This file is used to config app functions.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */
#ifndef _APP_FLAGS_H_
#define _APP_FLAGS_H_


/** @defgroup  CENTRAL_Config Central App Configuration
    * @brief This file is used to config app functions.
    * @{
    */
/*============================================================================*
 *                              Constants
 *============================================================================*/

/** @brief  Config APP LE link number */
#define APP_MAX_LINKS  2
/** @brief  Config GATT services storage: 0-Not save, 1-Save to flash
 *
 * If configure to 1, the GATT services discovery results will save to the flash.
 */
#define F_BT_GATT_SRV_HANDLE_STORAGE            0

/** @} */ /* End of group CENTRAL_Config */

#endif
