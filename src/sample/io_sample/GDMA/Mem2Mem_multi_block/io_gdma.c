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
//uint32_t GDMA_Send_Buf[GDMA_TRANSFER_SIZE];
//uint32_t GDMA_Recv_Buf[GDMA_TRANSFER_SIZE];
uint8_t GDMA_Send_Buffer[GDMA_MULTIBLOCK_SIZE][GDMA_TRANSFER_SIZE];
uint8_t GDMA_Recv_Buffer[GDMA_MULTIBLOCK_SIZE][GDMA_TRANSFER_SIZE];
GDMA_LLIDef GDMA_LLIStruct[GDMA_MULTIBLOCK_SIZE];

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_gdma_init(void)
{
    uint32_t i, j = 0;

    /*--------------Initialize test buffer---------------------*/
    for (i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        for (j = 0; j < GDMA_MULTIBLOCK_SIZE; j++)
        {
            GDMA_Send_Buffer[j][i] = (i + j) & 0xff;
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

    /** Either single block transmission completion interruption or transmission completion interruption can be choose.
      * Synchronized modifications are also required in GDMA_Channel_Handler if a single block transmission interrupt is used.
      */
#if (GDMA_INTERRUPT_MODE == INT_TRANSFER)
    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
#elif (GDMA_INTERRUPT_MODE == INT_BLOCK)
    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Block, ENABLE);
#endif
    /** To debug, enable GDMA transmission when the GAP stack is ready.
      * In app.c->app_handle_dev_state_evt()
      */
//    GDMA_Cmd(GDMA_CHANNEL_NUM, ENABLE);
}

#if (GDMA_INTERRUPT_MODE == INT_TRANSFER)
/**
  * @brief  Handle gdma data function.
  * @param  No parameter.
  * @return void
  */
void io_handle_gdma_msg(T_IO_MSG *io_gdma_msg)
{
    APP_PRINT_INFO0("[io_gdma] io_handle_gdma_msg: GDMA transfer data completion!");
    for (uint32_t i = 0; i < GDMA_MULTIBLOCK_SIZE; i++)
    {
        for (uint32_t j = 0; j < GDMA_TRANSFER_SIZE; j++)
        {
            if (GDMA_Send_Buffer[i][j] != GDMA_Recv_Buffer[i][j])
            {
                APP_PRINT_INFO2("[io_gdma]io_handle_gdma_msg: Data transmission error! GDMA_Send_Buffer = %d, GDMA_Recv_Buffer = %d",
                                GDMA_Send_Buffer[i][j], GDMA_Recv_Buffer[i][j]);
            }
            GDMA_Recv_Buffer[i][j] = 0;
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

#elif (GDMA_INTERRUPT_MODE == INT_BLOCK)
static uint8_t GDMA_INT_Block_Counter = 0;
/**
  * @brief  Handle gdma data function.
  * @param  No parameter.
  * @return void
  */
void io_handle_gdma_msg(T_IO_MSG *io_gdma_msg)
{
    uint8_t *p_buf = io_gdma_msg->u.buf;
    APP_PRINT_INFO1("[io_gdma] io_handle_gdma_msg: GDMA block%d transfer data completion!", *p_buf);
    for (uint32_t j = 0; j < GDMA_TRANSFER_SIZE; j++)
    {
        if (GDMA_Send_Buffer[*p_buf][j] != GDMA_Recv_Buffer[*p_buf][j])
        {
            APP_PRINT_INFO2("[io_gdma]io_handle_gdma_msg: Data transmission error! GDMA_Send_Buffer = %d, GDMA_Recv_Buffer = %d",
                            GDMA_Send_Buffer[*p_buf][j], GDMA_Recv_Buffer[*p_buf][j]);
        }
        GDMA_Recv_Buffer[*p_buf][j] = 0;
    }
    GDMA_INT_Block_Counter++;
    if (GDMA_INT_Block_Counter < GDMA_MULTIBLOCK_SIZE)
    {
        GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Block, ENABLE);
    }
    else
    {
        GDMA_INT_Block_Counter = 0;
    }

}
/**
  * @brief  GDMA channel interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void GDMA_Channel_Handler(void)
{
    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Block, DISABLE);

    T_IO_MSG int_gdma_msg;

    int_gdma_msg.type = IO_MSG_TYPE_GDMA;
    int_gdma_msg.subtype = 0;
    int_gdma_msg.u.buf = (void *)&GDMA_INT_Block_Counter;
    if (false == app_send_msg_to_apptask(&int_gdma_msg))
    {
        APP_PRINT_ERROR0("[io_gdma]GDMA_Channel_Handler: Send int_gdma_msg failed!");
        //Add user code here!
        GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Block);
        return;
    }
}

#endif
