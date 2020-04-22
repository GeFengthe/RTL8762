/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_tim.c
* @brief    This file provides demo code of gpio interrupt mode.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "io_tim.h"

#include "app_task.h"

uint32_t Timer_Period = 0;

/**
  * @brief  Initialize TIM peripheral.
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
}

void timer_cmd(FunctionalState vNewStatus)
{
    if (vNewStatus != DISABLE)
    {
        TIM_Cmd(TIMER_NUM, ENABLE);
    }
    else
    {
        TIM_Cmd(TIMER_NUM, DISABLE);
        TIM_INTConfig(TIMER_NUM, DISABLE);
    }
}

/**
  * @brief  TIM6 interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void Timer6_Handler(void)
{
//    APP_PRINT_INFO0("Timer6_Handler");
    TIM_ClearINT(TIM6);
    TIM_Cmd(TIM6, DISABLE);

    T_IO_MSG int_timer_msg;

    int_timer_msg.type = IO_MSG_TYPE_TIMER;
    int_timer_msg.subtype = 0;
    if (false == app_send_msg_to_apptask(&int_timer_msg))
    {
        APP_PRINT_ERROR0("[io_tim] Timer6_Handler: Send int_timer_msg failed!");
        //Add user code here!
        return;
    }
    /* Modify the timing period, overtime reset. */
    APP_PRINT_ERROR0("[io_tim] Timer6_Handler: Change period!");
    Timer_Period = 440000000;//11s,
    TIM_ChangePeriod(TIM6, Timer_Period);
    TIM_Cmd(TIM6, ENABLE);
}
/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
