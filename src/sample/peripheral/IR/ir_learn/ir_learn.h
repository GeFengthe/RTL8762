/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file         ir_learn.h
* @brief
* @details
* @author    elliot chen
* @date       2017-11-16
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
#include "rtl876x.h"
#include "ir_learn_config.h"
#include "data_trans.h"
#include "rtl876x_tim.h"

#if (IR_FUN && IR_LEARN_MODE)
/* Defines -------------------------------------------------------------------*/

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
    IR_LEARN_DUTY_CYCLE_ENERR_NO_SAMPLE,    /**< IR learn duty cycle error: samples were not collceted */
    IR_LEARN_DUTY_CYCLE_ERR_NO_valid_DATA,  /**< IR learn duty cycle error: have no valid data */
    //IR_LEARN_DUTY_CYCLE_ERR_NO_Carrier_TIME,/**< IR learn duty cycle error: have no valid carrier high time */
} IR_Learn_Status;

/**
  * @brief  IR learn data structure
  */
typedef struct
{
    LoopQueue_BufType   ir_buf[IR_LEARN_WAVE_MAX_SIZE];
#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
    LoopQueue_BufType   last_handle_data;
#endif
    uint16_t            buf_index;
    uint16_t            carrier_info_buf[IR_LEARN_WAVE_MAX_SIZE / 2 + 2];
    uint16_t            carrier_info_idx;
    uint8_t             is_carrier;

#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
    //LoopQueue_BufType carrier_high_time;
    //LoopQueue_BufType carrier_low_time;
    LoopQueue_BufType   carrier_time;
    float               duty_cycle;
#endif
    float               freq;
} IR_LearnTypeDef;

/** End of IR_LEARN_Exported_Types
    * @}
    */

void IR_Learn_DeInit(void);
void IR_Learn_Init(void);
IR_Learn_Status IR_Learn_Decode(IR_LearnTypeDef *pIR_Packet);
IR_Learn_Status IR_Learn_Freq(IR_LearnTypeDef *pIR_Packet);
void IR_Learn_ConvertData(IR_LearnTypeDef *pIR_Packet);

#endif

#ifdef __cplusplus
}
#endif

#endif /*__IR_LEARN_H*/


/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

