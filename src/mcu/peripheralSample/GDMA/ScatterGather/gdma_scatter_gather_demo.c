/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     gdma_scatter_gather_demo.c
* @brief    This file provides demo code of meomory to memory transfer by scatter/gather gdma.
* @details
* @author   yuan
* @date     2018-06-28
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"
#include "rtl876x_rcc.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#define GDMA_CHANNEL_NUM                3
#define GDMA_Channel                    GDMA_Channel3
#define GDMA_Channel_IRQn               GDMA0_Channel3_IRQn
#define GDMA_Channel_Handler            GDMA0_Channel3_Handler

#define GDMA_TRANSFER_SIZE              (255)
#define GDMA_GATHER_EN                  ENABLE
#define GDMA_SCATTER_EN                 DISABLE
#define GDMA_GATHER_COUNT               (4)
#define GDMA_GATHER_INTERVAL            (4)
#define GDMA_SCATTER_COUNT              (4)
#define GDMA_SCATTER_INTERVAL           (4)

#define SOURCE_DATA_SIZE                (((GDMA_TRANSFER_SIZE/GDMA_GATHER_COUNT)+1)*(GDMA_GATHER_COUNT+GDMA_GATHER_INTERVAL)-GDMA_GATHER_INTERVAL)
#define DESTINATION_DATA_SIZE           (((GDMA_TRANSFER_SIZE/GDMA_SCATTER_COUNT)+1)*(GDMA_SCATTER_COUNT+GDMA_SCATTER_INTERVAL)-GDMA_SCATTER_INTERVAL)

/* Globales ------------------------------------------------------------------*/
uint8_t GDMA_Send_Buffer[SOURCE_DATA_SIZE];
uint8_t GDMA_Recv_Buffer[DESTINATION_DATA_SIZE];

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_gdma_scatter_gather_init(void)
{
    uint32_t i = 0;

    for (i = 0; i < SOURCE_DATA_SIZE; i++)
    {
        GDMA_Send_Buffer[i] = i;
    }
    for (i = 0; i < DESTINATION_DATA_SIZE; i++)
    {
        GDMA_Recv_Buffer[i] = 0;
    }

    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);

    GDMA_InitStruct.GDMA_ChannelNum      = GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR             = GDMA_DIR_MemoryToMemory;
    GDMA_InitStruct.GDMA_BufferSize      = GDMA_TRANSFER_SIZE;//determine total transfer size
    GDMA_InitStruct.GDMA_SourceInc       = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc  = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize  = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize        = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize   = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr         = (uint32_t)GDMA_Send_Buffer;
    GDMA_InitStruct.GDMA_DestinationAddr    = (uint32_t)GDMA_Recv_Buffer;

    GDMA_InitStruct.GDMA_Scatter_En         = GDMA_SCATTER_EN;
    GDMA_InitStruct.GDMA_ScatterCount       = GDMA_SCATTER_COUNT;
    GDMA_InitStruct.GDMA_ScatterInterval    = GDMA_SCATTER_INTERVAL;
    GDMA_InitStruct.GDMA_Gather_En          = GDMA_GATHER_EN;
    GDMA_InitStruct.GDMA_GatherCount        = GDMA_GATHER_COUNT;
    GDMA_InitStruct.GDMA_GatherInterval     = GDMA_GATHER_INTERVAL;

    GDMA_Init(GDMA_Channel, &GDMA_InitStruct);

    /* GDMA irq config */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);

    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    GDMA_Cmd(GDMA_CHANNEL_NUM, ENABLE);
}

/**
  * @brief  Demo code of operation about i2c + gdma.
  * @param  No parameter.
  * @return void
*/
void gdma_scatter_gather_demo(void)
{
    driver_gdma_scatter_gather_init();
}

/**
  * @brief  Demo code of operation about i2c + gdma.
  * @param  No parameter.
  * @return void
*/
void gdma_demo(void)
{
    gdma_scatter_gather_demo();
}

/**
  * @brief  GDMA0 channel interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void GDMA_Channel_Handler(void)
{
    GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
    GDMA_Cmd(GDMA_CHANNEL_NUM, DISABLE);
    for (uint32_t i = 0; i < SOURCE_DATA_SIZE; i++)
    {
        DBG_DIRECT("GDMA send data[%d] = %d", i, GDMA_Send_Buffer[i]);
    }
    for (uint32_t i = 0; i < DESTINATION_DATA_SIZE; i++)
    {
        DBG_DIRECT("GDMA receive data[%d] = %d", i, GDMA_Recv_Buffer[i]);
        GDMA_Recv_Buffer[i] = 0;
    }
    GDMA_SetSourceAddress(GDMA_Channel, (uint32_t)GDMA_Send_Buffer);
    GDMA_SetDestinationAddress(GDMA_Channel, (uint32_t)GDMA_Recv_Buffer);
    GDMA_Cmd(GDMA_CHANNEL_NUM, ENABLE);
}
