/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_adc.c
* @brief    This file provides demo code of adc continuous mode.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stk8321.h"


/* Globals ------------------------------------------------------------------*/
GSensor_Data_TypeDef GSensor_Data;

uint8_t stk8321_id_get(void)
{
    uint8_t stk8321_chip_id = 0;
    uint8_t reg_addr = STK8321_REG_CHIP_ID;
    I2C_RepeatRead(I2C0, &reg_addr, 1, &stk8321_chip_id, 1);
    return stk8321_chip_id;
}

void stk8321_outdata_get(void)
{
    uint8_t reg_addr = STK8321_REG_X_OUT_LOW;
    I2C_RepeatRead(I2C0, &reg_addr, 1, GSensor_Data.OutData, 6);
}
