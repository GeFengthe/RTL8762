/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_codec.h
* @brief     header file of codec driver.
* @details
* @author    elliot chen
* @date      2017-11-29
* @version   v1.0
* *********************************************************************************************************
*/


#ifndef _RTL876X_CODEC_H_
#define _RTL876X_CODEC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"
#include "rtl876x_bitfields.h"

/* Peripheral: CODEC */
/* Description: CODEC register defines */

/* Register: CR0 --------------------------------------------------------------*/
/* Description: Control register 0. Offset: 0x00. Address: 0x40022000UL. */

/* CR0[10]: ADC_ANA_POW. 0x1: Powen on. 0x0: Power down. */
#define CODEC_ADC_ANA_POW_POS               (10)
#define CODEC_ADC_ANA_POW_MSK               (0x1 << CODEC_ADC_ANA_POW_POS)
#define CODEC_ADC_ANA_POW_CLR               (~CODEC_ADC_ANA_POW_MSK)
/* CR0[9]: DTSDM_CLK_EN. 0x1: Powen on. 0x0: Power down. */
#define CODEC_DTSDM_CLK_EN_POS              (9)
#define CODEC_DTSDM_CLK_EN_MSK              (0x1 << CODEC_DTSDM_CLK_EN_POS)
#define CODEC_DTSDM_CLK_EN_CLR              (~CODEC_DTSDM_CLK_EN_MSK)
/* CR0[0]: DAC_ADDACK_POW. 0x1: Powen on. 0x0: Power down. */
#define CODEC_DAC_ADDACK_POW_POS            (0)
#define CODEC_DAC_ADDACK_POW_MSK            (0x1 << CODEC_DAC_ADDACK_POW_POS)
#define CODEC_DAC_ADDACK_POW_CLR            (~CODEC_DAC_ADDACK_POW_MSK)

/* Register: CR1 --------------------------------------------------------------*/
/* Description: Control register 1. Offset: 0x04. Address: 0x40022004UL. */

/* CR1[27:26]: VREF_SEL. */
#define CODEC_VREF_SEL_POS                  (26)
#define CODEC_VREF_SEL_MSK                  (0x3 << CODEC_VREF_SEL_POS)
#define CODEC_VREF_SEL_CLR                  (~CODEC_VREF_SEL_MSK)
#define CODEC_VREF_SEL_DEFALUT_MSK          (0x2 << CODEC_VREF_SEL_POS)
/* CR1[25:24]: MICBST_POW. */
#define CODEC_MICBST_POW_POS                (24)
#define CODEC_MICBST_POW_MSK                (0x3 << CODEC_MICBST_POW_POS)
#define CODEC_MICBST_POW_CLR                (~CODEC_MICBST_POW_MSK)
#define CODEC_MICBST_POW_DEFALUT_MSK        (0x2 << CODEC_MICBST_POW_POS)
/* CR1[21:20]: MICBST_MUTE. */
#define CODEC_MICBST_MUTE_POS               (20)
#define CODEC_MICBST_MUTE_MSK               (0x3 << CODEC_MICBST_MUTE_POS)
#define CODEC_MICBST_MUTE_CLR               (~CODEC_MICBST_MUTE_MSK)
#define CODEC_MICBST_MUTE_MIC_MSK           (0x2 << CODEC_MICBST_MUTE_POS)
/* CR1[17:16]: MICBST_GAIN. */
#define CODEC_MICBST_GAIN_POS               (16)
#define CODEC_MICBST_GAIN_MSK               (0x3 << CODEC_MICBST_GAIN_POS)
#define CODEC_MICBST_GAIN_CLR               (~CODEC_MICBST_GAIN_MSK)
/* CR1[15]: MICBST_VREF_POW. */
#define CODEC_MICBST_VREF_POW_POS           (15)
#define CODEC_MICBST_VREF_POW_MSK           (0x1 << CODEC_MICBST_VREF_POW_POS)
#define CODEC_MICBST_VREF_POW_CLR           (~CODEC_MICBST_VREF_POW_MSK)
/* CR1[14]: MICBST_ENDFL. */
#define CODEC_MICBST_ENDFL_POS              (14)
#define CODEC_MICBST_ENDFL_MSK              (0x1 << CODEC_MICBST_ENDFL_POS)
#define CODEC_MICBST_ENDFL_CLR              (~CODEC_MICBST_ENDFL_MSK)
/* CR1[13:11]: MICBIAS_VSET. */
#define CODEC_MICBIAS_VSET_POS              (11)
#define CODEC_MICBIAS_VSET_MSK              (0x7 << CODEC_MICBIAS_VSET_POS)
#define CODEC_MICBIAS_VSET_CLR              (~CODEC_MICBIAS_VSET_MSK)
/* CR1[10]: MICBIAS_POW. 0x1: Powen on. 0x0: Power down. */
#define CODEC_MICBIAS_POW_POS               (10)
#define CODEC_MICBIAS_POW_MSK               (0x1 << CODEC_MICBIAS_POW_POS)
#define CODEC_MICBIAS_POW_CLR               (~CODEC_MICBIAS_POW_MSK)
/* CR1[9]: MICBIAS_ENCHX.  */
#define CODEC_MICBIAS_ENCHX_POS             (9)
#define CODEC_MICBIAS_ENCHX_MSK             (0x1 << CODEC_MICBIAS_ENCHX_POS)
#define CODEC_MICBIAS_ENCHX_CLR             (~CODEC_MICBIAS_ENCHX_MSK)

