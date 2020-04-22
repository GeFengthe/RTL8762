/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     rtl876x_codec.c
* @brief    This file provides all the CODEC firmware functions.
* @details
* @author   elliot chen
* @date     2017-11-29
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_rcc.h"
#include "rtl876x_codec.h"
#include "rtl876x_pinmux.h"
#include "platform_utils.h"

/**
  * @brief  Initialize the CODEC analog registers in AON area.
  * @param  None.
  * @retval None
  */
static void CODEC_AnalogCircuitInit(void)
{
    uint8_t reg_value = 0;

    /* Enable ADC 1V1 power cut */
    reg_value = btaon_fast_read_safe(0x110);
    btaon_fast_write(0x110, reg_value | 0x01);

    /* Delay 10us */
    platform_delay_us(10);

    reg_value = btaon_fast_read_safe(0x113);
    btaon_fast_write(0x113, reg_value | 0x01);
    /* Enable ADC 1V8 LDO */
    reg_value = btaon_fast_read_safe(0x110);
    btaon_fast_write(0x110, reg_value | 0x04);

    /* Delay 2ms */
    platform_delay_ms(2);
    /* Disable ADC 1V8 LDO current limit */
    reg_value = btaon_fast_read_safe(0x113);
    btaon_fast_write(0x113, reg_value & (~(0x01)));
}

/**
  * @brief  Deinitialize the CODEC analog registers in AON area.
  * @param  None.
  * @retval None
  */
static void CODEC_AnalogCircuitDeInit(void)
{
    uint8_t reg_value = 0;

    /* Disable  1V1 power cut */
    reg_value = btaon_fast_read_safe(0x110);
    btaon_fast_write(0x110, reg_value & (~0x01));
}

/**
  * @brief  Deinitializes the CODEC peripheral registers to their default reset values(turn off CODEC clock).
  * @param  CODECx: selected CODEC peripheral.
  * @retval None
  */
void CODEC_DeInit(CODEC_TypeDef *CODECx)
{
    /* Check the parameters */
    assert_param(IS_CODEC_PERIPH(CODECx));

    /* switch power mode */
    uint8_t reg_value = btaon_fast_read_safe(0x26);
    reg_value |= 0x08;
    btaon_fast_write_safe(0x26, reg_value);

    RCC_PeriphClockCmd(APBPeriph_CODEC, APBPeriph_CODEC_CLOCK, DISABLE);

    CODEC_AnalogCircuitDeInit();
}

/**
  * @brief Initializes the CODEC peripheral according to the specified
  *   parameters in the CODEC_InitStruct
  * @param  CODECx: selected CODEC peripheral.
  * @param  CODEC_InitStruct: pointer to a CODEC_InitTypeDef structure that
  *   contains the configuration information for the specified CODEC peripheral
  * @retval None
  */
