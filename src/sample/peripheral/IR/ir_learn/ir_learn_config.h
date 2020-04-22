/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      ir_learn_config.h
* @brief    ir learn confguration header file.
* @details
* @author   elliot chen
* @date     2017-11-16
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __IR_LEARN_CONFIG_H
#define __IR_LEARN_CONFIG_H

#ifdef __cplusplus
extern "C"  {
#endif

/* Includes ------------------------------------------------------------------*/

/**
  * @brief  loop queue buffer type definition
  */
typedef unsigned char                       BOOL;
typedef unsigned int                        uint32_t;
typedef uint32_t                            LoopQueue_BufType;
#define TRUE                                1
#define FALSE                               0

#define IR_FUN                              1
#define IR_LEARN_MODE                       1
#define IR_LEARN_PIN                        P4_1

/**
  * @brief  Enable print log or not
  */
#define PRINT_LOG

/**
  * @brief  Board configuration by user
  */

#define IR_RX_FIFO_THR_LEVEL                20
#define IR_LEARN_FREQ                       40000
#define  IR_LEARN_WAVE_MAX_SIZE             100
#define IR_LEARN_TIMEOUT                    (20*1000)  /* 20s */
/**
  * @brief  No carrier waveform maximum time is 6ms
  */
#define IR_LEARN_MAX_NO_WAVEFORM_TIME       ((uint32_t)IR_LEARN_FREQ*6)
#define IR_LEARN_STOP_TIME                  (IR_LEARN_MAX_NO_WAVEFORM_TIME*0.95)

/**
  * @brief  Software adjustment for sending IR learn data
  */

#define SOFTWARE_ADJUSTMENT

/**
  * @brief  Prevent data truncation
  */

#define TRUNCATION_COMPENSATION             (0.01)

/**
  * @brief  Enable filter IR freqency or not
  */
#define FILTER_IR_LEARN_FREQ

/**
  * @brief  Enable IR duty cycle learning or not
  */
#define IR_LEARN_DUTY_CYCLE_SUPPORT
#define IR_LEARN_DUTY_CYCLE_SAMPLE_MAX_SIZE 6

//#define USE_DATA_TRANS_MEMCPY

/**
  * @brief  IR loop queue parameters
  */
#define LOOP_QUEUE_MAX_SIZE                 (1024)
#define QUEUE_CAPABILITY                    (LOOP_QUEUE_MAX_SIZE-1)

#ifdef __cplusplus
}
#endif

#endif /* !defined (__IR_LEARN_CONFIG_H) */

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

