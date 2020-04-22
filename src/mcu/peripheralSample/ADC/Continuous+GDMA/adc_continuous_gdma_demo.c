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
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_tim.h"

#include "bee2_adc_lib.h"
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

#define GPIO_OUTPUT_PIN_0                   P4_0
#define GPIO_PIN_OUTPUT                     GPIO_GetPin(GPIO_OUTPUT_PIN_0)

/* ADC sample channel config:P2_0~P2_7,VBAT */
#define ADC_SAMPLE_PIN_0                    P2_2
#define ADC_SAMPLE_PIN_1                    P2_3

#define ADC_SAMPLE_CHANNEL_0                ADC_Channel_Index_2
#define ADC_SAMPLE_CHANNEL_1                ADC_Channel_Index_3

#define ADC_SAMPLE_PERIOD                   (400000)//400K
#define ADC_CONTINUOUS_SAMPLE_PERIOD        (((10000000-5*(ADC_SAMPLE_PERIOD))/(ADC_SAMPLE_PERIOD))-1) // (T-0.5us)=(N+1)/10M 19-255

#define ADC_GDMA_CHANNEL_NUM                3
#define ADC_GDMA_Channel                    GDMA_Channel3
#define ADC_GDMA_Channel_IRQn               GDMA0_Channel3_IRQn
#define ADC_GDMA_Channel_Handler            GDMA0_Channel3_Handler
#define GDMA_TRANSFER_SIZE                  4095

#define ADC_TIMER_PERIOD                    (10)

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_adc_init(void)
{
    Pad_Config(ADC_SAMPLE_PIN_0, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    Pinmux_Config(ADC_SAMPLE_PIN_0, IDLE_MODE);
}

/**
  * @brief  Initialize adc peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_adc_init(void)
{
    ADC_DeInit(ADC);
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    ADC_InitTypeDef ADC_InitStruct;
    ADC_StructInit(&ADC_InitStruct);
    ADC_InitStruct.adcSamplePeriod     = ADC_CONTINUOUS_SAMPLE_PERIOD;
    ADC_InitStruct.adcFifoThd          = 16;
    ADC_InitStruct.adcBurstSize        = 16;
    ADC_InitStruct.schIndex[0]         = EXT_SINGLE_ENDED(ADC_SAMPLE_CHANNEL_0);
    ADC_InitStruct.bitmap              = 0x01;
    ADC_InitStruct.adcPowerAlwaysOnCmd = ADC_POWER_ALWAYS_ON_ENABLE;
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
    uint32_t i = 0;

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
}
/* Defines ------------------------------------------------------------------*/

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_gpio_init(void)
{
    Pad_Config(GPIO_OUTPUT_PIN_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pinmux_Config(GPIO_OUTPUT_PIN_0, DWGPIO);
}

/**
  * @brief  Initialize GPIO peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_gpio_init(void)
{
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_PIN_OUTPUT;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);

}

/**
  * @brief  Demo code of operation about gpio.
  * @param  No parameter.
  * @return void
*/
void gpio_demo(void)
{
    /* Configure pad and pinmux firstly! */
    board_gpio_init();

    /* Initialize gpio peripheral */
    driver_gpio_init();

    GPIO_WriteBit(GPIO_PIN_OUTPUT, (BitAction)(0));
}

/**
  * @brief  Demo code of operation about gdma+adc.
  * @param  No parameter.
  * @return void
*/
void gdma_adc_demo(void)
{
    ADC_CalibrationInit();
    gpio_demo();
    board_adc_init();
    driver_adc_init();
    driver_gdma_adc_init();
    for (uint32_t i = 0; i < 100000; i++);
    ADC_Cmd(ADC, ADC_Continuous_Mode, ENABLE);
    GPIO_WriteBit(GPIO_PIN_OUTPUT, (BitAction)(1));
}

/**
  * @brief  Demo code.
  * @param  No parameter.
  * @return void
*/
void adc_demo(void)
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
    GPIO_WriteBit(GPIO_PIN_OUTPUT, (BitAction)(0));
    ADC_ErrorStatus error_status = NO_ERROR;
    DBG_DIRECT("ADC_GDMA_Channel_Handler!");
    GDMA_ClearINTPendingBit(ADC_GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
    ADC_Cmd(ADC, ADC_Continuous_Mode, DISABLE);
    for (uint32_t i = 0; i < GDMA_TRANSFER_SIZE; i++)
    {
        DBG_DIRECT("ADC sample data[%d] = %10d          %10.2f", i, ADC_Recv_Buffer[i],
                   ADC_GetVoltage(DIVIDE_SINGLE_MODE, ADC_Recv_Buffer[i],
                                  &error_status));
    }
}