/* Register: CR2 --------------------------------------------------------------*/
/* Description: Control register 2. Offset: 0x08. Address: 0x40022008UL. */

/* CR2[14]: MICBIAS_POWSHDT. */
#define CODEC_MICBIAS_POWSHDT_POS           (14)
#define CODEC_MICBIAS_POWSHDT_MSK           (0x1 << CODEC_MICBIAS_POWSHDT_POS)
#define CODEC_MICBIAS_POWSHDT_CLR           (~CODEC_MICBIAS_POWSHDT_MSK)
#define CODEC_MICBIAS_POWSHDT_DEFALUT_MSK   (0)
/* CR2[13:12]: MICBIAS_OCSEL. */
#define CODEC_MICBIAS_OCSEL_POS             (12)
#define CODEC_MICBIAS_OCSEL_MSK             (0x3 << CODEC_MICBIAS_OCSEL_POS)
#define CODEC_MICBIAS_OCSEL_CLR             (~CODEC_MICBIAS_OCSEL_MSK)
#define CODEC_MICBIAS_OCSEL_DEFALUT_MSK     (0x1 << CODEC_MICBIAS_OCSEL_POS)
/* CR2[11:8]: MICBIAS_COUNT. */
#define CODEC_MICBIAS_COUNT_POS             (8)
#define CODEC_MICBIAS_COUNT_MSK             (0xF << CODEC_MICBIAS_COUNT_POS)
#define CODEC_MICBIAS_COUNT_CLR             (~CODEC_MICBIAS_COUNT_MSK)
#define CODEC_MICBIAS_COUNT_DEFALUT_MSK     (0x1 << CODEC_MICBIAS_COUNT_POS)

/* Register: CR3 --------------------------------------------------------------*/
/* Description: Control register 3. Offset: 0x10. Address: 0x40022010UL. */

/* CR3[0]: CKX_MICBIAS_EN. */
#define CODEC_CKX_MICBIAS_EN_POS           (0)
#define CODEC_CKX_MICBIAS_EN_MSK           (0x1 << CODEC_CKX_MICBIAS_EN_POS)
#define CODEC_CKX_MICBIAS_EN_CLR           (~CODEC_CKX_MICBIAS_EN_MSK)

/* Register: I2S_CTRL -------------------------------------------------------*/
/* Description: I2S Control register. Offset: 0x14. Address: 0x40022014UL. */

/* I2S_CTRL[7:6]: I2S_TX_CH. */
#define CODEC_I2S_TX_CH_POS                 (6)
#define CODEC_I2S_TX_CH_MSK                 (0x3 << CODEC_I2S_TX_CH_POS)
#define CODEC_I2S_TX_CH_CLR                 (~CODEC_I2S_TX_CH_MSK)
#define CODEC_I2S_TX_CH_DEFAULT             (0x2 << CODEC_I2S_TX_CH_POS)

/* I2S_CTRL[5:4]: I2S_RX_CH. */
#define CODEC_I2S_RX_CH_POS                 (4)
#define CODEC_I2S_RX_CH_MSK                 (0x3 << CODEC_I2S_RX_CH_POS)
#define CODEC_I2S_RX_CH_CLR                 (~CODEC_I2S_RX_CH_MSK)
#define CODEC_I2S_RX_CH_DEFAULT             (0x2 << CODEC_I2S_RX_CH_POS)
/* I2S_CTRL[3:2]: I2S_DATA_LEN. */
#define CODEC_I2S_DATA_LEN_POS              (2)
#define CODEC_I2S_DATA_LEN_MSK              (0x3 << CODEC_I2S_DATA_LEN_POS)
#define CODEC_I2S_DATA_LEN_CLR              (~CODEC_I2S_DATA_LEN_MSK)
/* I2S_CTRL[1:0]: I2S_DATA_FORMAT. */
#define CODEC_I2S_DATA_FORMAT_POS           (0)
#define CODEC_I2S_DATA_FORMAT_MSK           (0x3 << CODEC_I2S_DATA_FORMAT_POS)
#define CODEC_I2S_DATA_FORMAT_CLR           (~CODEC_I2S_DATA_FORMAT_MSK)

/* Register: AUDIO_CTRL -------------------------------------------------------*/
/* Description: Audio Control register. Offset: 0x18. Address: 0x40022018UL. */

/* AUDIO_CTRL[0]: AUDIO_RST_N. 0x1: Powen on. 0x0: Power down. */
#define CODEC_AUDIO_RST_N_POS               (0)
#define CODEC_AUDIO_RST_N_MSK               (0x1 << CODEC_AUDIO_RST_N_POS)
#define CODEC_AUDIO_RST_N_CLR               (~CODEC_AUDIO_RST_N_MSK)

/* Register: CLK_CTRL ----------------------------------------------------------*/
/* Description: Clock Control register. Offset: 0x1C. Address: 0x4002201CUL. */

