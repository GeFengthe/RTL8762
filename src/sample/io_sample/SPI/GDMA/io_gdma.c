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
#include "io_uart.h"


/* Globals ------------------------------------------------------------------*/
uint8_t GDMA_WriteCmdBuffer[5] = {0x0B, 0x01, 0x00, 0x00, 0x00};
uint8_t GDMA_Recv_Buffer[GDMA_MULTIBLOCK_SIZE][GDMA_TRANSFER_SIZE];
GDMA_LLIDef GDMA_LLIStruct[GDMA_MULTIBLOCK_SIZE];

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_spi_gdma_init(void)
{
    /*----------------test buffer init------------------*/
    memset(GDMA_Recv_Buffer, 0, sizeof(GDMA_Recv_Buffer) / sizeof(GDMA_Recv_Buffer[0][0]));

    SPI_InitTypeDef  SPI_InitStructure;
    GDMA_InitTypeDef GDMA_InitStruct;

    SPI_DeInit(FLASH_SPI);
    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);

    /*----------------------SPI init---------------------------------*/
    SPI_StructInit(&SPI_InitStructure);
    SPI_InitStructure.SPI_Direction   = SPI_Direction_EEPROM;//SPI_Direction_RxOnly;
    SPI_InitStructure.SPI_Mode        = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_BaudRatePrescaler = 4;
    SPI_InitStructure.SPI_FrameFormat = SPI_Frame_Motorola;
    SPI_InitStructure.SPI_NDF         = GDMA_READ_SIZE_MAX - 1;
    SPI_InitStructure.SPI_RxDmaEn     = ENABLE;
    SPI_InitStructure.SPI_RxWaterlevel = 1;
    SPI_Init(FLASH_SPI, &SPI_InitStructure);

    /*---------------------GDMA initial------------------------------*/
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_PeripheralToMemory;
    GDMA_InitStruct.GDMA_BufferSize          = GDMA_TRANSFER_SIZE;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Inc;

    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;

    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)FLASH_SPI->DR;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)GDMA_Recv_Buffer;
    GDMA_InitStruct.GDMA_SourceHandshake     = GDMA_Handshake_SPI0_RX;
    GDMA_InitStruct.GDMA_ChannelPriority     = 1;
    GDMA_InitStruct.GDMA_Multi_Block_En      = 1;
    GDMA_InitStruct.GDMA_Multi_Block_Mode    = LLI_TRANSFER;
    GDMA_InitStruct.GDMA_Multi_Block_Struct  = (uint32_t)GDMA_LLIStruct;

    for (int i = 0; i < GDMA_MULTIBLOCK_SIZE; i++)
    {
        GDMA_LLIStruct[i].SAR = (uint32_t)FLASH_SPI->DR;
        GDMA_LLIStruct[i].DAR = (uint32_t)GDMA_Recv_Buffer[i];
        if (i == (GDMA_MULTIBLOCK_SIZE - 1))
        {
            GDMA_LLIStruct[i].DAR = (uint32_t)GDMA_Recv_Buffer[i];
            GDMA_LLIStruct[i].LLP = 0;
            /* configure low 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct.GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct.GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct.GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct.GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct.GDMA_DIR << 20);
            /* configure high 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_HIGH = GDMA_InitStruct.GDMA_BufferSize;
        }
        else
        {
            GDMA_LLIStruct[i].LLP = (uint32_t)&GDMA_LLIStruct[i + 1];
            /* configure low 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct.GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct.GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct.GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct.GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct.GDMA_DIR << 20)
                                        | (GDMA_InitStruct.GDMA_Multi_Block_Mode & LLP_SELECTED_BIT);
            /* configure high 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_HIGH = GDMA_InitStruct.GDMA_BufferSize;
        }
    }

    GDMA_Init(GDMA_Channel, &GDMA_InitStruct);

    /*-----------------GDMA IRQ-----------------------------*/
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /* Enable transfer interrupt */
    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Block, ENABLE);
    GDMA_Cmd(GDMA_CHANNEL_NUM, ENABLE);
    SPI_Cmd(FLASH_SPI, ENABLE);

    /* Send read data command */
    SPI_SendBuffer(FLASH_SPI, GDMA_WriteCmdBuffer, 5);
}

/**
  * @brief  Handle gdma data function.
  * @param  No parameter.
  * @return void
  */
uint8_t gdma_transfer_num = 0;
void io_handle_gdma_msg(T_IO_MSG *io_gdma_msg)
{
    uint8_t *p_buf = io_gdma_msg->u.buf;
    uint16_t data_len = (GDMA_TRANSFER_SIZE);
    APP_PRINT_INFO2("[io_gdma] io_handle_gdma_msg: read data complete, num = %d, data_len = %d",
                    gdma_transfer_num, data_len);
    uart_senddata_continuous(UART, &p_buf[gdma_transfer_num * data_len], data_len);
    gdma_transfer_num++;
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
    int_gdma_msg.u.buf = (void *)GDMA_Recv_Buffer;
    if (false == app_send_msg_to_apptask(&int_gdma_msg))
    {
        APP_PRINT_ERROR0("[io_gdma]GDMA_Channel_Handler: Send int_gdma_msg failed!");
        //Add user code here!
        GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Block);
        return;
    }
    GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Block);
    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Block, ENABLE);
}

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/
