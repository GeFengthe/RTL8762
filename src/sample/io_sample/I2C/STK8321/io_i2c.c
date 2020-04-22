/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_i2c.c
* @brief    This file provides demo code of i2c master.
            STK8321 is slave.
            Read the chip id of STK8321 and Gsensor outdata.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "io_i2c.h"

#include "stk8321.h"

#include "app_task.h"

/* Globals ------------------------------------------------------------------*/


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
    I2C_InitStruct.I2C_SlaveAddress  = STK8321_ADDRESS;
    I2C_InitStruct.I2C_Ack           = I2C_Ack_Enable;

    I2C_Init(I2C0, &I2C_InitStruct);
    I2C_Cmd(I2C0, ENABLE);

}

/**
  * @brief  Config i2c nvic.
  * @param  No parameter.
  * @return void
  */
void nvic_i2c_config(void)
{
    /* Detect stop signal */
    I2C_ClearINTPendingBit(I2C0, I2C_INT_STOP_DET);
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
    if (0x23 == stk8321_id_get())
    {
        APP_PRINT_INFO0("[io_i2c] i2c_demo: Get stk8321 chip id OK. ");
        nvic_i2c_config();
        stk8321_outdata_get();
    }
}

/**
  * @brief  Handle i2c data function.
  * @param  No parameter.
  * @return void
  */
void io_i2c_handle_msg(T_IO_MSG *io_i2c_msg)
{
    uint8_t *p_buf = io_i2c_msg->u.buf;

    GSensor_Data.XData = p_buf[0] & 0xFF;
    GSensor_Data.XData = (GSensor_Data.XData + ((uint16_t)p_buf[1] << 8)) >> 4;
    GSensor_Data.YData = p_buf[2] & 0xFF;
    GSensor_Data.YData = (GSensor_Data.YData + ((uint16_t)p_buf[3] << 8)) >> 4;
    GSensor_Data.ZData = p_buf[4] & 0xFF;
    GSensor_Data.ZData = (GSensor_Data.ZData + ((uint16_t)p_buf[5] << 8)) >> 4;
    APP_PRINT_INFO3("[io_i2c]io_i2c_handle_msg: GSensor x_axic_data = %d, y_axic_data = %d, z_axic_data = %d,",
                    GSensor_Data.XData, GSensor_Data.YData, GSensor_Data.ZData);
}

/**
  * @brief  Handle i2c data function.
  * @param  No parameter.
  * @return void
  */
void io_handle_i2c_msg(T_IO_MSG *io_i2c_msg)
{
    io_i2c_handle_msg(io_i2c_msg);
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
        T_IO_MSG int_i2c_msg;

        int_i2c_msg.type = IO_MSG_TYPE_I2C;
        int_i2c_msg.subtype = 0;
        int_i2c_msg.u.buf = (void *)(GSensor_Data.OutData);
        if (false == app_send_msg_to_apptask(&int_i2c_msg))
        {
            APP_PRINT_ERROR0("[io_i2c] I2C0_Handler: Send int_i2c_msg failed!");
            I2C_ClearINTPendingBit(I2C0, I2C_INT_STOP_DET);
            //Add user code here!
            return;
        }
        I2C_ClearINTPendingBit(I2C0, I2C_INT_STOP_DET);
//        stk8321_outdata_get();
    }
}

