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


/** @defgroup  PRIVA_PERIPH_Config Peripheral Privacy App Configuration
    * @brief This file is used to config app functions.
    * @{
    */
/*============================================================================*
 *                              Constants
 *============================================================================*/

/** @brief  Configure APP LE link number */
#define APP_MAX_LINKS  1
/** @brief  User command: 0-Close user command, 1-Open user command */
#define USER_CMD_EN    1

/** @brief  Configure Privacy1.2 feature: 0-Closed, 1-Open */
#define APP_PRIVACY_EN 1

#if APP_PRIVACY_EN
/** @brief  Configure the authentication requirement of simple_ble_service.c */
#define SIMP_SRV_AUTHEN_EN 1
#endif

/** @} */ /* End of group PRIVA_PERIPH_Config */
#endif
