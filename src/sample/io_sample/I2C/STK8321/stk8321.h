/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_adc.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __STK8321_H
#define __STK8321_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_i2c.h"



/* Defines ------------------------------------------------------------------*/
#define STK8321_ADDRESS             0x0F
#define STK8321_REG_CHIP_ID         0x00
#define STK8321_REG_X_OUT_LOW       0x02
#define STK8321_REG_X_OUT_HIGH      0x03
#define STK8321_REG_Y_OUT_LOW       0x04
#define STK8321_REG_Y_OUT_HIGH      0x05
#define STK8321_REG_Z_OUT_LOW       0x06
#define STK8321_REG_Z_OUT_HIGH      0x07

typedef struct
{
//    uint8_t XOut[2];
//    uint8_t YOut[2];
//    uint8_t ZOut[2];
    uint8_t RegAddr;
    uint8_t OutData[6];
    uint16_t XData;
    uint16_t YData;
    uint16_t ZData;
} GSensor_Data_TypeDef;

extern GSensor_Data_TypeDef GSensor_Data;

uint8_t stk8321_id_get(void);
void stk8321_outdata_get(void);

#ifdef __cplusplus
}
#endif

#endif