void CODEC_Init(CODEC_TypeDef *CODECx, CODEC_InitTypeDef *CODEC_InitStruct)
{
    /* Check the parameters */
    assert_param(IS_CODEC_PERIPH(CODECx));
    assert_param(IS_CODEC_MIC_TYPE(CODEC_InitStruct->CODEC_MicType));
    assert_param(IS_SAMPLE_RATE(CODEC_InitStruct->CODEC_SampleRate));
    assert_param(IS_CODEC_I2S_DATA_FORMAT(CODEC_InitStruct->CODEC_I2SFormat));
    assert_param(IS_CODEC_I2S_DATA_WIDTH(CODEC_InitStruct->CODEC_I2SDataWidth));
    assert_param(IS_AD_GAIN(CODEC_InitStruct->CODEC_AdGain));
    assert_param(IS_BOOST_GAIN(CODEC_InitStruct->CODEC_BoostGain));
    assert_param(IS_ADC_ZERO_DET_TIMEOUT(CODEC_InitStruct->CODEC_AdcZeroDetTimeout));
    assert_param(IS_MICBIAS_CONFIG(CODEC_InitStruct->CODEC_MicBIAS));
    assert_param(IS_MICBST_GAIN(CODEC_InitStruct->CODEC_MICBstGain));
    assert_param(IS_MICBST_MODE(CODEC_InitStruct->CODEC_MICBstMode));
    assert_param(IS_CODEC_DMIC_CLOCK(CODEC_InitStruct->CODEC_DmicClock));
    assert_param(IS_DMIC_LATCH_EDGE(CODEC_InitStruct->CODEC_DmicDataLatch));
    assert_param(IS_PDM_Clock_INVERT_CMD(CODEC_InitStruct->CODEC_PDMClockInvert));
    assert_param(IS_DAC_MUTE(CODEC_InitStruct->CODEC_DaMute));
    assert_param(IS_DA_GAIN(CODEC_InitStruct->CODEC_DaGain));
    assert_param(IS_DAC_ZERO_DET_TIMEOUT(CODEC_InitStruct->CODEC_DacZeroDetTimeout));

    /* Switch power mode */
    uint8_t reg_value = btaon_fast_read_safe(0x26);
    reg_value &= (~0x08);
    btaon_fast_write_safe(0x26, reg_value);

    if (CODEC_InitStruct->CODEC_MicType == CODEC_AMIC)
    {
        /* Analog initialization in AON register */
        CODEC_AnalogCircuitInit();

        if (CODEC_InitStruct->CODEC_MICBstMode == MICBST_Mode_Single)
        {
            uint8_t reg_value = 0;
            /* Enable ADC 1V1 power cut */
            reg_value = btaon_fast_read_safe(0x113);
            btaon_fast_write(0x113, reg_value | 0x02);
        }

        /* Enable AD/DA clock and ADC analog power */
        CODECx->CR0 = CODEC_ADC_ANA_POW_MSK | CODEC_DTSDM_CLK_EN_MSK | CODEC_DAC_ADDACK_POW_MSK;
        CODECx->CR1 = CODEC_MICBIAS_ENCHX_MSK | CODEC_MICBIAS_POW_MSK | CODEC_InitStruct->CODEC_MicBIAS | \
                      CODEC_InitStruct->CODEC_MICBstMode | CODEC_MICBST_VREF_POW_MSK | \
                      CODEC_InitStruct->CODEC_MICBstGain | CODEC_MICBST_MUTE_MIC_MSK | \
                      CODEC_MICBST_POW_DEFALUT_MSK | CODEC_VREF_SEL_DEFALUT_MSK;
        CODECx->CR2 = CODEC_MICBIAS_POWSHDT_DEFALUT_MSK | CODEC_MICBIAS_OCSEL_DEFALUT_MSK | \
                      CODEC_MICBIAS_COUNT_DEFALUT_MSK;
        CODECx->CR3 = CODEC_CKX_MICBIAS_EN_MSK;
    }

    /*  Reset audio digital IP */
    CODECx->AUDIO_CTRL = 0;
    CODECx->AUDIO_CTRL = CODEC_AUDIO_RST_N_MSK;

    /* Configure codec mic type, clock and sample rate */
    CODECx->CLK_CTRL = CODEC_InitStruct->CODEC_MicType | CODEC_InitStruct->CODEC_DmicClock | \
                       CODEC_InitStruct->CODEC_PDMClockInvert | CODEC_ADC_JITTER_SEL_MSK | \
                       CODEC_InitStruct->CODEC_SampleRate | CODEC_AD_FILTER_CLK_MSK;

    /* Configure I2S parameters */
    CODECx->I2S_CTRL = CODEC_InitStruct->CODEC_I2SFormat | CODEC_InitStruct->CODEC_I2SDataWidth | \
                       CODEC_I2S_TX_CH_DEFAULT | CODEC_I2S_RX_CH_DEFAULT;

    /* Configure Audio ADC gain and zero time out */
    CODECx->ADC_CTRL = CODEC_InitStruct->CODEC_DmicDataLatch | CODEC_AD_DC_HPF_EN_MSK | \
                       CODEC_AD_ZDET_TOUT_MSK | CODEC_InitStruct->CODEC_AdcZeroDetTimeout | \
                       CODEC_InitStruct->CODEC_BoostGain | CODEC_InitStruct->CODEC_AdGain;

    /* Configure PDM parametes */
    if (CODEC_InitStruct->CODEC_DaMute != DAC_Mute)
    {
        CODECx->CLK_CTRL |= CODEC_DA_FILTER_CLK_MSK;
    }
    CODECx->DAC_CTRL = CODEC_InitStruct->CODEC_DaMute | CODEC_DA_ZDET_FUNC_MSK | \
                       CODEC_InitStruct->CODEC_DacZeroDetTimeout | CODEC_InitStruct->CODEC_DaGain;
}

