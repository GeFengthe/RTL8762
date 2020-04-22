/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     pos.h
  * @brief    Head file for using posture service.
  * @details  Posture data.
  * @author
  * @date
  * @version  v1.0
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _POS_H_
#define _POS_H_

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

/* Add Includes here */
#include "profile_server.h"


/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @defgroup Posture Exported Macros
  * @brief
  * @{
  */

#define POS_READ_LEVEL  1
#define POS_NOTIFY_LEVEL_ENABLE  1
#define POS_NOTIFY_LEVEL_DISABLE  2

/** End of POS_Exported_Macros
* @}
*/
typedef void *TimerHandle_t;
extern TimerHandle_t xTimerPOS;
/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup POS_Exported_Types POS Exported Types
  * @brief
  * @{
  */

/**
*  @brief Posture service parameter type
*/
typedef enum
{
    POS_PARAM_LEVEL = 0x01,
} T_POS_PARAM_TYPE;

/**
*  @brief set posture service parameter upstream message data
*/
typedef union
{
    uint8_t notification_indification_index;
    uint8_t read_value_index;
} T_POS_UPSTREAM_MSG_DATA;

/**
*  @brief set posture service parameter upstream callback data
*/
typedef struct
{
    uint8_t                 conn_id;
    T_SERVICE_CALLBACK_TYPE msg_type;
    T_POS_UPSTREAM_MSG_DATA msg_data;
} T_POS_CALLBACK_DATA;

/** End of POS_Exported_Types
* @}
*/


/*============================================================================*
 *                         Functions
 *============================================================================*/

T_SERVER_ID pos_add_service(void *p_func);
bool pos_set_parameter(T_POS_PARAM_TYPE param_type, uint8_t length, uint8_t *p_value);
bool pos_value_notify(uint8_t conn_id, T_SERVER_ID service_id, uint8_t posture_level);
bool pos_value_read_confirm(uint8_t conn_id, T_SERVER_ID service_id,
                            uint8_t posture_level);
void vTimerPOSCallback(TimerHandle_t pxTimer);
/** @} End of POS_Exported_Functions */

/** @} End of POS */


#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif  /* _POS_H_ */
