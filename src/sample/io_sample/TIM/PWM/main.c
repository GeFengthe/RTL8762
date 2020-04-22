/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    PWM demo and complementary output demo of PWM.
* @details
* @author   yuan
* @date     2019-02-18
* @version  v1.0
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
/** Define pwm output pin.
  * P0_1 is connected to LED0 on the EVB.
  */
#define PWM_OUT_PIN             P0_1
/** Define pwm complementary output pin.
  * P0_2 is connected to LED1 on the EVB.
  */
#define PWM_OUT_P_PIN           P0_2
#define PWM_OUT_N_PIN           P2_2


/** Define TIM num and pinmux of PWM output.
  * Timer2 to Timer7 all have PWM output function.
  * But only Timer2 and Timer3 support PWM complementary output function, other timers do not support it.
  * When the complementary output function is turned on, when the dead time is not set,
  * the complementary output PWM2_P of timer2 is in phase with PWM2, and PWM2_N and PWM2 are  inverted.
  * Timer3 is the same as timer2.
  */
#define PWM_TIMER_NUM           TIM2
#define PWM_OUT_PIN_PINMUX      timer_pwm2
#define PWM_OUT_P_PIN_PINMUX    PWM2_P
#define PWM_OUT_N_PIN_PINMUX    PWM2_N

/* Config PWM_PERIOD and PWM_DUTY_CYCLE */
#define PWM_PERIOD              1000000 //uint:us
#define PWM_DUTY_CYCLE          50      //uint:percent
/* PWM_HIGH_COUNT =  */
#define PWM_HIGH_COUNT          (((PWM_PERIOD)*((PWM_DUTY_CYCLE*40)/100))-1)    //PWM CLOCK = 40000000
#define PWM_LOW_COUNT           (((PWM_PERIOD)*(((100-PWM_DUTY_CYCLE)*40)/100))-1)

/** Config PWM_DEAD_ZONE_TIME. Max 8ms.
  * When dead time is set, PWM2_P and PWM2_N will
  * delay one dead time longer than PWM2 at high output level.
  * When the dead_zone time is longer than the high-level time of PWM,
  * PWM_P will always output low-level.
  * When the dead_zone time is longer than the low-level time of PWM,
  * PWM_N will always output low-level.
  */
#define PWM_DEAD_ZONE_TIME      1 //uint:ms
#define PWM_DEAD_ZONE_SIZE      ((PWM_DEAD_ZONE_TIME)*32-1)

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
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;

    TIM_StructInit(&TIM_InitStruct);
    TIM_InitStruct.TIM_PWM_En   = PWM_ENABLE;
    TIM_InitStruct.TIM_Mode     = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_PWM_High_Count   = PWM_HIGH_COUNT;
    TIM_InitStruct.TIM_PWM_Low_Count    = PWM_LOW_COUNT;
    TIM_InitStruct.PWM_Deazone_Size     = PWM_DEAD_ZONE_SIZE;
    TIM_InitStruct.PWMDeadZone_En       = DEADZONE_ENABLE;  //enable to use pwn p/n output
    TIM_TimeBaseInit(TIM2, &TIM_InitStruct);

    TIM_Cmd(TIM2, ENABLE);
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
  * @brief    Entry of app code
  * @return   int (To avoid compile warning)
  */
int main(void)
{
    pwm_demo();

    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();

    }
}

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/
