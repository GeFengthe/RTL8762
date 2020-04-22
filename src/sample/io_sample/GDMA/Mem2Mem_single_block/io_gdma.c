/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     io_gdma.c
* @brief    This file provides demo code of meomory to memory transfer by gdma.
* @details
* @author   yuan
* @date     2019-01-11
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "io_gdma.h"

#include "app_task.h"

/* Globals ------------------------------------------------------------------*/
uint32_t GDMA_Send_Buf[GDMA_TRANSFER_SIZE];
uint32_t GDMA_Recv_Buf[GDMA_TRANSFER_SIZE];

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_gdma_init(void)
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

    /** To debug, enable GDMA transmission when the GAP stack is ready.
      * In app.c->app_handle_dev_state_evt()
      */
//    GDMA_Cmd(GDMA_CHANNEL_NUM, ENABLE);
}

/**
  * @brief  Handle gdma data function.
  * @param  No parameter.
  * @return void
  */
void io_handle_gdma_msg(T_IO_MSG *io_gdma_msg)
{
    APP_PRINT_INFO0("[io_gdma] io_handle_gdma_msg: GDMA transfer data completion!");
    for (uint32_t i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        if (GDMA_Send_Buf[i] != GDMA_Recv_Buf[i])
        {
            APP_PRINT_INFO2("[io_gdma] io_handle_gdma_msg: Data transmission error! GDMA_Send_Buf = %d, GDMA_Recv_Buf = %d",
                            GDMA_Send_Buf[i], GDMA_Recv_Buf[i]);
        }
    }
}

/**
  * @brief  GDMA channel interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void GDMA_Channel_Handler(void)
{
    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Transfer, DISABLE);

    T_IO_MSG int_gdma_msg;

    int_gdma_msg.type = IO_MSG_TYPE_GDMA;
    int_gdma_msg.subtype = 0;
    if (false == app_send_msg_to_apptask(&int_gdma_msg))
    {
        APP_PRINT_ERROR0("[io_gdma] GDMA_Channel_Handler: Send int_gdma_msg failed!");
        //Add user code here!
        GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
        return;
    }

    GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
}


