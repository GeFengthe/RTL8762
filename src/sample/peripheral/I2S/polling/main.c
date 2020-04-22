/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     i2s_dmo.c
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

#define I2S_USR_BCLK                    BCLK_SPORT0
#define I2S_USR_LRC                     LRC_SPORT0
#define I2S_USR_DACDAT                  DACDAT_SPORT0
#define APB_I2S_USR                     APBPeriph_I2S0
#define APB_I2S_USR_CLK                 APBPeriph_I2S0_CLOCK
#define I2S_USR                         I2S0
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
    I2S_InitStruct.I2S_DataWidth        = I2S_Width_16Bits;
    I2S_InitStruct.I2S_DataFormat       = I2S_Mode;
    I2S_InitStruct.I2S_DMACmd           = I2S_DMA_DISABLE;
    I2S_Init(I2S_USR, &I2S_InitStruct);
    I2S_Cmd(I2S_USR, I2S_MODE_TX | I2S_MODE_RX, ENABLE);
}

/**
  * @brief  demo code of sending data continuously.
  * @param   No parameter.
  * @return  void
  */
void I2S_SendDemoCode(void)
{
    uint32_t i = 0x12345678;

    while (1)
    {
        if (I2S_GetTxFIFOFreeLen(I2S_USR))
        {
            /* 16bit format, lower half word send first! */
            I2S_SendData(I2S_USR, i);
        }
        while (I2S_GetINTStatus(I2S_USR, I2S_INT_TF_FULL));

        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
    }
}

/**
  * @brief  demo code of I2S communication.
  * @param   No parameter.
  * @return  void
  */
void I2S_DemoCode(void)
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
int main()
{
    I2S_DemoCode();
}
/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

