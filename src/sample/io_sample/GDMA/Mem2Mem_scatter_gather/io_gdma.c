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
uint8_t GDMA_Send_Buffer[SOURCE_DATA_SIZE];
uint8_t GDMA_Recv_Buffer[DESTINATION_DATA_SIZE];

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_gdma_init(void)
{
    uint32_t i = 0;

    for (i = 0; i < SOURCE_DATA_SIZE; i++)
    {
        GDMA_Send_Buffer[i] = i;
        APP_PRINT_INFO2("[io_gdma]driver_gdma_init: GDMA send data[%d] = %d", i, GDMA_Send_Buffer[i]);
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

    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    /* GDMA irq config */
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
    APP_PRINT_INFO1("[io_gdma]io_handle_gdma_msg: %d", SOURCE_DATA_SIZE);
    APP_PRINT_INFO1("[io_gdma]io_handle_gdma_msg: %d", DESTINATION_DATA_SIZE);
    for (uint32_t i = 0; i < DESTINATION_DATA_SIZE; i++)
    {
        APP_PRINT_INFO2("GDMA receive data[%d] = %d", i, GDMA_Recv_Buffer[i]);
        GDMA_Recv_Buffer[i] = 0;
    }
    for (uint32_t i = 0; i < 10000000; i++);
    GDMA_SetSourceAddress(GDMA_Channel, (uint32_t)GDMA_Send_Buffer);
    GDMA_SetDestinationAddress(GDMA_Channel, (uint32_t)GDMA_Recv_Buffer);
    GDMA_Cmd(GDMA_CHANNEL_NUM, ENABLE);
}

/**
  * @brief  GDMA channel interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void GDMA_Channel_Handler(void)
{
    GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
    GDMA_Cmd(GDMA_CHANNEL_NUM, DISABLE);

    T_IO_MSG int_gdma_msg;

    int_gdma_msg.type = IO_MSG_TYPE_GDMA;
    int_gdma_msg.subtype = 0;
    if (false == app_send_msg_to_apptask(&int_gdma_msg))
    {
        APP_PRINT_ERROR0("[io_gdma]GDMA_Channel_Handler: Send int_gdma_msg failed!");
        //Add user code here!
        GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
        return;
    }

}

