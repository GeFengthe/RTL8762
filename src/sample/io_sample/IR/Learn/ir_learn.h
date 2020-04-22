/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     ir_learn.h
* @brief
* @details
* @author   yuan
* @date     2019-01-24
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __IR_LEARN_H
#define __IR_LEARN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "board.h"


#if (IR_FUN_EN && IR_FUN_LEARN_EN)

/* Includes ------------------------------------------------------------------*/
#include "ir_trans_rx.h"
#include "ir_learn_config.h"

/* Defines -------------------------------------------------------------*/
/* Filter threshold value. If time interval< 200us(10KHz), treat it as a part of a carrier time. */
/* IR learn carrier freqency between 10KHz and 2.5MHz */
#define TIME_HIGHEST_VALUE          (200*IR_LEARN_FREQ/1000)

/* IR data mask */
#define IR_DATA_MSK                 ((uint32_t)0x7FFFFFFFUL)
/* Carrier waveform data type select */
#define IR_CARRIER_DATA_TYPE        ((uint32_t)0x80000000UL)

/** @defgroup IR_LEARN_Exported_Types IR Learn Exported Types
  * @{
  */
typedef enum
{
    IR_LEARN_OK,                            /**< IR learn ok: learning */
    IR_LEARN_EXIT,                          /**< IR learn exit: complete IR learn */
    IR_LEARN_WAVEFORM_ERR,                  /**< IR learn waveform error */
    IR_LEARN_CARRIRE_DATA_HANDLE_ERR,       /**< IR learn carrier data compensation error */
    IR_LEARN_NO_CARRIRE_DATA_HANDLE_ERR,    /**< IR learn carrier data compensation error */
    IR_LEARN_EXCEED_SIZE,                   /**< IR learn exceed maximum size */
    IR_LEARN_DUTY_CYCLE_ERR_NO_SAMPLE,      /**< IR learn duty cycle error: samples were not collceted */
    IR_LEARN_DUTY_CYCLE_ERR_NO_VALID_DATA,  /**< IR learn duty cycle error: have no valid data */
    //IR_LEARN_DUTY_CYCLE_ERR_NO_Carrier_TIME,/**< IR learn duty cycle error: have no valid carrier high time */
} IR_Learn_Status;

/**
  * @brief  IR learn data structure
  */
typedef struct
{
    uint32_t    ir_buf[IR_LEARN_WAVEFORM_SIZE_MAX];
#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
    uint32_t    last_handle_data;
#endif
    uint16_t    buf_index;
    uint16_t    carrier_info_buf[IR_LEARN_WAVEFORM_SIZE_MAX / 2 + 2];
    uint16_t    carrier_info_idx;
    uint8_t     is_carrier;

#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
    uint32_t    carrier_time;
    float       duty_cycle;
#endif
    float       freq;
} IR_Learn_TypeDef;

/** End of IR_LEARN_Exported_Types
  * @}
  */

void board_ir_learn_init(void);
void ir_learn_deinit(void);
void ir_learn_init(void);
IR_Learn_Status ir_learn_freq(IR_Learn_TypeDef *pIR_Packet);
void ir_learn_data_convert(IR_Learn_TypeDef *pIR_Packet);
IR_Learn_Status ir_learn_decode(IR_Learn_TypeDef *pIR_Packet);

#endif /* (IR_FUN_EN && IR_FUN_LEARN_EN) */

#ifdef __cplusplus
}
#endif

#endif /* __IR_LEARN_H */


/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/

