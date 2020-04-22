/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     dmic_demo.c
* @brief    DMIC demo code
* @details
* @author   elliot chen
* @date     2017-12-12
* @version  v1.0
*********************************************************************************************************
*/

/* Includes -----------------------------------------------------------------*/
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_codec.h"
#include "rtl876x_i2s.h"
#include "rtl876x_uart.h"
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"
#include "rtl876x_wdg.h"
#include "board.h"
#include "os_sched.h"
#include "trace.h"
/* Defines ------------------------------------------------------------------*/

/* Audio operation ---------------------------------------------*/
#define AUDIO_FRAME_SIZE                4000

/* GDMA RX defines */
#define GDMA_Channel_DMIC               GDMA_Channel0
#define GDMA_Channel_DMIC_NUM           0
#define GDMA0_Channel_DMIC_IRQn         GDMA0_Channel0_IRQn
#define GDMA0_Channel_DMIC_Handler      GDMA0_Channel0_Handler

/**
  * @brief  initialization of pinmux settings and pad settings.
  * @param   No parameter.
  * @return  void
  */
void Board_dmic_Init(void)
{
    Pad_Config(DMIC_MSBC_CLK, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(DMIC_MSBC_DAT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(UART_TX, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(UART_RX, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pinmux_Config(DMIC_MSBC_CLK, DMIC1_CLK);
    Pinmux_Config(DMIC_MSBC_DAT, DMIC1_DAT);
    Pinmux_Config(UART_TX, UART0_TX);
    Pinmux_Config(UART_RX, UART0_RX);
}

/**
  * @brief  Initialize codec peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_DMIC_Codec_Init(void)
{
    RCC_PeriphClockCmd(APBPeriph_CODEC, APBPeriph_CODEC_CLOCK, ENABLE);

    CODEC_InitTypeDef CODEC_InitStruct;
    CODEC_StructInit(&CODEC_InitStruct);
    CODEC_InitStruct.CODEC_MicType          = CODEC_DMIC;
    CODEC_InitStruct.CODEC_DmicClock        = DMIC_Clock_2MHz;
    CODEC_InitStruct.CODEC_DmicDataLatch    = DMIC_Rising_Latch;
    CODEC_InitStruct.CODEC_BoostGain        = Boost_Gain_0dB;
    CODEC_InitStruct.CODEC_SampleRate       = SAMPLE_RATE_16KHz;
    CODEC_InitStruct.CODEC_I2SFormat        = CODEC_I2S_DataFormat_I2S;
    CODEC_InitStruct.CODEC_I2SDataWidth     = CODEC_I2S_DataWidth_16Bits;
    CODEC_Init(CODEC, &CODEC_InitStruct);
}

/**
  * @brief  Initialize I2S peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_DMIC_I2S_Init(void)
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
    I2S_InitStruct.I2S_RxWaterlevel       = 0x4;
    I2S_InitStruct.I2S_TxWaterlevel       = 0x8;
    I2S_Init(I2S0, &I2S_InitStruct);

    I2S_Cmd(I2S0,  I2S_MODE_TX | I2S_MODE_RX, ENABLE);
}

/**
  * @brief  Initialize UART peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_UART_Init(void)
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
  * @brief  Initialize GDMA peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_GDMARx_Init(void)
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
    NVIC_InitStruct.NVIC_IRQChannel = GDMA0_Channel_DMIC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
* @brief  main() is the entry of user code.
* @param   No parameter.
* @return  void
*/
void dmic_demo(void)
{
    Board_dmic_Init();

    Driver_DMIC_I2S_Init();
    Driver_UART_Init();
    Driver_GDMARx_Init();
    Driver_DMIC_Codec_Init();
    I2S_Cmd(I2S0,  I2S_MODE_TX | I2S_MODE_RX, ENABLE);
    GDMA_Cmd(GDMA_Channel_DMIC_NUM, ENABLE);
}

/**
* @brief GDMA interrupt handler function.
* @param   No parameter.
* @return  void
*/
void GDMA0_Channel_DMIC_Handler(void)
{
    GDMA_SetSourceAddress(GDMA_Channel_DMIC, (uint32_t)(&(I2S0->RX_DR)));
    GDMA_SetDestinationAddress(GDMA_Channel_DMIC, (uint32_t)(&(UART->RB_THR)));

    GDMA_SetBufferSize(GDMA_Channel_DMIC, AUDIO_FRAME_SIZE / 4);

    GDMA_ClearINTPendingBit(GDMA_Channel_DMIC_NUM, GDMA_INT_Transfer);
    GDMA_Cmd(GDMA_Channel_DMIC_NUM, ENABLE);


}
int main()
{
    WDG_Disable();
    __enable_irq();
    dmic_demo();
    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
    }
}
