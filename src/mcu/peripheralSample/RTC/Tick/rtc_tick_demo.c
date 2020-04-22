/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     rtc_tick_demo.c
* @brief    This file provides demo code to realize RTC tick function.
* @details
* @author   yuan
* @date     2018-05-25
* @version  v1.0
*********************************************************************************************************
*/

/* Defines ------------------------------------------------------------------*/
#define RTC_PRESCALER_VALUE     0

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_nvic.h"
#include "rtl876x_rcc.h"
#include "rtl876x_rtc.h"
#include "trace.h"

/**
  * @brief  Initialize LPC peripheral.
  * @param   No parameter.
  * @return  void
  */
void driver_rtc_init(void)
{
    RTC_DeInit();
    RTC_SetPrescaler(RTC_PRESCALER_VALUE);

    RTC_MaskINTConfig(RTC_INT_TICK, DISABLE);
    RTC_TickINTConfig(ENABLE);

    /* Config RTC interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /* Start RTC */
    RTC_RunCmd(ENABLE);
}

/**
  * @brief  Demo code of operation about RTC.
  * @param  No parameter.
  * @return void
*/
void rtc_demo(void)
{
    /* Initialize RTC peripheral */
    driver_rtc_init();

    //Add application code here
}

void RTC_Handler(void)
{
    DBG_DIRECT("RTC_Handler");

    /* RTC tick interrupt handle */
    if (RTC_GetINTStatus(RTC_INT_TICK) == SET)
    {
        // Add application code here
        DBG_DIRECT("RTC_INT_TICK");

        RTC_ClearTickINT();
    }
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

