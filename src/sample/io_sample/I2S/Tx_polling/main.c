/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of i2s send data.
* @details
* @author   yuan
* @date     2018-05-29
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_i2s.h"
//#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"

/* Defines -------------------------------------------------------------------*/
#define I2S_LRCK_PIN                P3_2
#define I2S_BCLK_PIN                P3_3
#define I2S_DATA_TX_PIN             P4_0

#define I2S_NUM                     I2S0
#define I2S_LRCK_PINMUX             LRC_SPORT0
#define I2S_BCLK_PINMUX             BCLK_SPORT0
#define I2S_DATA_TX_PINMUX          DACDAT_SPORT0

#define APB_I2S                     APBPeriph_I2S0
#define APB_I2S_CLOCK               APBPeriph_I2S0_CLOCK

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
    /* BCLK = 40MHz*(ni/mi),  LRCK = BCLK/64 */
    I2S_InitStruct.I2S_BClockMi         = 0x271;    /* <!LRCK = 16K */
    I2S_InitStruct.I2S_BClockNi         = 0x10;     /* <!BCLK = 1024K */
    I2S_InitStruct.I2S_DeviceMode       = I2S_DeviceMode_Master;
    I2S_InitStruct.I2S_ChannelType      = I2S_Channel_stereo;
    I2S_InitStruct.I2S_DataWidth        = I2S_Width_16Bits;
    I2S_InitStruct.I2S_DataFormat       = I2S_Mode;
    I2S_InitStruct.I2S_DMACmd           = I2S_DMA_DISABLE;
    I2S_Init(I2S_NUM, &I2S_InitStruct);
    I2S_Cmd(I2S_NUM, I2S_MODE_TX, ENABLE);
}

/**
  * @brief  Demo code of sending data continuously.
  * @param  No parameter.
  * @return Void
  */
void i2s_senddata(void)
{
    uint32_t i = 0x12348800;

    while (1)
    {
        if (I2S_GetTxFIFOFreeLen(I2S_NUM))
        {
            /* 16bit format, lower half word send first! */
            I2S_SendData(I2S_NUM, i++);
        }
    }
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
    i2s_senddata();
}

/**
  * @brief    Entry of app code
  * @return   int (To avoid compile warning)
  */
int main(void)
{
    i2s_demo();

    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();

    }
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

