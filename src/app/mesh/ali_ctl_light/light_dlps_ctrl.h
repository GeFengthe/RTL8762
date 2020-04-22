/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      light_dlps_ctrl.h
* @brief
* @details
* @author    Elliot Chen
* @date      2018-11-27
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _LIGHT_DLPS_CTRL_
#define _LIGHT_DLPS_CTRL_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"
/**
 * @addtogroup LIGHT_DLPS
 * @{
 */

/**
 * @defgroup Light_Dlps_Exported_Types Light Dlps Exported Types
 * @brief
 * @{
 */
typedef union
{
    uint32_t dlps_ctrl;
    struct
    {
        uint32_t io_dlps_bit: 1;
        uint32_t unprov_dlps_bit: 1;
        uint32_t rsvd: 30;
    } dlps_bit;
} DLPS_Ctrl_Status;
/** @} */

/**
 * @defgroup Light_Dlps_Exported_Functions Light Dlps Exported Functions
 * @brief
 * @{
 */
void light_dlps_ctrl_init(void);
void light_io_ctrl_dlps(bool allow_enter_dlps);
void light_unprov_ctrl_dlps(bool allow_enter_dlps);
bool light_check_dlps(void);
/** @} */
/** @} */

#ifdef __cplusplus
}
#endif

#endif

/* _SWITCH_DLPS_CTRL_ */

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

