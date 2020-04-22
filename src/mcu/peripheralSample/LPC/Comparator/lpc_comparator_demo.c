/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     lpc_demo.c
* @brief    This file provides demo code to detect voltage.
* @details
* @author   yuan
* @date     2018-06-27
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_lpc.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#define LPC_CAPTURE_PIN                 P2_4
#define LPC_CAPTURE_CHANNEL             LPC_CHANNEL_P2_4

#define LPC_VOLTAGE_DETECT_EDGE         LPC_Vin_Over_Vth;
#define LPC_VOLTAGE_DETECT_THRESHOLD    LPC_3200_mV;

#define LPC_COMP_VALUE                  0x01

/**
  * @brief  initialization of pinmux settings and pad settings.
  * @param   No parameter.
  * @return  void
  */
void board_lpc_init(void)
{
    Pad_Config(LPC_CAPTURE_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);
    Pinmux_Config(LPC_CAPTURE_PIN, IDLE_MODE);
}

/**
  * @brief  Initialize LPC peripheral.
  * @param   No parameter.
  * @return  void
  */
void driver_lpc_init(void)
{
    LPC_InitTypeDef LPC_InitStruct;

    LPC_StructInit(&LPC_InitStruct);
    LPC_InitStruct.LPC_Channel   = LPC_CAPTURE_CHANNEL;
    LPC_InitStruct.LPC_Edge      = LPC_VOLTAGE_DETECT_EDGE;
    LPC_InitStruct.LPC_Threshold = LPC_VOLTAGE_DETECT_THRESHOLD;
    LPC_Init(&LPC_InitStruct);
    LPC_Cmd(ENABLE);

    LPC_CounterReset();
    LPC_WriteComparator(LPC_COMP_VALUE);
    LPC_CounterCmd(ENABLE);
    LPC_INTConfig(LPC_INT_COUNT_COMP, ENABLE);

    //LPC_INTConfig(LPC_INT_VOLTAGE_COMP, ENABLE);
    RTC_SystemWakeupConfig(ENABLE);

    /* Config LPC interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Demo code of operation about lpc.
  * @param  No parameter.
  * @return void
*/
void lpc_demo(void)
{
    /* Configure PAD and pinmux firstly! */
    board_lpc_init();

    /* Initialize LPC peripheral */
    driver_lpc_init();
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
        DBG_DIRECT("LPC_INT_COUNT_COMP");
        LPC_WriteComparator(LPC_ReadComparator() + LPC_COMP_VALUE);
        LPC_ClearINTPendingBit(LPC_INT_COUNT_COMP);
    }
}

/******************* (C) COPYRIGHT 2016 Realtek Semiconductor Corporation *****END OF FILE****/

