/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    tim interrupt demo
            Timer timing to control the LED flashing .
* @details
* @author   yuan
* @date     2018-06-28
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_tim.h"

#include "trace.h"

/* Defines -------------------------------------------------------------------*/
/* P0_1 is connected to LED0 on EVB board */
#define GPIO_OUTPUT_PIN_0       P0_1
#define GPIO_PIN_OUTPUT         GPIO_GetPin(GPIO_OUTPUT_PIN_0)

/* Timer define Tim2~Tim7 */
#define TIMER_NUM       TIM6
#define TIMER_IRQN      TIMER6_IRQ

/* Timer timing config */
#define TIMING_TIME     1000000    //uint: us
#define TIMER_PERIOD    ((TIMING_TIME)*40-1)

/* Globals ------------------------------------------------------------------*/
uint8_t LED_Status = 0;

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_gpio_init(void)
{
    Pad_Config(GPIO_OUTPUT_PIN_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);

    Pinmux_Config(GPIO_OUTPUT_PIN_0, DWGPIO);
}

/**
  * @brief  Initialize GPIO peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_gpio_init(void)
{
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_PIN_OUTPUT;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_ResetBits(GPIO_PIN_OUTPUT);
}

/**
  * @brief  Initialize tim peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_timer_init(void)
{
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
    LED_Status = 0;
    board_gpio_init();
    driver_gpio_init();
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
 * @brief    Entry of APP code
 * @return   int (To avoid compile warning)
 */
int main(void)
{
    __enable_irq();

    tim_demo();

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

/**
  * @brief  TIM6 interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void Timer6_Handler(void)
{
    TIM_ClearINT(TIM6);
    TIM_Cmd(TIM6, DISABLE);
    if (!LED_Status)
    {
        GPIO_SetBits(GPIO_PIN_OUTPUT);
        LED_Status = 1;
    }
    else
    {
        GPIO_ResetBits(GPIO_PIN_OUTPUT);
        LED_Status = 0;
    }
    //Add user code here
    TIM_Cmd(TIM6, ENABLE);

}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