/* CLK_CTRL[11]: AD_ANA_CLK_EN. 0x1: Enable ADC analog clock. 0x0: Disable ADC analog clock. */
#define CODEC_AD_ANA_CLK_EN_POS             (11)
#define CODEC_AD_ANA_CLK_EN_MSK             (0x1 << CODEC_AD_ANA_CLK_EN_POS)
#define CODEC_AD_ANA_CLK_EN_CLR             (~CODEC_AD_ANA_CLK_EN_MSK)
/* CLK_CTRL[10]: DMIC_CLOCK_EN. 0x1: Enable. 0x0: Disable. */
#define CODEC_DMIC_CLOCK_EN_POS             (10)
#define CODEC_DMIC_CLOCK_EN_MSK             (0x1 << CODEC_DMIC_CLOCK_EN_POS)
#define CODEC_DMIC_CLOCK_EN_CLR             (~CODEC_DMIC_CLOCK_EN_MSK)
/* CLK_CTRL[8]: AMIC_DMIC_SEL. 0x1: ADC path. 0x0: DMIC path. */
#define CODEC_AMIC_DMIC_SEL_POS             (8)
#define CODEC_AMIC_DMIC_SEL_MSK             (0x1 << CODEC_AMIC_DMIC_SEL_POS)
#define CODEC_AMIC_DMIC_SEL_CLR             (~CODEC_AMIC_DMIC_SEL_MSK)
/* CLK_CTRL[7]: PDM_CLOCK_INVERT. 0x1: Enable. 0x0: Disable. */
#define CODEC_PDM_CLOCK_INVERT_POS          (7)
#define CODEC_PDM_CLOCK_INVERT_MSK          (0x1 << CODEC_PDM_CLOCK_INVERT_POS)
#define CODEC_PDM_CLOCK_INVERT_CLR          (~CODEC_PDM_CLOCK_INVERT_MSK)
/* CLK_CTRL[5:4]: DMIC_CLK_SEL.  */
#define CODEC_DMIC_CLK_SEL_POS              (4)
#define CODEC_DMIC_CLK_SEL_MSK              (0x3 << CODEC_DMIC_CLK_SEL_POS)
#define CODEC_DMIC_CLK_SEL_CLR              (~CODEC_DMIC_CLK_SEL_MSK)
/* CLK_CTRL[3]: ADC_JITTER_SEL. */
#define CODEC_ADC_JITTER_SEL_POS            (3)
#define CODEC_ADC_JITTER_SEL_MSK            (0x1 << CODEC_ADC_JITTER_SEL_POS)
#define CODEC_ADC_JITTER_SEL_CLR            (~CODEC_ADC_JITTER_SEL_MSK)
/* CLK_CTRL[2]: SAMPLE_RATE. 0x1: 8K. 0x0: 16K. */
#define CODEC_SAMPLE_RATE_POS               (2)
#define CODEC_SAMPLE_RATE_MSK               (0x1 << CODEC_SAMPLE_RATE_POS)
#define CODEC_SAMPLE_RATE_CLR               (~CODEC_SAMPLE_RATE_MSK)
/* CLK_CTRL[1]: AD_FILTER_CLK. 0x1: Enable clock. 0x0: Disable clock. */
#define CODEC_AD_FILTER_CLK_POS             (1)
#define CODEC_AD_FILTER_CLK_MSK             (0x1 << CODEC_AD_FILTER_CLK_POS)
#define CODEC_AD_FILTER_CLK_CLR             (~CODEC_AD_FILTER_CLK_MSK)
/* CLK_CTRL[0]: DA_FILTER_CLK. 0x1: Enable clock. 0x0: Disable clock. */
#define CODEC_DA_FILTER_CLK_POS             (0)
#define CODEC_DA_FILTER_CLK_MSK             (0x1 << CODEC_DA_FILTER_CLK_POS)
#define CODEC_DA_FILTER_CLK_CLR             (~CODEC_DA_FILTER_CLK_MSK)

/* Register: DAC_CTRL ---------------------------------------------------------*/
/* Description: DAC Control register. Offset: 0x20. Address: 0x40022020UL. */

/* DAC_CTRL[11]: DAC_MUTE. */
#define CODEC_DAC_MUTE_POS                  (11)
#define CODEC_DAC_MUTE_MSK                  (0x1 << CODEC_DAC_MUTE_POS)
#define CODEC_DAC_MUTE_CLR                  (~CODEC_DAC_MUTE_MSK)
/* DAC_CTRL[10]: DA_ZDET_FUNC. */
#define CODEC_DA_ZDET_FUNC_POS              (10)
#define CODEC_DA_ZDET_FUNC_MSK              (0x1 << CODEC_DA_ZDET_FUNC_POS)
#define CODEC_DA_ZDET_FUNC_CLR              (~CODEC_DA_ZDET_FUNC_MSK)

/* DAC_CTRL[9:8]: DAC_ZDET_TOUT. */
#define CODEC_DAC_ZDET_TOUT_POS             (8)
#define CODEC_DAC_ZDET_TOUT_MSK             (0x3 << CODEC_DAC_ZDET_TOUT_POS)
#define CODEC_DAC_ZDET_TOUT_CLR             (~CODEC_DAC_ZDET_TOUT_MSK)

