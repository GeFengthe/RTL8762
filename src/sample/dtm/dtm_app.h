/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      dtm_app.h
* @brief
* @details
* @author    jeff
* @date      2016-12-01
* @version   v1.0
*
*/

#ifndef _DTM_H_
#define _DTM_H_

#include <stdint.h>
#include <gap_le.h>
#include <gap_msg.h>

/**
  * @brief  Uart initialization
  * @param  void
  * @return void
  */
void dtm_uart_init(void);

/**
  * @brief      handle test command from 8852B, then invoke gap api to start test procedure.
  * @param[in]  command commands from 8852B.
  * @return     void
  */
void dtm_test_req(uint16_t command);

/**
  * @brief      Callback for gap le to notify app
  * @param[in]  cb_type callback msy type
  * @param[in]  p_cb_data point to callback data
  * @retval     result
  */
T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data);

#endif

