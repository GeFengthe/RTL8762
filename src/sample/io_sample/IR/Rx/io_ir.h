/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_ir.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __IO_IR_H
#define __IO_IR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_ir.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"

#include "board.h"

#include "app_msg.h"


/* Defines ------------------------------------------------------------------*/
#define IR_DATA_SIZE_MAX                100

#define IR_RX_FIFO_THR_LEVEL            30

///**  @brief IO subtype definitions for @ref IO_MSG_TYPE_IR type */
//typedef enum
//{
//    IO_MSG_IR_RX_RF_LEVEL,
//    IO_MSG_IR_RX_CNT_THR,
//} T_IO_MSG_TYPE_IR;

/**
  * @brief  IR data structure definition
  */
typedef struct
{
    /* Unit of carrierFreq is KHz */
    uint8_t     CarrierFreq;
    uint32_t    DataBuf[IR_DATA_SIZE_MAX];
    uint16_t    DataLen;
} IR_Data_TypeDef;

void global_data_ir_init(void);
void board_ir_init(void);
void driver_ir_init(void);
void ir_demo(void);
void io_handle_ir_msg(T_IO_MSG *io_ir_msg);


#ifdef __cplusplus
}
#endif

#endif

