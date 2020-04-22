/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     lpc_demo.c
* @brief        This file provides demo code to detect voltage.
* @details
* @author   elliot chen
* @date         2016-11-30
* @version  v1.0
*********************************************************************************************************
*/

/* Defines ------------------------------------------------------------------*/
#define LPC_CAPTURE_PIN         1
#define LPC_COMP_VALUE          0x100

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_rcc.h"
#include "rtl876x_lpc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_nvic.h"
#include "board.h"
#include "trace.h"
#include "os_sched.h"

/**
  * @brief  initialization of pinmux settings and pad settings.
  * @param   No parameter.
  * @return  void
  */
void Board_LPC_Init(void)
{
    Pad_Config(LPC_CAPTURE_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);
}

/**
  * @brief  Initialize LPC peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_LPC_Init(void)
{
    LPC_InitTypeDef LPC_InitStruct;
    LPC_StructInit(&LPC_InitStruct);
    LPC_InitStruct.LPC_Channel   = LPC_CAPTURE_PIN;
    LPC_InitStruct.LPC_Edge      = LPC_Vin_Over_Vth;
    LPC_InitStruct.LPC_Threshold = LPC_2000_mV;
    LPC_Init(&LPC_InitStruct);
    LPC_Cmd(ENABLE);

    LPC_CounterReset();
    LPC_WriteComparator(LPC_COMP_VALUE);
    LPC_CounterCmd(ENABLE);
    LPC_INTConfig(LPC_INT_COUNT_COMP, ENABLE);
    //LPC_INTConfig(LPC_INT_VOLTAGE_COMP, ENABLE);
    RTC_SystemWakeupConfig(ENABLE);
    DBG_DIRECT("%d", LPC_ReadComparator());
    /* Config LPC interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  demo code of operation about LPC.
  * @param   No parameter.
  * @return  void
  */
void LPC_SampleCode(void)
{
    /* Configure PAD and pinmux firstly! */
    Board_LPC_Init();

    /* Initialize LPC peripheral */
    Driver_LPC_Init();
}
int main()
{
    LPC_SampleCode();
    os_sched_start();



}
/**
  * @brief  RTC interrupt handle function.
  * @param  None.
  * @return None.
  */
void RTC_Handler(void)
{
    DBG_DIRECT("RTC_Handler");
    /* LPC counter comparator interrupt */
    if (LPC_GetINTStatus(LPC_INT_COUNT_COMP) == SET)
    {
        LPC_WriteComparator(LPC_ReadComparator() + LPC_COMP_VALUE);
        LPC_ClearINTPendingBit(LPC_INT_COUNT_COMP);
    }
}

/******************* (C) COPYRIGHT 2016 Realtek Semiconductor Corporation *****END OF FILE****/

