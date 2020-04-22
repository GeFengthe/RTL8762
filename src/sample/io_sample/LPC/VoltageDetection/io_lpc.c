/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_lpc.c
* @brief    This file provides demo code of lpc voltage detect.
            Low power interruption will occur when the voltage on the pin(P2_0~P2_7) is below the set value.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "io_lpc.h"

#include "app_task.h"

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
}

/**
  * @brief  Config lpc nvic.
  * @param  No parameter.
  * @return void
  */
void nvic_lpc_config(void)
{
    /* Config LPC interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = LPCOMP_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /* Enable voltage detection interrupt.If Vin<Vth, cause this interrupt */
    LPC_INTConfig(LPC_INT_VOLTAGE_COMP, ENABLE);
}

/**
  * @brief  LPC voltage detection interrupt handle function.
  * @param  None.
  * @return None.
  */
void LPCOMP_Handler(void)
{
    LPC_INTConfig(LPC_INT_VOLTAGE_COMP, DISABLE);
    T_IO_MSG int_lpc_msg;

    int_lpc_msg.type = IO_MSG_TYPE_BAT_LPC;
    if (false == app_send_msg_to_apptask(&int_lpc_msg))
    {
        APP_PRINT_ERROR0("[io_lpc] LPCOMP_Handler: Send int_lpc_msg failed!");
        return;
    }
    //Add Application code here
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
