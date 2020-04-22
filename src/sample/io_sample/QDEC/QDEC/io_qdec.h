/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_qdec.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __IO_QDEC_H
#define __IO_QDEC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_qdec.h"
#include "rtl876x_rcc.h"

#include "board.h"

#include "app_msg.h"


/* Globals ------------------------------------------------------------------*/

/* QDEC data struct*/
typedef struct
{

    uint16_t AxisDirection;
    uint16_t AxisCount;
} QDEC_Data_TypeDef;

extern QDEC_Data_TypeDef   Y_Axis_Data;

void board_qdec_init(void);
void driver_qdec_init(void);
void io_handle_qdec_msg(T_IO_MSG *io_qdec_msg);


#ifdef __cplusplus
}
#endif

#endif

