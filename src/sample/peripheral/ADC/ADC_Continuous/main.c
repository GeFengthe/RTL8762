/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of ADC Continuous mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
/* Include ---------------------------------------------------------------*/
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_adc.h"
#include "rtl876x_nvic.h"
#include "rtl876x_tim.h"
#include "rtl876x_gdma.h"
#include "rtl876x_wdg.h"
#include "trace.h"
#include "board.h"
#include <os_sched.h>
/* Defines --------------------------------------------------------------*/
#define ADC_SCHEDULE_0               0
#define ADC_SCHEDULE_1               1
#define ADC_SCHEDULE_2               2
#define ADC_SCHEDULE_3               3
#define ADC_SCHEDULE_4               4
#define ADC_SCHEDULE_5               5
#define ADC_SCHEDULE_6               6
#define ADC_SCHEDULE_7               7

#define ADC_SCHEDULE_8               8
#define ADC_SCHEDULE_9               9
#define ADC_SCHEDULE_10              10
#define ADC_SCHEDULE_11              11
#define ADC_SCHEDULE_12              12
#define ADC_SCHEDULE_13              13
#define ADC_SCHEDULE_14              14
#define ADC_SCHEDULE_15              15


#define ADC_CHANNEL_2                2
#define ADC_CHANNEL_4                4

#define ADC_CHANNEL_2_PIN            P2_2
#define ADC_CHANNEL_4_PIN            P2_4

#define ADC_Channel_index               2
#define ADC_Schedule_index              0

//#define ADC_GDMA                       1
#ifdef ADC_GDMA
#define GDMA_TRANSFER_SIZE             1024
uint16_t ADC_Recv_Buffer[GDMA_TRANSFER_SIZE];

#define ADC_GDMA_CHANNEL_NUM                3
#define ADC_GDMA_Channel                    GDMA_Channel3
#define ADC_GDMA_Channel_IRQn               GDMA0_Channel3_IRQn
#define ADC_GDMA_Channel_Handler            GDMA0_Channel3_Handler
#endif
/**
  * @brief  initialization of pinmux settings and pad settings.
  * @param   No parameter.
  * @return  void
  */
