/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     wdg.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __WDG_H
#define __WDG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_rcc.h"
#include "rtl876x_wdg.h"

/* Defines ------------------------------------------------------------------*/

void driver_wdg_init(void);
void wdg_feed(void);


#ifdef __cplusplus
}
#endif

#endif