/**
  * @brief  Fills each CODEC_InitStruct member with its default value.
  * @param  CODEC_InitStruct: pointer to an CODEC_InitTypeDef structure which will be initialized.
  * @retval None
  */
void CODEC_StructInit(CODEC_InitTypeDef *CODEC_InitStruct)
{
    /* MIC initialization parameters for input */
    CODEC_InitStruct->CODEC_MicType             = CODEC_DMIC;
    CODEC_InitStruct->CODEC_SampleRate          = SAMPLE_RATE_16KHz;
    CODEC_InitStruct->CODEC_I2SFormat           = CODEC_I2S_DataFormat_I2S;
    CODEC_InitStruct->CODEC_I2SDataWidth        = CODEC_I2S_DataWidth_16Bits;
    CODEC_InitStruct->CODEC_AdcZeroDetTimeout   = ADC_Zero_DetTimeout_1024_32_Sample;
    /* Amic initialization parameters */
    CODEC_InitStruct->CODEC_MicBIAS             = MICBIAS_VOLTAGE_1_8;
    CODEC_InitStruct->CODEC_MICBstMode          = MICBST_Mode_Single;
    CODEC_InitStruct->CODEC_MICBstGain          = MICBST_Gain_20dB;
    CODEC_InitStruct->CODEC_AdGain              = 0x2F;
    /* DMIC initilaizaiton parameters */
    CODEC_InitStruct->CODEC_DmicClock           = DMIC_Clock_4MHz;
    CODEC_InitStruct->CODEC_DmicDataLatch       = DMIC_Rising_Latch;
    CODEC_InitStruct->CODEC_BoostGain           = Boost_Gain_0dB;
    /* PDM initialization parameters for output */
    CODEC_InitStruct->CODEC_PDMClockInvert      = PDM_Clock_Invert_Disable;
    CODEC_InitStruct->CODEC_DaMute              = DAC_Mute;
    CODEC_InitStruct->CODEC_DaGain              = 0xFF;
    CODEC_InitStruct->CODEC_DacZeroDetTimeout   = DAC_Zero_DetTimeout_1024_16_Sample;
}

/**
  * @brief Initializes the CODEC EQ module according to the specified
  *   parameters in the CODEC_EQInitStruct
  * @param  CODEC_EQx: selected CODEC EQ channel.
  * @param  CODEC_EQInitStruct: pointer to a CODEC_EQInitTypeDef structure that
  *   contains the configuration information for the specified CODEC EQ channel
  * @retval None
  */
void CODEC_EQInit(CODEC_EQTypeDef *CODEC_EQx, CODEC_EQInitTypeDef *CODEC_EQInitStruct)
{
    CODEC_EQx->EQ_H0 = CODEC_EQInitStruct->CODEC_EQCoefH0;
    CODEC_EQx->EQ_B1 = CODEC_EQInitStruct->CODEC_EQCoefB1;
    CODEC_EQx->EQ_B2 = CODEC_EQInitStruct->CODEC_EQCoefB2;
    CODEC_EQx->EQ_A1 = CODEC_EQInitStruct->CODEC_EQCoefA1;
    CODEC_EQx->EQ_A2 = CODEC_EQInitStruct->CODEC_EQCoefA2 | CODEC_EQInitStruct->CODEC_EQChCmd;
}

/**
  * @brief  Fills each CODEC_EQInitStruct member with its default value.
  * @param  CODEC_EQInitStruct: pointer to an CODEC_EQInitTypeDef structure which will be initialized.
  * @retval None
  */
void CODEC_EQStructInit(CODEC_EQInitTypeDef *CODEC_EQInitStruct)
{
    CODEC_EQInitStruct->CODEC_EQChCmd   = EQ_CH_Cmd_DISABLE;
    /*!< The following all parameters can be 0 to 0x7FFFF, whose physical meaning represents a range of-8 to 7.99 */
    CODEC_EQInitStruct->CODEC_EQCoefH0  = 0;
    CODEC_EQInitStruct->CODEC_EQCoefB1  = 0;
    CODEC_EQInitStruct->CODEC_EQCoefB2  = 0;
    CODEC_EQInitStruct->CODEC_EQCoefA1  = 0;
    CODEC_EQInitStruct->CODEC_EQCoefA2  = 0;
}
/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

