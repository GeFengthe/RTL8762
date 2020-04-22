/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
* @file     light_config.h
* @brief    Head file for light config.
* @details  Data structs and external functions declaration.
* @author   hector_huang
* @date     2018-11-16
* @version  v0.1
* *************************************************************************************
*/

#ifndef _LIGHT_CONFIG_H_
#define _LIGHT_CONFIG_H_

#include "ftl_map.h"

/**
 * @addtogroup LIGHT_CONFIG
 * @{
 */

/**
 * @defgroup Light_Config_Exported_Macros Light Config Exported Macros
 * @brief
 * @{
 */

/* light types */
#define LIGHT_UNKNOWN       0
#define LIGHT_LIGHTNESS     1
#define LIGHT_CW            2
#define LIGHT_RGB           3
#define LIGHT_CWRGB         4

/** @brief set light type */
#define LIGHT_TYPE                         LIGHT_CW

#define LIGHT_FLASH_PARAMS_APP_OFFSET      FTL_MAP_DIMMABLE_LIGHT_OFFSET
#define LIGHT_POWER_ON_COUNT               5    //!< close the light LIGHT_POWER_ON_COUNT times to reset
#define LIGHT_POWER_ON_TIME                8000 //!< millisecond

/** @brief set this value to 1 if need to use ali certification */
#define MESH_ALI_CERTIFICATION             0
/** @} */
/** @} */

#endif /** _LIGHT_CONFIG_H_ */


