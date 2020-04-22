/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      switch_flash_mgr.h
* @brief     header file of flash manager implementation
* @details
* @author    elliot chen
* @date      2018-09-17
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _SWITCH_FLASH_MGR_
#define _SWITCH_FLASH_MGR_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "ftl.h"
#include "platform_macros.h"
#include "mesh_api.h"

/* Defines ------------------------------------------------------------------*/

#define FLASH_PARAMS_APP_SWITCH_OFFSET                  1900 //!< Shall be bigger than or equal to the size of mesh stack flash usage
#define SWITCH_POWER_ON_COUNT                           3 //!< close the light LIGHT_POWER_ON_COUNT times to reset
#define SWITCH_POWER_ON_TIME_OUT                        8000//!< millisecond

typedef struct
{
    uint8_t count;
    uint8_t used;
    uint8_t padding[2];
} flash_switch_power_on_count_t;

typedef struct
{
    flash_switch_power_on_count_t power_on_count;
} flash_switch_param_t;

typedef enum
{
    FLASH_SWITCH_PARAM_TYPE_SWITCH_STATE,
    FLASH_SWITCH_PARAM_TYPE_POWER_ON_COUNT
} flash_switch_param_type_t;

void switch_flash_store(flash_switch_param_type_t type, uint8_t used);
bool switch_flash_restore(void);

#ifdef __cplusplus
}
#endif

#endif /*_SWITCH_FLASH_MGR_*/

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
