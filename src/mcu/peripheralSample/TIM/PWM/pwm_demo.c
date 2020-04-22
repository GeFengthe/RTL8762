/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     pwm_demo.c
* @brief    tim + pwm demo and deadzone demo
* @details
* @author   renee
* @date     2017-01-23
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

/* Defines --------------------------------------------------------------*/
#define PWM_OUT_PIN             P2_2
#define PWM_OUT_P_PIN           P2_4
#define PWM_OUT_N_PIN           P2_5
#define PWM_TIMER_NUM           TIM2
//#define PWM_PINMUX_OUT          timer_pwm2
//#define PWM_PINMUX_OUT_P        PWM2_P
//#define PWM_PINMUX_OUT_N        PWM2_N
#define PWM_OUT_PIN_PINMUX      timer_pwm2
#define PWM_OUT_P_PIN_PINMUX    PWM2_P
#define PWM_OUT_N_PIN_PINMUX    PWM2_N

#define PWM_HIGH_COUNT          (10000000-1)
#define PWM_LOW_COUNT           (20000000-1)
#define PWM_DEAD_ZONE_SIZE      (200)

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_pwm_init(void)
{
    Pad_Config(PWM_OUT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(PWM_OUT_P_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(PWM_OUT_N_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    /* Normal mode */
    Pinmux_Config(PWM_OUT_PIN, PWM_OUT_PIN_PINMUX);
    /* Deadzone mode */
    Pinmux_Config(PWM_OUT_P_PIN, PWM_OUT_P_PIN_PINMUX);
    Pinmux_Config(PWM_OUT_N_PIN, PWM_OUT_N_PIN_PINMUX);

}

/**
  * @brief  Initialize tim peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_pwm_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;

    TIM_StructInit(&TIM_InitStruct);
    TIM_InitStruct.TIM_PWM_En = PWM_ENABLE;
    TIM_InitStruct.TIM_Period = 2000 * 10 - 1 ;
    TIM_InitStruct.TIM_PWM_High_Count = 1000000 - 1 ;
    TIM_InitStruct.TIM_PWM_Low_Count = 1000000 - 1 ;
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_TimeBaseInit(TIM2, &TIM_InitStruct);

    TIM_Cmd(TIM2, ENABLE);

    TIM_InitStruct.TIM_PWM_En = PWM_ENABLE;
    TIM_InitStruct.TIM_Period = 2000 * 10 - 1 ;
    TIM_InitStruct.TIM_PWM_High_Count = 1000000 -
                                        1; //should be large enough to generate a complementary PWM
    TIM_InitStruct.TIM_PWM_Low_Count = 1000000 - 1 ;
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_InitStruct.PWM_Deazone_Size = 0x10;
    TIM_InitStruct.PWMDeadZone_En = DEADZONE_ENABLE;
    TIM_TimeBaseInit(TIM3, &TIM_InitStruct);

    TIM_Cmd(TIM3, ENABLE);
}

/**
  * @brief  Demo code of operation about PWM.
  * @param  No parameter.
  * @return void
*/
void pwm_demo(void)
{
    /* Configure PAD and pinmux firstly! */
    board_pwm_init();

    /* Initialize TIM peripheral */
    driver_pwm_init();
}

/**
  * @brief  Demo code of timer.
  * @param  No parameter.
  * @return void
*/
void tim_demo(void)
{
    pwm_demo();
}