/* Register: ADC_CTRL ---------------------------------------------------------*/
/* Description: ADC Control register. Offset: 0x24. Address: 0x40022024UL. */

/* ADC_CTRL[14]: DMIC_RI_FA_SEL. 0x1: Falling latching. 0x0: Rising latching. */
#define CODEC_DMIC_RI_FA_SEL_POS            (14)
#define CODEC_DMIC_RI_FA_SEL_MSK            (0x1 << CODEC_DMIC_RI_FA_SEL_POS)
#define CODEC_DMIC_RI_FA_SEL_CLR            (~CODEC_DMIC_RI_FA_SEL_MSK)
/* ADC_CTRL[13]: AD_DC_HPF_EN. 0x1: Enable. 0x0: Disable. */
#define CODEC_AD_DC_HPF_EN_POS              (13)
#define CODEC_AD_DC_HPF_EN_MSK              (0x1 << CODEC_AD_DC_HPF_EN_POS)
#define CODEC_AD_DC_HPF_EN_CLR              (~CODEC_AD_DC_HPF_EN_MSK)
/* ADC_CTRL[12]: AD_ZDET_TOUT. */
#define CODEC_AD_ZDET_TOUT_POS              (12)
#define CODEC_AD_ZDET_TOUT_MSK              (0x1 << CODEC_AD_ZDET_TOUT_POS)
#define CODEC_AD_ZDET_TOUT_CLR              (~CODEC_AD_ZDET_TOUT_MSK)
/* ADC_CTRL[11:10]: ADC_ZDET_TOUT. */
#define CODEC_ADC_ZDET_TOUT_POS             (10)
#define CODEC_ADC_ZDET_TOUT_MSK             (0x3 << CODEC_ADC_ZDET_TOUT_POS)
#define CODEC_ADC_ZDET_TOUT_CLR             (~CODEC_ADC_ZDET_TOUT_MSK)
/* ADC_CTRL[9:8]: BOOST_GAIN. */
#define CODEC_BOOST_GAIN_POS                (8)
#define CODEC_BOOST_GAIN_MSK                (0x3 << CODEC_BOOST_GAIN_POS)
#define CODEC_BOOST_GAIN_CLR                (~CODEC_BOOST_GAIN_MSK)
/* ADC_CTRL[7]: ADC_VOL_MUTE. */
#define CODEC_ADC_VOL_MUTE_POS              (7)
#define CODEC_ADC_VOL_MUTE_MSK              (0x1 << CODEC_ADC_VOL_MUTE_POS)
#define CODEC_ADC_VOL_MUTE_CLR              (~CODEC_ADC_VOL_MUTE_MSK)
/* ADC_CTRL[6:0]: ADC_DIGITAL_GAIN. */
#define CODEC_ADC_DIGITAL_GAIN_POS          (0)
#define CODEC_ADC_DIGITAL_GAIN_MSK          (0x7F << CODEC_ADC_DIGITAL_GAIN_POS)
#define CODEC_ADC_DIGITAL_GAIN_CLR          (~CODEC_ADC_DIGITAL_GAIN_MSK)



/** @addtogroup CODEC CODEC
  * @brief Codec driver module
  * @{
  */

/** @defgroup CODEC_Exported_Types CODEC Exported Types
  * @{
  */

/**
 * @brief CODEC initialize parameters
 *
 * codec initialize parameters
 */
typedef struct
{
    uint32_t CODEC_MicType;             /*!< Specifies the mic type, which can be dmic or amic */
    uint32_t CODEC_SampleRate;          /*!< Specifies the sample rate */

    /* I2S parameters section */
    uint32_t CODEC_I2SFormat;           /*!< Specifies the I2S format of codec port */
    uint32_t CODEC_I2SDataWidth;        /*!< Specifies the I2S data width of codec port */

    /* Input: ADC parametes section */
    uint32_t CODEC_MicBIAS;             /*!< Specifies the MICBIAS voltage */
    uint32_t CODEC_AdGain;              /*!< Specifies the ADC digital volume */
    uint32_t CODEC_BoostGain;           /*!< Specifies the boost gain */
    uint32_t CODEC_AdcZeroDetTimeout;   /*!< Specifies the mono ADC zero detection timeout mode control */
    uint32_t CODEC_MICBstGain;          /*!< Specifies the MICBST gain */
    uint32_t CODEC_MICBstMode;          /*!< Specifies the MICBST mode */

    /* Input: Dmic parametes section */
    uint32_t CODEC_DmicClock;           /*!< Specifies the dmic clock */
    uint32_t CODEC_DmicDataLatch;       /*!< Specifies the dmic data latch type */

    /* Output: DAC control */
    uint32_t CODEC_PDMClockInvert;      /*!< Specifies the DAC clock invert status */
    uint32_t CODEC_DaMute;              /*!< Specifies the DAC mute status */
    uint32_t CODEC_DaGain;              /*!< Specifies the DAC gain control */
    uint32_t CODEC_DacZeroDetTimeout;   /*!< Specifies the mono DAC zero detection timeout mode control */
} CODEC_InitTypeDef;

