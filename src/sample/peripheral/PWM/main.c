/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of PWM mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
/* Include ---------------------------------------------------------------*/
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_wdg.h"
#include "rtl876x_nvic.h"
#include "rtl876x_tim.h"
#include "trace.h"
#include "board.h"
#include <os_sched.h>
/* Defines --------------------------------------------------------------*/

/**
  * @brief  initialization of pinmux settings and pad settings.
  * @param   No parameter.
  * @return  void
  */
void Board_PWM_init(void)
{
    Pad_Config(PWM_OUT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(PWM_OUT, timer_pwm2);
}

/*when clock source divider 1, below setting can get max 20M clock PWM out,duty 1:1*/
#define PWM_HIGH_CNT  1
#define PWM_LOW_CNT   1

/**
  * @brief  Initialize PWM peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_PWM_init(void)
{

    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;

    TIM_StructInit(&TIM_InitStruct);
    TIM_InitStruct.TIM_PWM_En = PWM_ENABLE;
    TIM_InitStruct.TIM_PWM_High_Count = PWM_HIGH_CNT - 1;
    TIM_InitStruct.TIM_PWM_Low_Count  = PWM_LOW_CNT - 1 ;
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    /*Timer source Clock is 40M,40M/1 = 40M*/
    TIM_InitStruct.TIM_SOURCE_DIV = TIM_CLOCK_DIVIDER_1;
    TIM_TimeBaseInit(TIM2, &TIM_InitStruct);

    TIM_Cmd(TIM2, ENABLE);
}
int main()
{
    WDG_Disable();
    __enable_irq();
    Board_PWM_init();
    Driver_PWM_init();
    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
    }
}
