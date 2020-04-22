/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
    * @file     i2s_interrupt_demo.c
    * @brief    This file provides demo code of I2S comunication.
    * @details
    * @author   yuan
    * @date     2018-05-29
    * @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_i2s.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "trace.h"

/* Defines -------------------------------------------------------------------*/
#define I2S_LRCK_PIN                P3_2
#define I2S_BCLK_PIN                P3_3
#define I2S_DATA_TX_PIN             P3_4

#define I2S_NUM                     I2S1
#define I2S_LRCK_PINMUX             LRC_SPORT1
#define I2S_BCLK_PINMUX             BCLK_SPORT1
#define I2S_DATA_TX_PINMUX          DACDAT_SPORT1

#define APB_I2S                     APBPeriph_I2S1
#define APB_I2S_CLOCK               APBPeriph_I2S1_CLOCK
#define I2S_TX_IRQn                 SPORT1_TX_IRQn

/* Globals --------------------------------------------------------------------*/
uint32_t I2S_Send_Data = 0x12345678;
uint32_t I2S_Receive_Data[10];

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
*/
void board_i2s_init(void)
{
    Pad_Config(I2S_BCLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(I2S_LRCK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(I2S_DATA_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);

    Pinmux_Config(I2S_BCLK_PIN, I2S_BCLK_PINMUX);
    Pinmux_Config(I2S_LRCK_PIN, I2S_LRCK_PINMUX);
    Pinmux_Config(I2S_DATA_TX_PIN, I2S_DATA_TX_PINMUX);
}

/**
  * @brief  Initialize I2S peripheral.
  * @param  No parameter.
  * @return Void
*/
void driver_i2s_init(void)
{
    RCC_PeriphClockCmd(APB_I2S, APB_I2S_CLOCK, ENABLE);

    I2S_InitTypeDef I2S_InitStruct;
    I2S_StructInit(&I2S_InitStruct);
    I2S_InitStruct.I2S_ClockSource      = I2S_CLK_40M;
    I2S_InitStruct.I2S_BClockMi         = 0x271;/* <!BCLK = 16K */
    I2S_InitStruct.I2S_BClockNi         = 0x08;
    I2S_InitStruct.I2S_DeviceMode       = I2S_DeviceMode_Master;
    I2S_InitStruct.I2S_ChannelType      = I2S_Channel_stereo;
    I2S_InitStruct.I2S_DataFormat       = I2S_Mode;
    I2S_Init(I2S_NUM, &I2S_InitStruct);
    I2S_Cmd(I2S_NUM, I2S_MODE_TX | I2S_MODE_RX, ENABLE);

    /* Enable I2S0 Tx interrupt */
    I2S_INTConfig(I2S_NUM, I2S_INT_TF_EMPTY, ENABLE);
    /* Config NVIC */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = I2S_TX_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Demo code of I2S communication.
  * @param  No parameter.
  * @return Void
*/
void i2s_demo(void)
{
    board_i2s_init();
    driver_i2s_init();
}

/**
  * @brief  I2S interrupt handler function.
  * @param  No parameter.
  * @return Void
*/
void I2S1_TX_Handler(void)
{
    if (I2S_GetINTStatus(I2S_NUM, I2S_INT_TF_EMPTY))
    {
        I2S_SendData(I2S_NUM, I2S_Send_Data++);
        DBG_DIRECT("I2S_INT_TF_EMPTY");
    }
}

/**
  * @brief  I2S interrupt handler function.
  * @param  No parameter.
  * @return Void
*/
void I2S1_RX_Handler(void)
{
    if (I2S_GetINTStatus(I2S1, I2S_INT_RF_FULL))
    {
        uint8_t i2s_rx_datalenth = I2S_GetRxFIFOLen(I2S1);
        for (uint8_t i = 0; i < i2s_rx_datalenth; i++)
        {
            I2S_Receive_Data[i] = I2S_ReceiveData(I2S1);
        }
    }
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

