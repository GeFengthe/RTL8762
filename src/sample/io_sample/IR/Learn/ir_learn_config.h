/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     ir_learn_config.h
* @brief    ir learn confguration header file
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

/* Defines ------------------------------------------------------------------*/

/**
  * @brief  No carrier waveform maximum time is 6ms
  */
#define IR_LEARN_NO_WAVEFORM_TIME_MAX           ((uint32_t)IR_LEARN_FREQ*6)
#define IR_LEARN_STOP_TIME                      (IR_LEARN_NO_WAVEFORM_TIME_MAX*0.95)

/**
  * @brief  Enable IR duty cycle learning or not
  */
#define IR_LEARN_DUTY_CYCLE_SUPPORT
#define IR_LEARN_DUTY_CYCLE_SAMPLE_SIZE_MAX     6

/**
  * @brief  Enable filter IR freqency or not
  */
#define IR_LEARN_FREQ_FILTER_EN                 1
/**
  * @brief  Software adjustment for sending IR learn data
  */
#define IR_LEARN_SOFTWARE_ADJUST_EN             1




#ifdef __cplusplus
}
#endif

#endif /* !defined (__IR_LEARN_CONFIG_H) */

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

