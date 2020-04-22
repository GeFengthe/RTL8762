/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of GPIO Interrupt mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
#include "rtl876x_rcc.h"
#include "rtl876x_rtc.h"
#include "rtl876x_nvic.h"
#include <trace.h>
#include <os_sched.h>
#include <app_task.h>
#include "calc.h"

/**
  * @brief  Initialize LPC peripheral.
  * @param   No parameter.
  * @return  void
  */
void RTC_init(void)
{
    RTC_DeInit();
    RTC_SetPrescaler(RTC_PRESCALER_VALUE);
    RTC_MaskINTConfig(RTC_INT_CMP0, ENABLE);
    RTC_CompINTConfig(RTC_INT_CMP0, DISABLE);

    /* Config RTC interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}
void clock_start(void)
{
    uint32_t CompareValue;
    CompareValue = RTC_GetCounter() + (32000 / (RTC_PRESCALER_VALUE + 1));
    RTC_SetComp(ClOCK_RTC_COMPARATOR, CompareValue & 0xFFFFFF);

    RTC_MaskINTConfig(RTC_INT_CMP0, DISABLE);
    RTC_CompINTConfig(RTC_INT_CMP0, ENABLE);
    RTC_RunCmd(ENABLE);
}
/**
  * @brief  demo code of operation about RTC.
  * @param   No parameter.
  * @return  void
  */
void driver_init(void)
{
    /* Initialize RTC peripheral */
    RTC_init();
    Time_init();
    clock_start();
}

int main()
{
    app_task_init();
    os_sched_start();
}



