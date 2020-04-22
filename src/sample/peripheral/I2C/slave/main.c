/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of I2C slave mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
#include <trace.h>
#include <os_sched.h>
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_nvic.h"
#include "rtl876x_I2C.h"
#include "rtl876x_wdg.h"
#include "board.h"

/**
 * @brief    pinmux configuration
 * @return   void
 */
void PINMUX_Configuration(void)
{
    Pinmux_Config(I2C0_SCL, I2C0_CLK);
    Pinmux_Config(I2C0_SDA, I2C0_DAT);
    return;
}
/**
 * @brief    pad configuration
 * @return   void
 */
void PAD_Configuration(void)
{

    Pad_Config(I2C0_SCL, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(I2C0_SDA, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    return;
}

/**
 * @brief    rcc configuration
 * @return   void
 */
void RCC_Configuration(void)
{
    /* Enable I2C clock */
    RCC_PeriphClockCmd(APBPeriph_I2C0, APBPeriph_I2C0_CLOCK, ENABLE);
    return;
}

/**
 * @brief    Contains the initialization of pinmux settings and pad settings
 * @note     All the pinmux settings and pad settings shall be initiated in this function,
 *           but if legacy driver is used, the initialization of pinmux setting and pad setting
 *           should be peformed with the IO initializing.
 * @return   void
 */
void board_init(void)
{
    RCC_Configuration();
    PINMUX_Configuration();
    PAD_Configuration();
}
/**
  * @brief  Initialize IO peripheral.
  * @param   No parameter.
  * @return  void
  */
void I2C_Configuration(void)
{
    /* Initialize I2C */
    I2C_InitTypeDef  I2C_InitStructure;
    I2C_StructInit(&I2C_InitStructure);
    I2C_InitStructure.I2C_ClockSpeed = 100000;
    I2C_InitStructure.I2C_DeviveMode = I2C_DeviveMode_Slave;
    I2C_InitStructure.I2C_AddressMode = I2C_AddressMode_7BIT;
    I2C_InitStructure.I2C_SlaveAddress = 0x50;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_Init(I2C0, &I2C_InitStructure);
    I2C_Cmd(I2C0, ENABLE);
    /* configure interrupt type, please reference to I2C document about All interrupt type description */
    I2C_INTConfig(I2C0, I2C_INT_RD_REQ | I2C_INT_STOP_DET, ENABLE);
    /* Config I2C interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = I2C0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}
int main(void)
{
    WDG_Disable();
    __enable_irq();
    board_init();
    I2C_Configuration();
    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
    }
}

void I2C0_Handler(void)
{
    // any master want to read data will result this type interrupt.
    if (I2C_GetINTStatus(I2C0, I2C_INT_RD_REQ) == SET)
    {
        // add user code here
        uint8_t data = I2C_ReceiveData(I2C0);
//              DBG_DIRECT("%d",data);
        I2C_SendCmd(I2C0, I2C_WRITE_CMD, 0x66, DISABLE);
        // clear interrupt
        I2C_ClearINTPendingBit(I2C0, I2C_INT_RD_REQ);
    }

    if (I2C_GetINTStatus(I2C0, I2C_INT_STOP_DET) == SET)
    {
        // add user code here

        // clear interrupt
        I2C_ClearINTPendingBit(I2C0, I2C_INT_STOP_DET);
    }
}
