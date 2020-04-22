/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_tim.h
* @brief
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

#ifndef __IO_TIM_H
#define __IO_TIM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_nvic.h"
#include "rtl876x_rcc.h"
#include "rtl876x_tim.h"

#include "board.h"

/* Defines ------------------------------------------------------------------*/
/* Timer define Tim2~Tim7 */
#define TIMER_NUM       TIM6
#define TIMER_IRQN      TIMER6_IRQ

/* Timer timing config */
/* Timing time of timer should be less than watchdog reset time */
#define TIMING_TIME     5000000    //uint: us, 5s here
#define TIMER_PERIOD    ((TIMING_TIME)*40-1)

/* Globals ------------------------------------------------------------------*/
extern uint32_t Timer_Period;

//void board_timer_init(void);
void driver_timer_init(void);
void timer_cmd(FunctionalState vNewStatus);


#ifdef __cplusplus
}
#endif

#endif

