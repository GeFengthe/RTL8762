/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      scatternet_app.h
   * @brief     This file handles BLE scatternet application routines.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

#ifndef _PXP_SMART_APP_H_
#define _PXP_SMART_APP_H_

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "app_msg.h"

/*============================================================================*
 *                              Functions
 *============================================================================*/

#define MSG_KEY_PRESS      0
#define MSG_KEY_RELEASE    1
void app_handle_io_msg(T_IO_MSG io_msg);

#ifdef __cplusplus
}
#endif

#endif