/**
 * @brief CODEC EQ part initialize parameters
 *
 * codec EQ part initialize parameters
 */
typedef struct
{
    uint32_t CODEC_EQChCmd;             /*!< Specifies the EQ channel status */
    uint32_t CODEC_EQCoefH0;            /*!< Specifies the EQ coef.h0. This value can be 0 to 0x7FFFF,
                                        whose physical meaning represents a range of-8 to 7.99 */
    uint32_t CODEC_EQCoefB1;            /*!< Specifies the EQ coef.b1. This value can be 0 to 0x7FFFF,
                                        whose physical meaning represents a range of-8 to 7.99 */
    uint32_t CODEC_EQCoefB2;            /*!< Specifies the EQ coef.b2. This value can be 0 to 0x7FFFF,
                                        whose physical meaning represents a range of-8 to 7.99 */
    uint32_t CODEC_EQCoefA1;            /*!< Specifies the EQ coef.a1. This value can be 0 to 0x7FFFF,
                                        whose physical meaning represents a range of-8 to 7.99 */
    uint32_t CODEC_EQCoefA2;            /*!< Specifies the EQ coef.a2. This value can be 0 to 0x7FFFF,
                                        whose physical meaning represents a range of-8 to 7.99 */
} CODEC_EQInitTypeDef;

/**
  * @}
  */

/** @defgroup CODEC_Exported_Constants CODEC Exported Constants
  * @{
  */

#define IS_CODEC_PERIPH(PERIPH) ((PERIPH) == CODEC)

/** @defgroup CODEC_Mic_Type CODEC Mic Type
  * @{
  */

#define CODEC_AMIC                                 (CODEC_AMIC_DMIC_SEL_MSK | CODEC_AD_ANA_CLK_EN_MSK)
#define CODEC_DMIC                                 (CODEC_DMIC_CLOCK_EN_MSK)

#define IS_CODEC_MIC_TYPE(TYPE) (((TYPE) == CODEC_AMIC) || ((TYPE) == CODEC_DMIC))
/**
  * @}
  */

/** @defgroup DMIC_Clock_Type_Definition DMIC Clock Type Definition
  * @{
  */
#define SAMPLE_RATE_8KHz                            (CODEC_SAMPLE_RATE_MSK)
#define SAMPLE_RATE_16KHz                           ((uint32_t)(0x00))

#define IS_SAMPLE_RATE(RATE) (((RATE) == SAMPLE_RATE_8KHz) || ((RATE) == SAMPLE_RATE_16KHz))

/**
  * @}
  */

/** @defgroup I2S_Data_Format I2S Data Format
  * @{
  */

#define CODEC_I2S_DataFormat_I2S                    ((uint32_t)(0x00))
#define CODEC_I2S_DataFormat_LeftJustified          ((uint32_t)(0x01 << CODEC_I2S_DATA_FORMAT_POS))
#define CODEC_I2S_DataFormat_PCM_A                  ((uint32_t)(0x02 << CODEC_I2S_DATA_FORMAT_POS))
#define CODEC_I2S_DataFormat_PCM_B                  ((uint32_t)(0x03 << CODEC_I2S_DATA_FORMAT_POS))

#define IS_CODEC_I2S_DATA_FORMAT(FORMAT) (((FORMAT) == CODEC_I2S_DataFormat_I2S) || \
                                          ((FORMAT) == CODEC_I2S_DataFormat_LeftJustified) || \
                                          ((FORMAT) == CODEC_I2S_DataFormat_PCM_A) || \
                                          ((FORMAT) == CODEC_I2S_DataFormat_PCM_B))

/**
  * @}
  */

/** @defgroup I2S_Data_Width I2S Data Width
  * @{
  */

#define CODEC_I2S_DataWidth_16Bits                  ((uint32_t)(0x00))
#define CODEC_I2S_DataWidth_24Bits                  ((uint32_t)(0x02 << CODEC_I2S_DATA_LEN_POS))
#define CODEC_I2S_DataWidth_8Bits                   ((uint32_t)(0x03 << CODEC_I2S_DATA_LEN_POS))
#define IS_CODEC_I2S_DATA_WIDTH(WIDTH) (((WIDTH) == CODEC_I2S_DataWidth_16Bits) || \
                                        ((WIDTH) == CODEC_I2S_DataWidth_24Bits) || \
                                        ((WIDTH) == CODEC_I2S_DataWidth_8Bits))

/**
  * @}
  */

/** @defgroup DMIC_Clock_Type_Definition DMIC Clock Type Definition
  * @{
  */
#define DMIC_Clock_4MHz                             ((uint32_t)(0x0))
#define DMIC_Clock_2MHz                             ((uint32_t)(0x01 << CODEC_DMIC_CLK_SEL_POS))
#define DMIC_Clock_1MHz                             ((uint32_t)(0x02 << CODEC_DMIC_CLK_SEL_POS))
#define DMIC_Clock_500KHz                           ((uint32_t)(0x03 << CODEC_DMIC_CLK_SEL_POS))

