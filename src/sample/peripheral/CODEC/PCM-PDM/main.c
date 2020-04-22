/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     pdm_demo.c
* @brief    PDM demo code
* @details
* @author   elliot chen
* @date     2017-12-12
* @version  v1.0
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
#include "rtl876x_wdg.h"

/* Defines ------------------------------------------------------------------*/
/* Pin define of PDM peripheral */
#define PDM_CLK_PIN                     P2_0
#define PDM_DAT_PIN                     P2_1

/* Audio operation ---------------------------------------------*/
#define AUDIO_FRAME_SIZE                16000

/* GDMA RX defines */
#define GDMA_Channel_AMIC_NUM           0
#define GDMA_Channel_AMIC               GDMA_Channel0
#define GDMA_Channel_AMIC_IRQn          GDMA0_Channel0_IRQn
#define GDMA_Channel_AMIC_Handler       GDMA0_Channel0_Handler

uint32_t audio_offset = 0;

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_codec_init(void)
{
    Pad_Config(H_0, PAD_SW_MODE, PAD_NOT_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(P2_6, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(P2_7, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);

    Pad_Config(PDM_CLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(PDM_DAT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Config(PDM_CLK_PIN, LRC_I_PCM);
    Pinmux_Config(PDM_DAT_PIN, BCLK_I_PCM);

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
    CODEC_InitStruct.CODEC_MicType           = CODEC_AMIC;
    CODEC_InitStruct.CODEC_MICBstMode        = MICBST_Mode_Differential;
    CODEC_InitStruct.CODEC_SampleRate        = SAMPLE_RATE_16KHz;
    CODEC_InitStruct.CODEC_I2SFormat         = CODEC_I2S_DataFormat_I2S;
    CODEC_InitStruct.CODEC_I2SDataWidth      = CODEC_I2S_DataWidth_16Bits;
    /* PDM initialization parameters for output */
    CODEC_InitStruct.CODEC_DaMute            = DAC_UuMute;
    CODEC_InitStruct.CODEC_DaGain            = 0x7F;
    CODEC_InitStruct.CODEC_DacZeroDetTimeout = DAC_Zero_DetTimeout_1024_16_Sample;
    CODEC_Init(CODEC, &CODEC_InitStruct);
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
    I2S_InitStruct.I2S_BClockMi         = 907;           /* BCLK = 40MHz*(I2S_BClockNi/I2S_BClockMi)  \
                                                         bitwidth fix 32bit*2 ex.| 625 = 16K | 907= 11.025k | */
    I2S_InitStruct.I2S_BClockNi         = 0x10;
    I2S_InitStruct.I2S_DeviceMode       = I2S_DeviceMode_Master;
    I2S_InitStruct.I2S_ChannelType      = I2S_Channel_Mono;
    I2S_InitStruct.I2S_DataFormat       = I2S_Mode;
    I2S_InitStruct.I2S_TxWaterlevel     = 16;
    I2S_Init(I2S0, &I2S_InitStruct);
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
    GDMA_InitStruct.GDMA_ChannelNum          = GDMA_Channel_AMIC_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_PeripheralToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = AUDIO_FRAME_SIZE / 4;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)(&(I2S0->RX_DR));
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(&(I2S0->TX_DR));
    GDMA_InitStruct.GDMA_SourceHandshake     = GDMA_Handshake_SPORT0_RX;
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_SPORT0_TX;
    GDMA_Init(GDMA_Channel_AMIC, &GDMA_InitStruct);
    GDMA_INTConfig(GDMA_Channel_AMIC_NUM, GDMA_INT_Transfer, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GDMA_Channel_AMIC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_flash_to_i2s_gdma_init(uint32_t addr, uint32_t len)
{
    /* Disable SPIC module */
    SPIC->ssienr &= ~(BIT(0));
    SPIC->dmardlr = 16;
    /* Enable SPIC module */
    SPIC->ssienr = BIT(0);
    /* Enable GDMA clock */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    /* Initialize GDMA peripheral */
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = GDMA_Channel_AMIC_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = len / 4;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_16;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_16;
    GDMA_InitStruct.GDMA_SourceAddr          = addr;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(&(I2S0->TX_DR));
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_SPORT0_TX;
    GDMA_Init(GDMA_Channel_AMIC, &GDMA_InitStruct);
    GDMA_INTConfig(GDMA_Channel_AMIC_NUM, GDMA_INT_Transfer, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GDMA_Channel_AMIC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Demo code of codec pdm.
  * @param  No parameter.
  * @return Void
*/
void codec_demo(void)
{
    board_codec_init();
    driver_i2s_init();
    //driver_gdma_init();
    audio_offset = 0;
    driver_flash_to_i2s_gdma_init(0x1830000 + audio_offset, AUDIO_FRAME_SIZE);
    //audio_offset += AUDIO_FRAME_SIZE;
    driver_codec_init();

    I2S_Cmd(I2S0, I2S_MODE_TX | I2S_MODE_RX, ENABLE);
    GDMA_Cmd(GDMA_Channel_AMIC_NUM, ENABLE);
}

/**
  * @brief  GDMA interrupt handler function.
  * @param  No parameter.
  * @return Void
*/
void GDMA_Channel_AMIC_Handler(void)
{
    GDMA_ClearINTPendingBit(GDMA_Channel_AMIC_NUM, GDMA_INT_Transfer);

    if (audio_offset <= 110000)
    {
        audio_offset += AUDIO_FRAME_SIZE;
    }
    else
    {
        audio_offset = 0;//AUDIO_FRAME_SIZE;
    }
    driver_flash_to_i2s_gdma_init(0x1830000 + audio_offset, AUDIO_FRAME_SIZE);
    GDMA_Cmd(GDMA_Channel_AMIC_NUM, ENABLE);
}
/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

int main()
{
    WDG_Disable();
    __enable_irq();
    codec_demo();
    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
    }
}
