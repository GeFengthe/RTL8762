/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     gdma_i2c_demo.c
* @brief    gdma i2c master rx + slave tx demo
* @details
* @author   yuan
* @date     2018-06-28
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_gdma.h"
#include "rtl876x_i2c.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "trace.h"


/* Defines ------------------------------------------------------------------*/
#define I2C0_SCL_PIN                        P3_2
#define I2C0_SDA_PIN                        P1_2

#define I2C1_SCL_PIN                        P2_2
#define I2C1_SDA_PIN                        P2_5

#define I2C0_GDMA_CHANNEL_NUM               0
#define I2C0_GDMA_Channel                   GDMA_Channel0
#define I2C0_GDMA_Channel_IRQn              GDMA0_Channel0_IRQn
#define I2C0_GDMA_Channel_Handler           GDMA0_Channel0_Handler
#define I2C1_GDMA_CHANNEL_NUM               1
#define I2C1_GDMA_Channel                   GDMA_Channel1
#define I2C1_GDMA_Channel_IRQn              GDMA0_Channel1_IRQn
#define I2C1_GDMA_Channel_Handler           GDMA0_Channel1_Handler
#define GDMA_TRANSFER_SIZE                  200

/* Globals ------------------------------------------------------------------*/
uint16_t GDMA_I2C_Send_Buf[GDMA_TRANSFER_SIZE];
uint8_t GDMA_I2C_Recv_Buf[GDMA_TRANSFER_SIZE];