#define IS_CODEC_DMIC_CLOCK(CLOCK) (((CLOCK) == DMIC_Clock_4MHz) || ((CLOCK) == DMIC_Clock_2MHz) || \
                                    ((CLOCK) == DMIC_Clock_1MHz) || ((CLOCK) == DMIC_Clock_500KHz))

/**
  * @}
  */

/** @defgroup DMIC_Data_Latch_Edge DMIC Data Latch Edge
  * @{
  */

#define DMIC_Rising_Latch                           ((uint32_t)(0x00))
#define DMIC_Falling_Latch                          ((uint32_t)CODEC_DMIC_RI_FA_SEL_MSK)

#define IS_DMIC_LATCH_EDGE(EDGE) (((EDGE) == DMIC_Rising_Latch) || ((EDGE) == DMIC_Falling_Latch))

/**
  * @}
  */

/** @defgroup ADC_Zero_Detection_Timeout  ADC Zero Detection Timeout
  * @{
  */

#define ADC_Zero_DetTimeout_1024_16_Sample          ((uint32_t)(0x00))
#define ADC_Zero_DetTimeout_1024_32_Sample          ((uint32_t)(0x01 << CODEC_ADC_ZDET_TOUT_POS))
#define ADC_Zero_DetTimeout_1024_64_Sample          ((uint32_t)(0x02 << CODEC_ADC_ZDET_TOUT_POS))
#define ADC_Zero_DetTimeout_64_Sample               ((uint32_t)(0x03 << CODEC_ADC_ZDET_TOUT_POS))

#define IS_ADC_ZERO_DET_TIMEOUT(TIMEOUT) (((TIMEOUT) == ADC_Zero_DetTimeout_1024_16_Sample) || \
                                          ((TIMEOUT) == ADC_Zero_DetTimeout_1024_32_Sample) || \
                                          ((TIMEOUT) == ADC_Zero_DetTimeout_1024_64_Sample) || \
                                          ((TIMEOUT) == ADC_Zero_DetTimeout_64_Sample))


/**
  * @}
  */

/** @defgroup CODEC_AdGain Codec Ad Gain
  * @{
  */

#define IS_AD_GAIN(GAIN)            (((GAIN)<= 0x7F))

/**
  * @}
  */

/** @defgroup Boost_Gain Boost Gain
  * @{
  */

#define Boost_Gain_0dB                              ((uint32_t)(0x0))
#define Boost_Gain_12dB                             ((uint32_t)(0x1 << CODEC_BOOST_GAIN_POS))
#define Boost_Gain_24dB                             ((uint32_t)(0x2 << CODEC_BOOST_GAIN_POS))
#define Boost_Gain_36dB                             ((uint32_t)(0x3 << CODEC_BOOST_GAIN_POS))

#define IS_BOOST_GAIN(GAIN)             (((GAIN) == Boost_Gain_0dB) || \
                                         ((GAIN) == Boost_Gain_12dB) || \
                                         ((GAIN) == Boost_Gain_24dB) || \
                                         ((GAIN) == Boost_Gain_36dB))

/**
  * @}
  */

/** @defgroup MIC_Volume MIC Volume: 0x00:17.625dB, 0x2f: 0dB, 0x7f: 30dB
  * @{
  */

#define IS_CODEC_MIC_VOLUME_CONFIG(CONFIG) (CONFIG <= 0x7F)

/**
  * @}
  */

/** @defgroup CODEC_MIC_BIAS_Config CODEC MIC BIAS Config
  * @{
  */

#define MICBIAS_VOLTAGE_1_507                       ((uint32_t)(0x00))
#define MICBIAS_VOLTAGE_1_62                        ((uint32_t)(0x01 << CODEC_MICBIAS_VSET_POS))
#define MICBIAS_VOLTAGE_1_705                       ((uint32_t)(0x02 << CODEC_MICBIAS_VSET_POS))
#define MICBIAS_VOLTAGE_1_8                         ((uint32_t)(0x03 << CODEC_MICBIAS_VSET_POS))
#define MICBIAS_VOLTAGE_1_906                       ((uint32_t)(0x04 << CODEC_MICBIAS_VSET_POS))
#define MICBIAS_VOLTAGE_2_025                       ((uint32_t)(0x05 << CODEC_MICBIAS_VSET_POS))
#define MICBIAS_VOLTAGE_2_16                        ((uint32_t)(0x06 << CODEC_MICBIAS_VSET_POS))
#define MICBIAS_VOLTAGE_2_314                       ((uint32_t)(0x07 << CODEC_MICBIAS_VSET_POS))

#define IS_MICBIAS_CONFIG(CONFIG) (((CONFIG) == MICBIAS_VOLTAGE_1_507) || ((CONFIG) == MICBIAS_VOLTAGE_1_62) || \
                                   ((CONFIG) == MICBIAS_VOLTAGE_1_705) || ((CONFIG) == MICBIAS_VOLTAGE_1_8) || \
                                   ((CONFIG) == MICBIAS_VOLTAGE_1_906) || ((CONFIG) == MICBIAS_VOLTAGE_2_025) || \
                                   ((CONFIG) == MICBIAS_VOLTAGE_2_16) || ((CONFIG) == MICBIAS_VOLTAGE_2_314))

/**
  * @}
  */

