/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    DMIC demo code
* @details
* @author   yuan
* @date     2018-05-29
* @version  v0.1
*********************************************************************************************************
*/

/* Includes -----------------------------------------------------------------*/
#include "rtl876x_codec.h"
#include "rtl876x_gdma.h"
#include "rtl876x_i2s.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_uart.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
/* Pin define of DMIC peripheral */
#define DMIC_MSBC_CLK_PIN               P3_2
#define DMIC_MSBC_DAT_PIN               P3_3

/* Pin define of Data UART peripheral */
#define UART_TX_PIN                     P2_4
#define UART_RX_PIN                     P2_5

#define GDMA_TX_BURST_SIZE              ((uint32_t)1)
#define GDMA_RX_BURST_SIZE              ((uint32_t)1)

/* Audio operation ---------------------------------------------*/
#define AUDIO_FRAME_SIZE                4000

/* GDMA RX defines */
#define GDMA_Channel_DMIC_NUM           0
#define GDMA_Channel_DMIC               GDMA_Channel0
#define GDMA_Channel_DMIC_IRQn          GDMA0_Channel0_IRQn
#define GDMA_Channel_DMIC_Handler       GDMA0_Channel0_Handler

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_codec_init(void)
{
    Pad_Config(DMIC_MSBC_CLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(DMIC_MSBC_DAT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    Pinmux_Config(DMIC_MSBC_CLK_PIN, DMIC1_CLK);
    Pinmux_Config(DMIC_MSBC_DAT_PIN, DMIC1_DAT);
}

/**
  * @brief  Initialization of uart pinmux settings and uart pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_uart_init(void)
{
    Pad_Config(UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);

    Pinmux_Config(UART_TX_PIN, UART0_TX);
    Pinmux_Config(UART_RX_PIN, UART0_RX);
}

/**
  * @brief  Initialize codec peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_codec_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_CODEC, APBPeriph_CODEC_CLOCK, ENABLE);

    CODEC_InitTypeDef CODEC_InitStruct;

    CODEC_StructInit(&CODEC_InitStruct);
    CODEC_InitStruct.CODEC_MicType          = CODEC_DMIC;
    CODEC_InitStruct.CODEC_DmicClock        = DMIC_Clock_2MHz;
    CODEC_InitStruct.CODEC_DmicDataLatch    = DMIC_Rising_Latch;
    CODEC_InitStruct.CODEC_SampleRate       = SAMPLE_RATE_16KHz;
    CODEC_InitStruct.CODEC_I2SFormat        = CODEC_I2S_DataFormat_I2S;
    CODEC_InitStruct.CODEC_I2SDataWidth     = CODEC_I2S_DataWidth_16Bits;
    CODEC_Init(CODEC, &CODEC_InitStruct);
    DBG_DIRECT("Use dmic test!");
}

/**
  * @brief  Initialize i2s peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_i2s_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_I2S0, APBPeriph_I2S0_CLOCK, ENABLE);

    I2S_InitTypeDef I2S_InitStruct;

    I2S_StructInit(&I2S_InitStruct);
    I2S_InitStruct.I2S_ClockSource      = I2S_CLK_40M;
    I2S_InitStruct.I2S_BClockMi         = 0x271;/* <!BCLK = 16K */
    I2S_InitStruct.I2S_BClockNi         = 0x10;
    I2S_InitStruct.I2S_DeviceMode       = I2S_DeviceMode_Master;
    I2S_InitStruct.I2S_ChannelType      = I2S_Channel_Mono;
    I2S_InitStruct.I2S_DataFormat       = I2S_Mode;
    I2S_Init(I2S0, &I2S_InitStruct);
}

/**
  * @brief  Initialize uart peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_uart_init(void)
{
    /* Enable clock */
    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);

    /* UART init */
    UART_InitTypeDef UART_InitStruct;

    UART_StructInit(&UART_InitStruct);
#if 1
    /* change to 3M baudrate */
    UART_InitStruct.div             = 1;
    UART_InitStruct.ovsr            = 8;
    UART_InitStruct.ovsr_adj        = 0x492;
    UART_InitStruct.TxWaterlevel    = 12;
#else
    /* change to 1M baudrate */
    UART_InitStruct.div             = 4;
    UART_InitStruct.ovsr            = 5;
    UART_InitStruct.ovsr_adj        = 0;
    UART_InitStruct.TxWaterlevel    = 12;
#endif

    UART_InitStruct.TxDmaEn         = ENABLE;
    UART_InitStruct.dmaEn           = UART_DMA_ENABLE;
    UART_Init(UART, &UART_InitStruct);

}

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_gdma_init(void)
{
    /* Enable GDMA clock */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    /* Initialize GDMA peripheral */
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = GDMA_Channel_DMIC_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_PeripheralToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = AUDIO_FRAME_SIZE / 4;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_4;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)(&(I2S0->RX_DR));
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(&(UART->RB_THR));
    GDMA_InitStruct.GDMA_SourceHandshake     = GDMA_Handshake_SPORT0_RX;
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_UART0_TX;
    GDMA_Init(GDMA_Channel_DMIC, &GDMA_InitStruct);
    GDMA_INTConfig(GDMA_Channel_DMIC_NUM, GDMA_INT_Transfer, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GDMA_Channel_DMIC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Demo code of codec dmic.
  * @param  No parameter.
  * @return Void
*/
void codec_demo(void)
{
    board_codec_init();
    board_uart_init();

    driver_i2s_init();
    driver_uart_init();
    driver_gdma_init();
    driver_codec_init();

    I2S_Cmd(I2S0, I2S_MODE_TX | I2S_MODE_RX, ENABLE);
    GDMA_Cmd(GDMA_Channel_DMIC_NUM, ENABLE);
}


/**
 * @brief    Entry of APP code
 * @return   int (To avoid compile warning)
 */
int main(void)
{
    __enable_irq();

    codec_demo();

    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();

    }
}

/**
  * @brief  GDMA interrupt handler function.
  * @param  No parameter.
  * @return Void
*/
void GDMA_Channel_DMIC_Handler(void)
{
    GDMA_SetSourceAddress(GDMA_Channel_DMIC, (uint32_t)(&(I2S0->RX_DR)));
    GDMA_SetDestinationAddress(GDMA_Channel_DMIC, (uint32_t)(&(UART->RB_THR)));

    GDMA_SetBufferSize(GDMA_Channel_DMIC, AUDIO_FRAME_SIZE / 4);

    GDMA_ClearINTPendingBit(GDMA_Channel_DMIC_NUM, GDMA_INT_Transfer);
    GDMA_Cmd(GDMA_Channel_DMIC_NUM, ENABLE);
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

