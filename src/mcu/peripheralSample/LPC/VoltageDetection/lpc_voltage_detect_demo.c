/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     lpc_voltagedet_demo.c
* @brief    This file provides demo code to detect voltage.
* @details
* @author   yuan
* @date     2018-05-25
* @version  v0.1
*********************************************************************************************************
*/

/* Defines ------------------------------------------------------------------*/
#define LPC_CAPTURE_PIN                 P2_5
#define LPC_CAPTURE_CHANNEL             LPC_CHANNEL_P2_5

#define LPC_VOLTAGE_DETECT_EDGE         LPC_Vin_Over_Vth;
#define LPC_VOLTAGE_DETECT_THRESHOLD    LPC_3200_mV;


/* Includes ------------------------------------------------------------------*/
#include "rtl876x_lpc.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "trace.h"

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

    /* Enable voltage detection interrupt.If Vin<Vth, cause this interrupt */
    LPC_INTConfig(LPC_INT_VOLTAGE_COMP, ENABLE);

    /* Config LPC interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = LPCOMP_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    DBG_DIRECT("LPC init done!");

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
  * @brief  LPC battery detection interrupt handle function.
  * @param  None.
  * @return None.
  */
void LPCOMP_Handler(void)
{
    DBG_DIRECT("LPC_Handler");
    //Add Application code here
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