/** @defgroup MICBST_Gain MICBST Gain
  * @{
  */

#define MICBST_Gain_0dB                             ((uint32_t)(0x0))
#define MICBST_Gain_20dB                            ((uint32_t)(0x1 << CODEC_MICBST_GAIN_POS))
#define MICBST_Gain_30dB                            ((uint32_t)(0x2 << CODEC_MICBST_GAIN_POS))
#define MICBST_Gain_40dB                            ((uint32_t)(0x3 << CODEC_MICBST_GAIN_POS))

#define IS_MICBST_GAIN(GAIN)            (((GAIN) == MICBST_Gain_0dB) || \
                                         ((GAIN) == MICBST_Gain_20dB) || \
                                         ((GAIN) == MICBST_Gain_30dB) || \
                                         ((GAIN) == MICBST_Gain_40dB))

/**
  * @}
  */

/** @defgroup MICBST_Mode MICBST Mode
  * @{
  */

#define MICBST_Mode_Single                          ((uint32_t)(0x0))
#define MICBST_Mode_Differential                    ((uint32_t)CODEC_MICBST_ENDFL_MSK)
#define IS_MICBST_MODE(MODE)            (((MODE) == MICBST_Mode_Single) || \
                                         ((MODE) == MICBST_Mode_Differential))

/**
  * @}
  */

/** @defgroup PDM_Clock_Invert_Cmd PDM Clock Invert Cmd
  * @{
  */

#define PDM_Clock_Invert_Disable                    ((uint32_t)(0x00))
#define PDM_Clock_Invert_Enable                     ((uint32_t)CODEC_PDM_CLOCK_INVERT_MSK)

#define IS_PDM_Clock_INVERT_CMD(CMD) (((CMD) == PDM_Clock_Invert_Disable) || ((CMD) == PDM_Clock_Invert_Enable))

/**
  * @}
  */

/** @defgroup DAC_Mute_Control DAC Mute Control
  * @{
  */

#define DAC_UuMute                                  ((uint32_t)(0x0))
#define DAC_Mute                                    (CODEC_DAC_MUTE_MSK)

#define IS_DAC_MUTE(MUTE)               (((MUTE) == DAC_UuMute) || ((MUTE) == DAC_Mute))

/**
  * @}
  */

/** @defgroup CODEC_DaGain Codec Da Gain
  * @{
  */

#define IS_DA_GAIN(GAIN)            (((GAIN)<= 0xFF))

/**
  * @}
  */

/** @defgroup DAC_Zero_Detection_Timeout  DAC Zero Detection Timeout
  * @{
  */

#define DAC_Zero_DetTimeout_1024_16_Sample          ((uint32_t)(0x00 << CODEC_DAC_ZDET_TOUT_POS))
#define DAC_Zero_DetTimeout_1024_32_Sample          ((uint32_t)(0x01 << CODEC_DAC_ZDET_TOUT_POS))
#define DAC_Zero_DetTimeout_1024_64_Sample          ((uint32_t)(0x02 << CODEC_DAC_ZDET_TOUT_POS))
#define DAC_Zero_DetTimeout_256_Sample              ((uint32_t)(0x03 << CODEC_DAC_ZDET_TOUT_POS))

#define IS_DAC_ZERO_DET_TIMEOUT(TIMEOUT) (((TIMEOUT) == DAC_Zero_DetTimeout_1024_16_Sample) || \
                                          ((TIMEOUT) == DAC_Zero_DetTimeout_1024_32_Sample) || \
                                          ((TIMEOUT) == DAC_Zero_DetTimeout_1024_64_Sample) || \
                                          ((TIMEOUT) == DAC_Zero_DetTimeout_256_Sample))

/**
  * @}
  */

/** @defgroup EQ_CH_Cmd  EQ channel Cmd
  * @{
  */

#define EQ_CH_Cmd_ENABLE                            ((uint32_t)(0x01 << 19))
#define EQ_CH_Cmd_DISABLE                           ((uint32_t)(0x00))

#define IS_EQ_CH_CMD_STATUS(STATUS) (((STATUS) == EQ_CH_Cmd_ENABLE) || \
                                     ((STATUS) == EQ_CH_Cmd_DISABLE))

/**
  * @}
  */

/**
  * @}
  */


/** @defgroup CODEC_Exported_Functions CODEC Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the CODEC peripheral registers to their default reset values(turn off CODEC clock).
  * @param  CODECx: selected CODEC peripheral.
  * @retval None
  */
void CODEC_DeInit(CODEC_TypeDef *CODECx);

/**
  * @brief Initializes the CODEC peripheral according to the specified
  *   parameters in the CODEC_InitStruct
  * @param  CODECx: selected CODEC peripheral.
  * @param  CODEC_InitStruct: pointer to a CODEC_InitTypeDef structure that
  *   contains the configuration information for the specified CODEC peripheral
  * @retval None
  */
void CODEC_Init(CODEC_TypeDef *CODECx, CODEC_InitTypeDef *CODEC_InitStruct);

/**
  * @brief  Fills each CODEC_InitStruct member with its default value.
  * @param  CODEC_InitStruct: pointer to an CODEC_InitTypeDef structure which will be initialized.
  * @retval None
  */
