/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     gdma_adc_demo.c
* @brief    gdma adc demo
* @details
* @author   yuan
* @date     2018-06-28
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_adc.h"
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"

#include "trace.h"

/* Defines ------------------------------------------------------------------*/
#define ADC_Channel_Index_0         0
#define ADC_Channel_Index_1         1
#define ADC_Channel_Index_2         2
#define ADC_Channel_Index_3         3
#define ADC_Channel_Index_4         4
#define ADC_Channel_Index_5         5
#define ADC_Channel_Index_6         6
#define ADC_Channel_Index_7         7

/* ADC sample channel config:P2_0~P2_7,VBAT */
#define ADC_SAMPLE_PIN_0                    P2_2
#define ADC_SAMPLE_PIN_1                    P2_3

#define ADC_SAMPLE_CHANNEL_0                ADC_Channel_Index_2
#define ADC_SAMPLE_CHANNEL_1                ADC_Channel_Index_3

#define ADC_CONTINUOUS_SAMPLE_PERIOD        (200-1)

#define ADC_GDMA_CHANNEL_NUM                3
#define ADC_GDMA_Channel                    GDMA_Channel3
#define ADC_GDMA_Channel_IRQn               GDMA0_Channel3_IRQn
#define ADC_GDMA_Channel_Handler            GDMA0_Channel3_Handler
#define GDMA_TRANSFER_SIZE                  100

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_adc_init(void)
{
    Pad_Config(ADC_SAMPLE_PIN_0, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(ADC_SAMPLE_PIN_1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    Pinmux_Config(ADC_SAMPLE_PIN_0, IDLE_MODE);
    Pinmux_Config(ADC_SAMPLE_PIN_1, IDLE_MODE);
}

/**
  * @brief  Initialize adc peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_adc_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    ADC_InitTypeDef ADC_InitStruct;
    ADC_StructInit(&ADC_InitStruct);
    ADC_InitStruct.adcSamplePeriod     = ADC_CONTINUOUS_SAMPLE_PERIOD;
    ADC_InitStruct.adcFifoThd          = 10;
    ADC_InitStruct.adcBurstSize        = 1;
    ADC_InitStruct.schIndex[0]         = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL_0);
    ADC_InitStruct.schIndex[1]         = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL_1);
    ADC_InitStruct.bitmap              = 0x03;
    ADC_Init(ADC, &ADC_InitStruct);

    ADC_INTConfig(ADC, ADC_INT_FIFO_RD_ERR, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = ADC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
*/
uint16_t ADC_Recv_Buffer[GDMA_TRANSFER_SIZE];
void driver_gdma_adc_init(void)
{
    uint8_t i = 0;

    /* Initialize data buffer which for storing data from adc */
    for (i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        ADC_Recv_Buffer[i] = 0;
    }

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
    GDMA_InitStruct.GDMA_SourceMsize        = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize   = GDMA_Msize_1;
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
}

/**
  * @brief  Demo code of operation about gdma+adc.
  * @param  No parameter.
  * @return void
*/
void gdma_adc_demo(void)
{
    board_adc_init();
    driver_adc_init();
    driver_gdma_adc_init();
    ADC_Cmd(ADC, ADC_Continuous_Mode, ENABLE);
}

/**
  * @brief  Demo code.
  * @param  No parameter.
  * @return void
*/
void gdma_demo(void)
{
    gdma_adc_demo();

}
/**
  * @brief  ADC interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void ADC_Handler(void)
{
    DBG_DIRECT("ADC_Handler!");
    if (ADC_GetIntFlagStatus(ADC, ADC_INT_FIFO_RD_ERR) == SET)
    {
//        DBG_BUFFER(TYPE_BEE2, SUBTYPE_FORMAT, MODULE_ADC, LEVEL_INFO, "ADC read error interrupt!", 0);
        DBG_DIRECT("ADC read error interrupt!");
        ADC_ClearINTPendingBit(ADC, ADC_INT_FIFO_RD_ERR);
    }
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
    for (uint32_t i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        DBG_DIRECT("ADC sample data[%d] = %d", i, ADC_Recv_Buffer[i]);
    }
}

