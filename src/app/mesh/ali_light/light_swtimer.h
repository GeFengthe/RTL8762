/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      light_swtimer.h
* @brief     header file of software timer implementation
* @details
* @author    elliot chen
* @date      2018-11-27
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _LIGHT_SWTIMER_
#define _LIGHT_SWTIMER_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "platform_os.h"
#include "app_msg.h"
#include "app_task.h"
#include "mesh_beacon.h"
#include "gap_scheduler.h"
#include "light_dlps_ctrl.h"
#include "trace.h"
#include "mesh_api.h"

/* Defines ------------------------------------------------------------------*/
/**
 * @addtogroup LIGHT_SWTIMER
 * @{
 */

/**
 * @defgroup Light_Swtimer_Exported_Macros Light Software Timer Exported Macros
 * @brief
 * @{
 */
#define UNPROV_TIME_OUT                     (10*60*1000)
#define CHANGE_SCAN_PARAM_TIME_OUT          (10*1000)
/** @} */

/**
 * @defgroup Light_Swtimer_Exported_Types Light Software Timer Exported Types
 * @brief
 * @{
 */
typedef enum
{
    UNPROV_TIMEOUT,
    PROV_SUCCESS_TIMEOUT
} SW_TIMER_MSG_TYPE;
/** @} */

/**
 * @defgroup Light_Swtimer_Exported_Functions Light Software Timer Exported Functions
 * @brief
 * @{
 */
void unprov_timer_init(void);
void unprov_timer_start(void);
void unprov_timer_stop(void);
void change_scan_param_timer_init(void);
void change_scan_param_timer_start(void);
void light_handle_sw_timer_msg(T_IO_MSG *io_msg);
/** @} */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /*_LIGHT_SWTIMER_*/

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