void CODEC_StructInit(CODEC_InitTypeDef *CODEC_InitStruct);

/**
  * @brief Initializes the CODEC EQ module according to the specified
  *   parameters in the CODEC_EQInitStruct
  * @param  CODEC_EQx: selected CODEC EQ channel.
  * @param  CODEC_EQInitStruct: pointer to a CODEC_EQInitTypeDef structure that
  *   contains the configuration information for the specified CODEC EQ channel
  * @retval None
  */
void CODEC_EQInit(CODEC_EQTypeDef *CODEC_EQx, CODEC_EQInitTypeDef *CODEC_EQInitStruct);

/**
  * @brief  Fills each CODEC_EQInitStruct member with its default value.
  * @param  CODEC_EQInitStruct: pointer to an CODEC_EQInitTypeDef structure which will be initialized.
  * @retval None
  */
void CODEC_EQStructInit(CODEC_EQInitTypeDef *CODEC_EQInitStruct);

/**
  * @brief  Reset CODEC.
  * @param  CODECx: selected CODEC peripheral..
  * @retval None
  */
__STATIC_INLINE void CODEC_Reset(CODEC_TypeDef *CODECx)
{
    /* Check the parameters */
    assert_param(IS_CODEC_PERIPH(CODECx));

    /* CODEC IP reset */
    CODECx->AUDIO_CTRL = 0;
    CODECx->AUDIO_CTRL = CODEC_AUDIO_RST_N_MSK;
}

/**
  * @brief  Set Analog mic mute.
  * @param  CODECx: selected CODEC peripheral.
  * @param  NewState: new state of the specified CODEC interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
__STATIC_INLINE void CODEC_SetMicMute(CODEC_TypeDef *CODECx, FunctionalState newState)
{
    /* Check the parameters */
    assert_param(IS_CODEC_PERIPH(CODECx));
    assert_param(IS_FUNCTIONAL_STATE(newState));

    if (newState == ENABLE)
    {
        CODECx->ADC_CTRL |= CODEC_ADC_VOL_MUTE_MSK;
    }
    else
    {
        CODECx->ADC_CTRL &= CODEC_ADC_VOL_MUTE_CLR;
    }
}

/**
  * @brief  Set Amic volume
  * @param  CODECx: selected CODEC peripheral.
  * @param  volume: mic volume.
  * @retval None
  */
__STATIC_INLINE void CODEC_SetADCDigitalVolume(CODEC_TypeDef *CODECx, uint16_t volume)
{
    /* Check the parameters */
    assert_param(IS_CODEC_PERIPH(CODECx));
    assert_param(IS_CODEC_MIC_VOLUME_CONFIG(volume));

    CODECx->ADC_CTRL &= (CODEC_ADC_DIGITAL_GAIN_CLR & CODEC_ADC_VOL_MUTE_CLR);
    CODECx->ADC_CTRL |= volume;
}

/**
  * @brief  configure MIC BIAS Vref voltage.
  * @param  CODECx: selected CODEC peripheral.
  * @param  data: new value of MIC BIAS.
  *   This parameter can be one of the following values:
  *     @arg MICBIAS_VOLTAGE_1_507: Vref voltage is 1.507V.
  *     @arg MICBIAS_VOLTAGE_1_62:  Vref voltage is 1.62V.
  *     @arg MICBIAS_VOLTAGE_1_705: Vref voltage is 1.705V.
  *     @arg MICBIAS_VOLTAGE_1_8: Vref voltage is 1.8V.
  *     @arg MICBIAS_VOLTAGE_1_906: Vref voltage is 1.906V.
  *     @arg MICBIAS_VOLTAGE_2_025:  Vref voltage is 2.025V.
  *     @arg MICBIAS_VOLTAGE_2_16: Vref voltage is 2.16V.
  *     @arg MICBIAS_VOLTAGE_2_314: Vref voltage is 2.314V.
  * @return none.
  */
__STATIC_INLINE void CODEC_SetMICBIAS(CODEC_TypeDef *CODECx,  uint16_t data)

{
    /* Check the parameters */
    assert_param(IS_CODEC_PERIPH(CODECx));
    assert_param(IS_MICBIAS_CONFIG(data));

    CODECx->CR1 &= CODEC_MICBIAS_VSET_CLR & CODEC_MICBIAS_POW_CLR;
    CODECx->CR1 |= data;
}

/**
  * @brief  Enable or disable mic_bias output.
  * @param  CODECx: selected CODEC peripheral.
  * @param  NewState: new state of MICBIAS.
  *   This parameter can be: ENABLE or DISABLE.
  * @return none.
  */
__STATIC_INLINE void CODEC_MICBIASCmd(CODEC_TypeDef *CODECx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_CODEC_PERIPH(CODECx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == ENABLE)
    {
        CODECx->CR1 |= CODEC_MICBIAS_POW_MSK;
    }
    else
    {
        CODECx->CR1 &= CODEC_MICBIAS_POW_CLR;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* _RTL876X_CODEC_H_ */

/**
  * @}
  */

/**
  * @}
  */



/******************* (C) COPYRIGHT 2017 Realtek Semiconductor *****END OF FILE****/

