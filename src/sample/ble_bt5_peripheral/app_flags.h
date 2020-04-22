/**
*****************************************************************************************
*     Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      app_flags.h
   * @brief     This file is used to config app functions.
   * @author    berni
   * @date      2018-04-27
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2018 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */
#ifndef _APP_FLAGS_H_
#define _APP_FLAGS_H_


/** @defgroup  BT5_PERIPH_Config BT5 Peripheral App Configuration
    * @brief This file is used to configure app functions.
    * @{
    */
/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @brief Configure Advertising PHY */
#define ADVERTISING_PHY            APP_PRIMARY_CODED_SECONDARY_CODED
/** @brief Configure coding scheme of LE Coded PHY: 0 - S = 2, 1 - S = 8 */
#define LE_CODED_PHY_S8            0
/** @brief Configure APP LE link number */
#define APP_MAX_LINKS              1
/** @brief Configure DLPS: 0 - Disable DLPS, 1 - Enable DLPS */
#define F_BT_DLPS_EN               1
/** @brief 1 - Enable LE Advertising Extensions */
#define F_BT_LE_5_0_AE_ADV_SUPPORT 1

/** @} */ /* End of group BT5_PERIPH_Config */
#endif