/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_i2c_init(void)
{
    Pad_Config(I2C0_SCL_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(I2C0_SDA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(I2C1_SCL_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(I2C1_SDA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Config(I2C0_SCL_PIN, I2C0_CLK);
    Pinmux_Config(I2C0_SDA_PIN, I2C0_DAT);
    Pinmux_Config(I2C1_SCL_PIN, I2C1_CLK);
    Pinmux_Config(I2C1_SDA_PIN, I2C1_DAT);
}

/**
  * @brief  Initialize i2c0 peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_i2c0_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_I2C0, APBPeriph_I2C0_CLOCK, ENABLE);

    I2C_InitTypeDef  I2C_InitStruct;
    I2C_StructInit(&I2C_InitStruct);

    I2C_InitStruct.I2C_ClockSpeed   = 100000;
    I2C_InitStruct.I2C_DeviveMode   = I2C_DeviveMode_Slave;
    I2C_InitStruct.I2C_AddressMode  = I2C_AddressMode_7BIT;
    I2C_InitStruct.I2C_SlaveAddress = 0x50;
    I2C_InitStruct.I2C_Ack          = I2C_Ack_Enable;
    I2C_InitStruct.I2C_TxDmaEn      = ENABLE;
    I2C_InitStruct.I2C_TxWaterlevel = 20;
    I2C_Init(I2C0, &I2C_InitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = I2C0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);

    /* Configure interrupt type, please reference to i2c document about all interrupt type description */
    I2C_INTConfig(I2C0, I2C_INT_RD_REQ | I2C_INT_RX_FULL, ENABLE);
    I2C_Cmd(I2C0, ENABLE);
}

/**
  * @brief  Initialize i2c1 peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_i2c1_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_I2C1, APBPeriph_I2C1_CLOCK, ENABLE);

    I2C_InitTypeDef  I2C_InitStruct;
    I2C_StructInit(&I2C_InitStruct);

    I2C_InitStruct.I2C_ClockSpeed   = 100000;
    I2C_InitStruct.I2C_DeviveMode   = I2C_DeviveMode_Master;
    I2C_InitStruct.I2C_AddressMode  = I2C_AddressMode_7BIT;
    I2C_InitStruct.I2C_SlaveAddress = 0x50;
    I2C_InitStruct.I2C_Ack          = I2C_Ack_Enable;
    I2C_InitStruct.I2C_RxDmaEn      = ENABLE;
    I2C_InitStruct.I2C_RxWaterlevel = 3;
    I2C_Init(I2C1, &I2C_InitStruct);

//    I2C_INTConfig(I2C1, I2C_INT_RX_UNDER, ENABLE);
    I2C_Cmd(I2C1, ENABLE);
}

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
*/
void gdma_i2c0_tx_init(void)
{
    /* Initialize data buffer */
    for (uint32_t i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        GDMA_I2C_Send_Buf[i] = i;
    }

    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    /* GDMA init */
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);

    GDMA_InitStruct.GDMA_ChannelNum      = I2C0_GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR             = GDMA_DIR_MemoryToPeripheral;//GDMA_DIR_PeripheralToMemory
    GDMA_InitStruct.GDMA_BufferSize      = GDMA_TRANSFER_SIZE;
    GDMA_InitStruct.GDMA_SourceInc       = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc  = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize  = GDMA_DataSize_HalfWord;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_HalfWord;
    GDMA_InitStruct.GDMA_SourceMsize        = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_DestinationMsize   = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_SourceAddr         = (uint32_t)GDMA_I2C_Send_Buf;
    GDMA_InitStruct.GDMA_DestinationAddr    = (uint32_t)(&(I2C0->IC_DATA_CMD));
    GDMA_InitStruct.GDMA_DestHandshake      = GDMA_Handshake_I2C0_TX;

    GDMA_Init(I2C0_GDMA_Channel, &GDMA_InitStruct);

    /* GDMA irq config */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = I2C0_GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);

    GDMA_INTConfig(I2C0_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

}

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
*/
void gdma_i2c1_rx_init(void)
{
    /* Initialize data buffer */
    for (uint32_t i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        GDMA_I2C_Recv_Buf[i] = 0;
    }

    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    /* GDMA init */
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);

    GDMA_InitStruct.GDMA_ChannelNum      = I2C1_GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR             = GDMA_DIR_PeripheralToMemory;//GDMA_DIR_PeripheralToMemory
    GDMA_InitStruct.GDMA_BufferSize      = GDMA_TRANSFER_SIZE;
    GDMA_InitStruct.GDMA_SourceInc       = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc  = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize  = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize        = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_DestinationMsize   = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_SourceAddr         = (uint32_t)(&(I2C1->IC_DATA_CMD));
    GDMA_InitStruct.GDMA_DestinationAddr    = (uint32_t)(GDMA_I2C_Recv_Buf);
    GDMA_InitStruct.GDMA_SourceHandshake    = GDMA_Handshake_I2C1_RX;

    GDMA_Init(I2C1_GDMA_Channel, &GDMA_InitStruct);

    /* GDMA irq config */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = I2C1_GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);

    GDMA_INTConfig(I2C1_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    /* Start to receive data */
    GDMA_Cmd(I2C1_GDMA_CHANNEL_NUM, ENABLE);
}

void i2c1_send_read_cmd(void)
{
    /* Send read command */
    uint32_t cnt = 0;
    for (cnt = 0; cnt < GDMA_TRANSFER_SIZE; cnt++)
    {
        if (cnt >= GDMA_TRANSFER_SIZE - 1)
        {
            /* Generate stop singal */
            I2C1->IC_DATA_CMD = (0x00000003 << 8);
        }
        else
        {
            I2C1->IC_DATA_CMD = (0x00000001 << 8);
        }

        for (uint32_t i = 0; i < 1000; i++);
    }
}

/**
  * @brief  Demo code of operation about i2c + gdma.
  * @param  No parameter.
  * @return void
*/
void gdma_i2c_demo(void)
{
    board_i2c_init();
    driver_i2c0_init();
    driver_i2c1_init();
    gdma_i2c0_tx_init();
    gdma_i2c1_rx_init();
    i2c1_send_read_cmd();
}

/**
  * @brief  Demo code of operation about i2c + gdma.
  * @param  No parameter.
  * @return void
*/
void gdma_demo(void)
{
    gdma_i2c_demo();

}

/**
  * @brief  I2C0 interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void I2C0_Handler(void)
{
//    DBG_DIRECT("I2C0_Handler!");
    if (I2C_GetINTStatus(I2C0, I2C_INT_RD_REQ) == SET)
    {
//        DBG_DIRECT("I2C0_Handler I2C_INT_RD_REQ!");
        GDMA_Cmd(I2C0_GDMA_CHANNEL_NUM, ENABLE);
        //Add user code here
        /* Clear interrupt */
        I2C_ClearINTPendingBit(I2C0, I2C_INT_RD_REQ);
        I2C_INTConfig(I2C0, I2C_INT_RD_REQ, DISABLE);
    }

    if (I2C_GetINTStatus(I2C0, I2C_INT_RX_FULL) == SET)
    {
//        DBG_DIRECT("I2C0_Handler I2C_INT_RX_FULL!");
        //Add user code here
        /* Clear interrupt,read for clear */
        I2C_INTConfig(I2C0, I2C_INT_RX_FULL, DISABLE);
    }
}

/**
  * @brief  GDMA channel interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void I2C0_GDMA_Channel_Handler(void)
{
    GDMA_INTConfig(I2C0_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, DISABLE);
//    DBG_DIRECT("I2C0_GDMA_Channel_Handler!");
    //Add user code here
    GDMA_ClearINTPendingBit(I2C0_GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
}

/**
  * @brief  GDMA channel interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void I2C1_GDMA_Channel_Handler(void)
{
    GDMA_INTConfig(I2C1_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, DISABLE);
//    DBG_DIRECT("I2C1_GDMA_Channel_Handler!");
    //Add user code here
    GDMA_ClearINTPendingBit(I2C1_GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
}


