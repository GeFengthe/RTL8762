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

#ifndef _POS_APP_H_
#define _POS_APP_H_

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <profile_client.h>
#include <profile_server.h>

/*============================================================================*
 *                              Variables
 *============================================================================*/

extern T_SERVER_ID pos_srv_id;  /**< service id */
/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg);
/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data);

/**
 * @brief  Callback will be called when data sent from profile client layer.
 * @param  client_id the ID distinguish which module sent the data.
 * @param  conn_id connection ID.
 * @param  p_data  pointer to data.
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT app_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data);
T_APP_RESULT app_profile_callback(T_SERVER_ID service_id, void *p_data);
void swTimerInit(void);
#if F_BT_AIRPLANE_MODE_SUPPORT
void app_gap_common_callback(uint8_t cb_type, void *p_cb_data);
#endif
#if F_BT_GAPS_CHAR_WRITEABLE
T_APP_RESULT gap_service_callback(T_SERVER_ID service_id, void *p_para);
#endif

#ifdef __cplusplus
}
#endif

#endif

