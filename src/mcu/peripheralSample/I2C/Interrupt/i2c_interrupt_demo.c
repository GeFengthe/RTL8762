/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
    * @file     i2c_interrupt_demo.c
    * @brief    This file provides demo code of i2c in interrupt mode which as a master.
    * @details
    * @author   yuan
    * @date     2018-06-26
    * @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_i2c.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#define I2C0_SCL_PIN                P3_2        //C9: 0
#define I2C0_SDA_PIN                P1_2        //C9: 1

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_i2c_init(void)
{
    Pad_Config(I2C0_SCL_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(I2C0_SDA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Config(I2C0_SCL_PIN, I2C0_CLK);
    Pinmux_Config(I2C0_SDA_PIN, I2C0_DAT);
}

/**
  * @brief  Initialize i2c peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_i2c_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_I2C0, APBPeriph_I2C0_CLOCK, ENABLE);

    I2C_InitTypeDef  I2C_InitStruct;
    I2C_StructInit(&I2C_InitStruct);

    I2C_InitStruct.I2C_ClockSpeed    = 100000;
    I2C_InitStruct.I2C_DeviveMode    = I2C_DeviveMode_Master;
    I2C_InitStruct.I2C_AddressMode   = I2C_AddressMode_7BIT;
    I2C_InitStruct.I2C_SlaveAddress  = 0x50;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;

    I2C_Init(I2C0, &I2C_InitStruct);
    I2C_Cmd(I2C0, ENABLE);

    /* Detect stop signal */
    I2C_INTConfig(I2C0, I2C_INT_STOP_DET, ENABLE);

    /* Config I2C interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = I2C0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Demo code of operation about i2c.
  * @param  No parameter.
  * @return void
*/
void i2c_demo(void)
{
    uint8_t i2c_writeBuf[16] = {0x00, 0x00, 0x66, 0x68, 0x77, 0x88};

    board_i2c_init();
    driver_i2c_init();

    I2C_MasterWrite(I2C0, i2c_writeBuf, 4);

}

/**
  * @brief  I2C0 interrupt handle function.
  * @param  None.
  * @return None.
*/
void I2C0_Handler(void)
{
    if (I2C_GetINTStatus(I2C0, I2C_INT_STOP_DET) == SET)
    {
        DBG_DIRECT("Enter I2C0 interrupt!");
        I2C_ClearINTPendingBit(I2C0, I2C_INT_STOP_DET);
    }
}
