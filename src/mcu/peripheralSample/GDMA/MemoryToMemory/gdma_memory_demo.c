/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     gdma_i2c_demo.c
* @brief    This file provides demo code of meomory to memory transfer by gdma.
* @details
* @author   yuan
* @date     2018-06-28
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "trace.h"


/* Defines ------------------------------------------------------------------*/
#define GDMA_CHANNEL_NUM                5
#define GDMA_Channel                    GDMA_Channel5
#define GDMA_Channel_IRQn               GDMA0_Channel5_IRQn
#define GDMA_Channel_Handler            GDMA0_Channel5_Handler

#define GDMA_TRANSFER_SIZE              200

/* Globals ------------------------------------------------------------------*/
uint32_t GDMA_Send_Buf[GDMA_TRANSFER_SIZE];
uint32_t GDMA_Recv_Buf[GDMA_TRANSFER_SIZE];

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_gdma_memory_init(void)
{
    uint32_t i = 0;

    /* Test data buffer */
    for (i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        GDMA_Send_Buf[i] = i + 1;
        GDMA_Recv_Buf[i] = 0;
    }

    /* Turn on gdma clock */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    /* GDMA initial*/
    GDMA_InitStruct.GDMA_ChannelNum          = GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToMemory;
    GDMA_InitStruct.GDMA_BufferSize          = GDMA_TRANSFER_SIZE;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_8;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_8;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)GDMA_Send_Buf;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)GDMA_Recv_Buf;

    GDMA_Init(GDMA_Channel, &GDMA_InitStruct);

    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    /* GDMA irq init */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);

    GDMA_Cmd(GDMA_CHANNEL_NUM, ENABLE);
}

/**
  * @brief  Demo code of operation about i2c + gdma.
  * @param  No parameter.
  * @return void
*/
void gdma_memory_demo(void)
{
    driver_gdma_memory_init();
}

/**
  * @brief  Demo code of operation about i2c + gdma.
  * @param  No parameter.
  * @return void
*/
void gdma_demo(void)
{
    gdma_memory_demo();
}

/**
  * @brief  GDMA channel interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void GDMA_Channel_Handler(void)
{
    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Transfer, DISABLE);
    //Add user code here
    GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
}


