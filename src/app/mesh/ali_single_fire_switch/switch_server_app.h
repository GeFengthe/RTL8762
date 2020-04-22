/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      switch_server_app.h
* @brief     Smart mesh switch application header file
* @details
* @author    elliot chen
* @date      2018-9-14
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _SWITCH_SERVER_APP_H
#define _SWITCH_SERVER_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mesh_api.h"

#define SWITCH_MODELS_ELEMENT_NUM      3
#define SWITCH_MODELS_ELEMENT_IDX      0

void switch_server_models_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _SWITCH_SERVER_APP_H */
