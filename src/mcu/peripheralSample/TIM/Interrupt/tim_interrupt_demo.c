/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     tim_interrupt_demo.c
* @brief    tim interrupt demo
* @details
* @author   yuan
* @date     2018-06-28
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_tim.h"

#include "trace.h"

/* Defines -------------------------------------------------------------------*/
#define TIMER_NUM       TIM6
#define TIMER_IRQN      TIMER6_IRQ

#define TIMER_PERIOD    (500*4000-1)


/**
  * @brief  Initialize tim peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_timer_init(void)
{
    TIM_DeInit();
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);

    TIM_InitStruct.TIM_PWM_En = PWM_DISABLE;
    TIM_InitStruct.TIM_Period = TIMER_PERIOD ;
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_TimeBaseInit(TIMER_NUM, &TIM_InitStruct);

    /*  Enable TIMER IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIMER_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    TIM_ClearINT(TIMER_NUM);
    TIM_INTConfig(TIMER_NUM, ENABLE);
    TIM_Cmd(TIMER_NUM, ENABLE);

}

/**
  * @brief  Demo code of timer.
  * @param  No parameter.
  * @return void
*/
void timer_demo(void)
{
    driver_timer_init();

}

/**
  * @brief  Demo code of timer.
  * @param  No parameter.
  * @return void
*/
void tim_demo(void)
{
    timer_demo();
}

/**
  * @brief  TIM6 interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void Timer6_Handler(void)
{
    TIM_ClearINT(TIM6);
    TIM_Cmd(TIM6, DISABLE);
    //Add User code here

    TIM_Cmd(TIM6, ENABLE);

}

