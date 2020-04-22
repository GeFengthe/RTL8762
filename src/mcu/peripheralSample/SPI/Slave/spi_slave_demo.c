/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of SPI comunication in interrupt mode as salve.
* @details
* @author   elliot chen
* @date     2015-07-03
* @version  v0.1
*********************************************************************************************************
*/
#define SPI0_SCK                P1_0
#define SPI0_MOSI               P1_1
#define SPI0_MISO               P1_2
#define SPI0_CS                 P1_3

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_rcc.h"
#include "rtl876x_spi.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"

void RCC_Configuration(void)
{
    /* turn on SPI clock */
    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);
}

void SPI_PINMUXConfiguration(void)
{
    Pinmux_Config(SPI0_SCK, SPI0_CLK_SLAVE);
    Pinmux_Config(SPI0_MOSI, SPI0_SI_SLAVE);//SPI0_MO_Master <--> SPI0_SI_0_Slave
    Pinmux_Config(SPI0_MISO, SPI0_SO_SLAVE);//SPI0_MI_Master <--> SPI0_SO_0_Slave
    Pinmux_Config(SPI0_CS, SPI0_SS_N_0_SLAVE);
}

void SPI_PADConfiguration(void)
{
    Pad_Config(SPI0_SCK, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MOSI, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MISO, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_CS, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
}

void SPI_Configuration(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    SPI_StructInit(&SPI_InitStructure);

    SPI_InitStructure.SPI_Direction   = SPI_Direction_FullDuplex;
    SPI_InitStructure.SPI_Mode        = SPI_Mode_Slave;
    SPI_InitStructure.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA        = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_BaudRatePrescaler  = 100;

    SPI_InitStructure.SPI_RxThresholdLevel  =
        0;/* cause SPI_INT_RXF interrupt if data length in receive FIFO  >= SPI_RxThresholdLevel + 1*/
    SPI_InitStructure.SPI_FrameFormat = SPI_Frame_Motorola;

    SPI_Init(SPI0, &SPI_InitStructure);
    SPI_Cmd(SPI0, ENABLE);

    /* detect receive data */
    SPI_INTConfig(SPI0, SPI_INT_RXF, ENABLE);
    /* Config SPI interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = SPI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/*--------------------------SPI demo code----------------------------------*/
void SPI_DemoCode(void)
{
    //add user code here
}

int main(void)
{
    RCC_Configuration();
    SPI_PINMUXConfiguration();
    SPI_PADConfiguration();
    SPI_Configuration();
    SPI_DemoCode();
    while (1);
}

void Spi0IntrHandler(void)
{
    uint8_t len = 0;
    uint8_t idx = 0;
    uint8_t SPI_ReadINTBuf[16] = {0, 0, 0, 0};

    if (SPI_GetINTStatus(SPI0, SPI_INT_RXF) == SET)
    {
        len = SPI_GetRxFIFOLen(SPI0);
        for (idx = 0; idx < len; idx++)
        {
            /* must read all data in receive FIFO , otherwise cause SPI_INT_RXF interrupt again */
            SPI_ReadINTBuf[idx] = SPI_ReceiveData(SPI0);
        }
    }
}
