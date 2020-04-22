/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     fm25q16a.c
* @brief    This file provides demo code of adc continuous mode.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "fm25q16a.h"


/* Globals ------------------------------------------------------------------*/
uint8_t Flash_Data[4];


void fm25q16a_read_data(uint8_t *pCmd, uint16_t vDatalen)
{
    SPI_SendBuffer(SPI0, pCmd, 1);
}
