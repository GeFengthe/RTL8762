/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     dma_multiblock_demo.c
* @brief    This file provides demo code of meomory to memory transfer by gdma multiblock.
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
#define GDMA_CHANNEL_NUM                2
#define GDMA_Channel                    GDMA_Channel2
#define GDMA_Channel_IRQn               GDMA0_Channel2_IRQn
#define GDMA_Channel_Handler            GDMA0_Channel2_Handler

#define GDMA_TRANSFER_SIZE              10
#define GDMA_MULTIBLOCK_SIZE            12

#define GDMA_MULTIBLOCK_MODE            LLI_TRANSFER
//#define GDMA_MULTIBLOCK_MODE            LLI_WITH_CONTIGUOUS_SAR
//#define GDMA_MULTIBLOCK_MODE            LLI_WITH_CONTIGUOUS_DAR

/* Globals ------------------------------------------------------------------*/
uint8_t GDMA_Send_Buffer[12][GDMA_TRANSFER_SIZE];
uint8_t GDMA_Recv_Buffer[12][GDMA_TRANSFER_SIZE];
GDMA_LLIDef GDMA_LLIStruct[GDMA_MULTIBLOCK_SIZE];

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_gdma_multiblock_init(void)
{
    uint32_t i, j = 0;

    /*--------------Initialize test buffer---------------------*/
    for (i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        for (j = 0; j < GDMA_MULTIBLOCK_SIZE; j++)
        {
            GDMA_Send_Buffer[j][i] = (i + j) & 0xff;
        }
    }

    for (i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        for (j = 0; j < GDMA_MULTIBLOCK_SIZE; j++)
        {
            GDMA_Recv_Buffer[j][i] = 0;
        }
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
    GDMA_InitStruct.GDMA_Multi_Block_Mode   = GDMA_MULTIBLOCK_MODE;//LLI_TRANSFER;
    GDMA_InitStruct.GDMA_Multi_Block_En     = 1;
    GDMA_InitStruct.GDMA_Multi_Block_Struct = (uint32_t)GDMA_LLIStruct;

    for (uint32_t i = 0; i < GDMA_MULTIBLOCK_SIZE; i++)
    {
        if (i == GDMA_MULTIBLOCK_SIZE - 1)
        {
            //GDMA_LLIStruct[i].LLP=0;
            GDMA_LLIStruct[i].SAR = (uint32_t)GDMA_Send_Buffer[i];
            GDMA_LLIStruct[i].DAR = (uint32_t)GDMA_Recv_Buffer[i];
            GDMA_LLIStruct[i].LLP = 0;
            /* Configure low 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct.GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct.GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct.GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct.GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct.GDMA_DIR << 20);
            /* Configure high 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_HIGH = GDMA_InitStruct.GDMA_BufferSize;
        }
        else
        {
            GDMA_LLIStruct[i].SAR = (uint32_t)GDMA_Send_Buffer[i];
            GDMA_LLIStruct[i].DAR = (uint32_t)GDMA_Recv_Buffer[i];
            GDMA_LLIStruct[i].LLP = (uint32_t)&GDMA_LLIStruct[i + 1];
            /* Configure low 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct.GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct.GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct.GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct.GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct.GDMA_DIR << 20)
                                        | (GDMA_InitStruct.GDMA_Multi_Block_Mode & LLP_SELECTED_BIT);
            /* Configure high 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_HIGH = GDMA_InitStruct.GDMA_BufferSize;
        }
    }
    GDMA_Init(GDMA_Channel, &GDMA_InitStruct);

    /* GDMA irq config */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);

//    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Block, ENABLE);
    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    GDMA_Cmd(GDMA_CHANNEL_NUM, ENABLE);
}

/**
  * @brief  Demo code of operation about i2c + gdma.
  * @param  No parameter.
  * @return void
*/
void gdma_multiblock_demo(void)
{
    driver_gdma_multiblock_init();
}

/**
  * @brief  Demo code of operation about i2c + gdma.
  * @param  No parameter.
  * @return void
*/
void gdma_demo(void)
{
    gdma_multiblock_demo();
}

/**
  * @brief  GDMA0 channel interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void GDMA_Channel_Handler(void)
{
    DBG_DIRECT("GDMA_Channel_Handler!");
    GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
    for (uint32_t i = 0; i < GDMA_MULTIBLOCK_SIZE; i++)
    {
        for (uint32_t j = 0; j < GDMA_TRANSFER_SIZE; j++)
        {
            DBG_DIRECT("GDMA receive data[%d][%d] = %d", i, j, GDMA_Recv_Buffer[i][j]);
            GDMA_Recv_Buffer[i][j] = 0;
        }
    }
}
