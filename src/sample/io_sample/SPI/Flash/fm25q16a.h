/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     fm25q16a.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __FM25Q16A_H
#define __FM25Q16A_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_spi.h"

/* Defines ------------------------------------------------------------------*/
extern uint8_t Flash_Data[4];

void fm25q16a_read_data(uint8_t *pCmd, uint16_t vDatalen);

#ifdef __cplusplus
}
#endif

#endif