void Board_ADC_init(void)
{
    Pad_Config(ADC_CHANNEL_2_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(ADC_CHANNEL_4_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pinmux_Config(ADC_CHANNEL_2_PIN, IDLE_MODE);
    Pinmux_Config(ADC_CHANNEL_4_PIN, IDLE_MODE);
}

/**
  * @brief  Initialize ADC peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_ADC_init(void)
{
    ADC_DeInit(ADC);
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    ADC_InitTypeDef adcInitStruct;
    ADC_StructInit(&adcInitStruct);
    adcInitStruct.schIndex[ADC_SCHEDULE_0]         = EXT_SINGLE_ENDED(ADC_CHANNEL_2);
    adcInitStruct.schIndex[ADC_SCHEDULE_1]         = EXT_SINGLE_ENDED(ADC_CHANNEL_4);
    adcInitStruct.bitmap              = 0x03;
    adcInitStruct.adcFifoThd          = 16;
    adcInitStruct.adcBurstSize        = 16;
    adcInitStruct.adcSamplePeriod     = 255;
    ADC_Init(ADC, &adcInitStruct);
    //ADC->CR |= 1 << 27;
#ifdef ADC_GDMA
    ADC_INTConfig(ADC, ADC_INT_FIFO_RD_ERR, ENABLE);
#else
    ADC_INTConfig(ADC, ADC_INT_FIFO_TH, ENABLE);
#endif
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
#ifndef ADC_GDMA
    ADC_Cmd(ADC, ADC_Continuous_Mode, ENABLE);
#endif
}
#ifdef ADC_GDMA
#include "string.h"
void driver_gdma_adc_init(void)
{
    memset(ADC_Recv_Buffer, 0, GDMA_TRANSFER_SIZE);

    /* GDMA init */
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);

    GDMA_InitStruct.GDMA_ChannelNum      = ADC_GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR             = GDMA_DIR_PeripheralToMemory;
    GDMA_InitStruct.GDMA_BufferSize      = GDMA_TRANSFER_SIZE;//Determine total transfer size
    GDMA_InitStruct.GDMA_SourceInc       = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc  = DMA_DestinationInc_Inc;
    GDMA_InitStruct.GDMA_SourceDataSize  = GDMA_DataSize_HalfWord;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_HalfWord;
    GDMA_InitStruct.GDMA_SourceMsize        = GDMA_Msize_16;
    GDMA_InitStruct.GDMA_DestinationMsize   = GDMA_Msize_16;
    GDMA_InitStruct.GDMA_SourceAddr         = (uint32_t)(&(ADC->FIFO));
    GDMA_InitStruct.GDMA_DestinationAddr    = (uint32_t)ADC_Recv_Buffer;
    GDMA_InitStruct.GDMA_SourceHandshake    = GDMA_Handshake_ADC;

    GDMA_Init(ADC_GDMA_Channel, &GDMA_InitStruct);

    GDMA_INTConfig(ADC_GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);


    /* GDMA irq init */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = ADC_GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /* Start to receive data */
    GDMA_Cmd(ADC_GDMA_CHANNEL_NUM, ENABLE);

    ADC_Cmd(ADC, ADC_Continuous_Mode, ENABLE);
}
/**
  * @brief  GDMA channel interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void ADC_GDMA_Channel_Handler(void)
{
    DBG_DIRECT("ADC_GDMA_Channel_Handler!");
    GDMA_ClearINTPendingBit(ADC_GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
    ADC_Cmd(ADC, ADC_Continuous_Mode, DISABLE);
    ADC_ClearFifo(ADC);
    for (uint32_t i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        DBG_DIRECT("ADC sample data[%d] = %x", i, ADC_Recv_Buffer[i]);
        if (i == 9)
        {
            i = GDMA_TRANSFER_SIZE - 3;
        }
    }
    GDMA_SetDestinationAddress(ADC_GDMA_Channel, (uint32_t)ADC_Recv_Buffer);
    GDMA_Cmd(ADC_GDMA_CHANNEL_NUM, ENABLE);
    ADC_Cmd(ADC, ADC_Continuous_Mode, ENABLE);
}
#endif
/**
* @brief  ADC interrupt handler function.
* @param   No parameter.
* @return  void
*/
void ADC_Handler(void)
{
    uint32_t data = 0;
    //DBG_DIRECT("ADC_Handler!");
    if (ADC_GetIntFlagStatus(ADC, ADC_INT_FIFO_RD_ERR) == SET)
    {
        DBG_DIRECT("ADC read error interrupt!");
        ADC_ClearINTPendingBit(ADC, ADC_INT_FIFO_RD_ERR);
    }

    if (ADC_GetIntFlagStatus(ADC, ADC_INT_FIFO_TH) == SET)
    {
        ADC_Cmd(ADC, ADC_Continuous_Mode, DISABLE);
        uint8_t length = ADC_GetFifoLen(ADC);
        DBG_DIRECT("length = %d", length);
        for (int i = 0; i < length; i++)
        {
            data = ADC_ReadFifoData(ADC);
            //data = ADC_GetRes(data, EXT_SINGLE_ENDED(ADC_Channel_index));
            DBG_DIRECT("%d = %d", i, data);
        }
        ADC_ClearINTPendingBit(ADC, ADC_INT_FIFO_TH);
        ADC_Cmd(ADC, ADC_Continuous_Mode, ENABLE);
    }
}
int main()
{
    WDG_Disable();
    __enable_irq();
    Board_ADC_init();
    Driver_ADC_init();
#ifdef ADC_GDMA
    driver_gdma_adc_init();
#endif
    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
    }
}
