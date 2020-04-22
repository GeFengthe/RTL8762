/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     i2s_dmo
* @brief        This file provides demo code of I2S comunication.
* @details
* @author   elliot chen
* @date         2017-12-13
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_i2s.h"
#include "rtl876x_nvic.h"
#include "rtl876x_wdg.h"
#include "board.h"
#include "os_sched.h"
#include "trace.h"
/* Defines -------------------------------------------------------------------*/


#define I2S_USR_BCLK                    BCLK_SPORT1
#define I2S_USR_LRC                     LRC_SPORT1
#define I2S_USR_DACDAT                  DACDAT_SPORT1
#define APB_I2S_USR                     APBPeriph_I2S1
#define APB_I2S_USR_CLK                 APBPeriph_I2S1_CLOCK
#define I2S_USR                         I2S1
#define I2S_USR_IRQn                    SPORT1_TX_IRQn
#define I2S_USRIntrHandler              I2S1_TX_Handler

/* Globals --------------------------------------------------------------------*/
uint32_t i2s_send_data = 0x12345678;

/**
  * @brief  initialization of pinmux settings and pad settings.
  * @param   No parameter.
  * @return  void
  */
void Board_I2S_Init(void)
{
    Pad_Config(I2S_BCLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(I2S_LR_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(I2S_DATA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);

    Pinmux_Config(I2S_BCLK_PIN, I2S_USR_BCLK);
    Pinmux_Config(I2S_LR_PIN, I2S_USR_LRC);
    Pinmux_Config(I2S_DATA_PIN, I2S_USR_DACDAT);
}

/**
  * @brief  Initialize I2S peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_I2S_Init(void)
{
    RCC_PeriphClockCmd(APB_I2S_USR, APB_I2S_USR_CLK, ENABLE);

    I2S_InitTypeDef I2S_InitStruct;
    I2S_StructInit(&I2S_InitStruct);
    I2S_InitStruct.I2S_ClockSource      = I2S_CLK_40M;
    I2S_InitStruct.I2S_BClockMi         = 0x2710;/* <!BCLK = 1K */
    I2S_InitStruct.I2S_BClockNi         = 0x10;
    I2S_InitStruct.I2S_DeviceMode       = I2S_DeviceMode_Master;
    I2S_InitStruct.I2S_ChannelType      = I2S_Channel_Mono;
    I2S_InitStruct.I2S_DataFormat       = I2S_Mode;
    I2S_Init(I2S_USR, &I2S_InitStruct);
    I2S_Cmd(I2S_USR, I2S_MODE_TX | I2S_MODE_RX, ENABLE);

    /* Enable I2S0 Tx interrupt */
    I2S_INTConfig(I2S_USR, I2S_INT_TF_EMPTY, ENABLE);
    /* Config NVIC */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = I2S_USR_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}
int main()
{
    WDG_Disable();
    __enable_irq();
    Board_I2S_Init();
    Driver_I2S_Init();
    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
    }
}
/**
* @brief  I2S interrupt handler function.
* @param   No parameter.
* @return  void
*/
void I2S_USRIntrHandler(void)
{
    if (I2S_GetINTStatus(I2S_USR, I2S_INT_TF_EMPTY))
    {
        I2S_SendData(I2S_USR, i2s_send_data++);
    }
}

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

