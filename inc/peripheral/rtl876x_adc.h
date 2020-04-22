/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_adc.h
* @brief     header file of ADC driver.
* @details
* @author    renee
* @date      2016-11-04
* @version   v1.0
* *********************************************************************************************************
*/


#ifndef _RTL876X_ADC_H_
#define _RTL876X_ADC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"
#include "rtl876x_bitfields.h"
#include "platform_utils.h"

/** @addtogroup ADC ADC
  * @brief ADC driver module
  * @{
  */

/*============================================================================*
 *                         Types
 *============================================================================*/


/** @defgroup ADC_Exported_Types ADC Exported Types
  * @{
  */


/**
 * @brief ADC initialize parameters
 *
 *
 */
typedef struct
{
    uint8_t adcConvertTimePeriod;          /* Specifies the ADC Sample convert time.
                                                    This parameter can be a value of 0 to 3*/

    uint8_t adcSamplePeriod;                   /*!< Specifies the ADC Sample clock. adc sample period = (n+1) cycles from 10KHz.
                                                    This parameter can be a value of 0 to 255 */

    uint8_t adcFifoThd;                 /*!< Specifies the ADC fifo threshold to trigger interrupt ADC_INT_FIFO_TH.
                                                    This parameter can be a value of 0 to 31 */

    uint8_t adcBurstSize;               /*!< Specifies the ADC fifo Burst Size to trigger GDMA.
                                                    This parameter can be a value of 0 to 31 */

    uint16_t adcFifoOverWritEn;          /*!< Specifies if Over Write fifo when fifo overflow.
                                                    This parameter can be a value of @ref ADC_over_write_enable */

    uint16_t dataLatchEdge;              /*!< Specifies ADC data latch mode.
                                                    This parameter can be a value of @ref ADC_Latch_Data_Edge */

    uint16_t fifoOREn;                   /*!< Specifies fifo Overread.
                                                    This parameter can be a value of ENABLE or DISABLE */

    uint16_t schIndex[16];                  /*!< Specifies ADC mode and channel for schedule table.
                                                    This parameter can be a value of @ref ADC_schedule_table */

    uint16_t bitmap;                     /*!< Specifies the schedule table channel map.
                                                    This parameter can be a value of 16Bits map */

    uint8_t timerTriggerEn;              /*!< Enable ADC one-shot mode when tim7 toggles */

    uint32_t dataAligned;                 /*!< ADC Data MSB or LSB aligned */

    uint8_t dataWriteToFifo;              /*!< Write ADC one shot mode data into fifo */

    uint8_t dataMinusEn;                  /*!< Data minus the offset before writes to reg/fifo */

    uint8_t dataMinusOffset;             /*!< offset to be minused from */

    /* adc power setting */
    uint32_t pwrmode;                  /*!< Specifies ADC Power mode.
                                                    This parameter can be a value of @ref ADC_Power_mode */

    uint16_t datalatchDly;             /*!< Specifies delay of ck_ad to latch data.*/

    uint16_t adcRG2X0Dly;              /*!< Specifies the power on delay time selection of RG2X_AUXADC[0].
                                                    This parameter can be a value of @ref ADC_RG2X_0_Delay_Time */

    uint16_t adcRG0X1Dly;             /*!< Specifies the power on delay time selection of RG0X_AUXADC[1].
                                                    This parameter can be a value of @ref ADC_RG0X_1_Delay_Time */

    uint16_t adcRG0X0Dly;             /*!< Specifies the power on delay time selection of RG0X_AUXADC[0].
                                                    This parameter can be a value of @ref ADC_RG0X_0_Delay_Time */

    uint32_t adcRefMode;             /*!< Specifies if the peripheral is the source or destination.
                                                    This parameter can be a value of @ref ADC_reference_mode */
    uint32_t adcPowerAlwaysOnCmd;   /*!< Specifies the power always on.
                                                    This parameter can be a value of @ref ADC_power_always_on_cmd */

} ADC_InitTypeDef;

/** End of Group ADC_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/


/** @defgroup ADC_Exported_Constants ADC Exported Constants
  * @{
  */

#define IS_ADC_PERIPH(PERIPH) ((PERIPH) == ADC)

/** @defgroup ADC_Interrupts_Definition ADC Interrupts Definition
  * @{
  */

#define ADC_INT_FIFO_RD_REQ                             ((uint32_t)(1 << 0))
#define ADC_INT_FIFO_RD_ERR                             ((uint32_t)(1 << 1))
#define ADC_INT_FIFO_TH                                 ((uint32_t)(1 << 2))
#define ADC_INT_FIFO_OVERFLOW                           ((uint32_t)(1 << 3))
#define ADC_INT_ONE_SHOT_DONE                           ((uint32_t)(1 << 4))

#define IS_ADC_IT(IT) ((((IT) & 0xFFFFFFE0) == 0x00) && ((IT) != 0x00))

#define IS_ADC_GET_IT(IT) (((IT) == ADC_INT_FIFO_RD_REQ) || ((IT) == ADC_INT_FIFO_RD_ERR)\
                           || ((IT) == ADC_INT_FIFO_TH) || ((IT) == ADC_INT_FIFO_OVERFLOW)\
                           || ((IT) == ADC_INT_ONE_SHOT_DONE))
/** End of Group ADC_Exported_Types
  * @}
  */


/** @defgroup ADC_operation_Mode   ADC Operation mode
  * @{
  */
#define ADC_CONVERT_TIME_500NS          0
#define ADC_CONVERT_TIME_700NS          1
#define ADC_CONVERT_TIME_900NS          2
#define ADC_CONVERT_TIME_1100NS         3

#define ADC_Continuous_Mode                         ((uint16_t)(1 << 0))
#define ADC_One_Shot_Mode                           ((uint16_t)(1 << 1))

#define IS_ADC_MODE(MODE) (((MODE) == ADC_Continuous_Mode) || ((MODE) == ADC_One_Shot_Mode))

/** End of Group ADC_Exported_Types
  * @}
  */

/** @defgroup ADC_Latch_Data_Edge ADC latch data edge
  * @{
  */

#define ADC_Latch_Data_Positive                           ((uint16_t)(0 << 2))
#define ADC_Latch_Data_Negative                           ((uint16_t)(1 << 2))

#define IS_ADC_LATCH_MODE(MODE) (((MODE) == ADC_Latch_Data_Positive) || ((MODE) == ADC_Latch_Data_Negative))

/** End of Group ADC_Exported_Types
  * @}
  */

/** @defgroup ADC_schedule_table ADC channel and mode
  * @{
  */
#define SCHEDULE_TABLE(index)           index

#define EXT_SINGLE_ENDED(index)                     ((uint16_t)((0x00 << 3) | (index)))
#define EXT_DIFFERENTIAL(index)                     ((uint16_t)((0x01 << 3) | (index)))
#define INTERNAL_VBAT_MODE                          ((uint16_t)((0x02 << 3) | 0x00))


#define IS_ADC_SCHEDULE_INDEX_CONFIG(CONFIG) ((((CONFIG) & 0xFFE0) ==0) && ((((~(CONFIG)) & (0x18)) !=0)\
                                                                            || ((CONFIG) == INTERNAL_VBAT_MODE) || ((CONFIG) == INTERNAL_VDDCORE_MODE)\
                                                                            || ((CONFIG) == INTERNAL_VDD_DIGI_MODE)|| ((CONFIG) == INTERNAL_VDD_IO_MODE)))

/** End of Group ADC_schedule_table
  * @}
  */

/** @defgroup ADC_Data_Aligned ADC Data Aligned
  * @{
  */

#define ADC_DATA_LSB                                 (uint32_t)(0 << 30)
#define ADC_DATA_MSB                                 (uint32_t)(1 << 30)

#define IS_ADC_DATA_ALIGN(DATA_ALIGN) (((DATA_ALIGN) == ADC_DATA_LSB) || ((DATA_ALIGN) == ADC_DATA_MSB))

/** End of Group ADC_Data_Aligned
  * @}
  */

/** @defgroup ADC_Data_Minus ADC Data Minus
  * @{
  */

#define ADC_DATA_MINUS_DIS                                 (uint32_t)(0 << 30)
#define ADC_DATA_MINUS_EN                                  ((uint32_t)1 << 30)

#define IS_ADC_DATA_MINUS(DATA_MINUS) (((DATA_MINUS) == ADC_DATA_MINUS_DIS) || ((DATA_MINUS) == ADC_DATA_MINUS_EN))

/** End of Group ADC_Data_Aligned
  * @}
  */

/** @defgroup ADC_Burst_Size ADC Burst Size
  * @{
  */

#define IS_ADC_BURST_SIZE_CONFIG(CONFIG) ((CONFIG) <= 0x3F)

/** End of Group ADC_Burst_Size
  * @}
  */

/** @defgroup ADC_FIFO_Threshold ADC FIFO Threshold
  * @{
  */

#define IS_ADC_FIFO_THRESHOLD(THD) ((THD) <= 0x3F)

/** End of Group ADC_FIFO_Threshold
  * @}
  */

/** @defgroup ADC_Power_mode ADC Power Mode
  * @{
  */

#define ADC_POWER_MANNUAL                             ((uint32_t)(1 << 20))
#define ADC_POWER_AUTO                                ((uint32_t)(0 << 20))

#define IS_ADC_POWER_MODE(MODE) (((MODE) == ADC_POWER_MANNUAL) || ((MODE) == ADC_POWER_AUTO))

/** End of Group ADC_Power_mode
  * @}
  */

/** @defgroup ADC_RG2X_0_Delay_Time  ADC RG2X 0 Delay Time
  * @{
  */

#define ADC_RG2X_0_DELAY_10_US                      ((uint32_t)(0 << 4))
#define ADC_RG2X_0_DELAY_20_US                      ((uint32_t)(1 << 4))
#define ADC_RG2X_0_DELAY_40_US                      ((uint32_t)(2 << 4))
#define ADC_RG2X_0_DELAY_80_US                      ((uint32_t)(3 << 4))

#define IS_ADC_RG2X_0_DELAY_TIME(TIME) (((TIME) == ADC_RG2X_0_DELAY_10_US) || ((TIME) == ADC_RG2X_0_DELAY_20_US)\
                                        || ((TIME) == ADC_RG2X_0_DELAY_40_US) || ((TIME) == ADC_RG2X_0_DELAY_80_US))

/** End of Group ADC_RG2X_0_Delay_Time
  * @}
  */

/** @defgroup ADC_RG0X_1_Delay_Time  ADC RG0X 1 Delay Time
  * @{
  */

#define ADC_RG0X_1_DELAY_40_US                      ((uint16_t)(0 << 2))
#define ADC_RG0X_1_DELAY_80_US                      ((uint16_t)(1 << 2))
#define ADC_RG0X_1_DELAY_160_US                     ((uint16_t)(2 << 2))
#define ADC_RG0X_1_DELAY_320_US                     ((uint16_t)(3 << 2))

#define IS_ADC_RG0X_1_DELAY_TIME(TIME) (((TIME) == ADC_RG0X_1_DELAY_40_US) || ((TIME) == ADC_RG0X_1_DELAY_80_US)\
                                        || ((TIME) == ADC_RG0X_1_DELAY_160_US) || ((TIME) == ADC_RG0X_1_DELAY_320_US))

/** End of Group ADC_RG0X_1_Delay_Time
  * @}
  */

/** @defgroup ADC_RG0X_0_Delay_Time ADC RG0X 0 Delay Time
  * @{
  */

#define ADC_RG0X_0_DELAY_30_US                       ((uint16_t)(0 << 0))
#define ADC_RG0X_0_DELAY_60_US                       ((uint16_t)(1 << 0))
#define ADC_RG0X_0_DELAY_120_US                      ((uint16_t)(2 << 0))
#define ADC_RG0X_0_DELAY_240_US                      ((uint16_t)(3 << 0))

#define IS_ADC_RG0X_0_DELAY_TIME(TIME) (((TIME) == ADC_RG0X_0_DELAY_30_US) || ((TIME) == ADC_RG0X_0_DELAY_60_US)\
                                        || ((TIME) == ADC_RG0X_0_DELAY_120_US) || ((TIME) == ADC_RG0X_0_DELAY_240_US))

/** End of Group ADC_RG0X_0_Delay_Time
  * @}
  */

/** @defgroup ADC_Set_Power_Procedure ADC Set Power Procedure
  * @{
  */

#define ADC_POWERON_VBAT                              (uint32_t)(1 << 10)
#define ADC_POWERON_VA11                              (uint32_t)(1 << 11)
#define ADC_POWERON_RG1X_AUXADC_12                    (uint32_t)(1 << 12)
#define ADC_POWERON_RG2X_AUXADC_3                     (uint32_t)(1 << 13)
#define ADC_POWERON_ERC_VA11                          (uint32_t)(1 << 14)
#define ADC_POWERON_RG2X_AUXADC_0                     (uint32_t)(1 << 15)
#define ADC_POWERON_VA18                              (uint32_t)(1 << 16)
#define ADC_POWERON_RG0X_AUXADC_1                     (uint32_t)(1 << 17)
#define ADC_POWERON_RG0X_AUXADC_0                     (uint32_t)(1 << 18)

#define IS_ADC_POWER_PROCEDURE(CONFIG) (((CONFIG) == ADC_POWERON_VBAT) || ((CONFIG) == ADC_POWERON_VA11)\
                                        || ((CONFIG) == ADC_POWERON_RG1X_AUXADC_12) || ((CONFIG) == ADC_POWERON_RG2X_AUXADC_3))\
|| ((CONFIG) == ADC_POWERON_ERC_VA11) || ((CONFIG) == ADC_POWERON_RG2X_AUXADC_0))\
|| ((CONFIG) == ADC_POWERON_VA18) || ((CONFIG) == ADC_POWERON_RG0X_AUXADC_1))\
|| ((CONFIG) == ADC_POWERON_RG0X_AUXADC_0)))


/** End of Group ADC_Set_Power_Procedure
  * @}
  */


/** @defgroup ADC_over_write_enable ADC FIFO Over Write
  * @{
  */

#define ADC_FIFO_OVER_WRITE_ENABLE                             ((uint32_t)(1 << 13))
#define ADC_FIFO_OVER_WRITE_DISABLE                            ((uint32_t)(0 << 13))

#define IS_ADC_OVERWRITE_MODE(MODE) (((MODE) == ADC_FIFO_OVER_WRITE_ENABLE) || ((MODE) == ADC_FIFO_OVER_WRITE_DISABLE))

/** End of Group ADC_over_write_enable
  * @}
  */

/** @defgroup ADC_reference_mode ADC reference mode
  * @{
  */

#define ADC_Internal_Reference                             ((uint32_t)(0 << 21))
#define ADC_External_Reference                             ((uint32_t)(1 << 21))

#define IS_ADC_REF_MODE(MODE) (((MODE) == ADC_Internal_Reference) || ((MODE) == ADC_External_Reference))

/** End of Group ADC_reference_mode
  * @}
  */

/** @defgroup ADC_power_always_on_cmd ADC power always on cmd
  * @{
  */

#define ADC_POWER_ALWAYS_ON_ENABLE                          ((uint32_t)(1 << 19))
#define ADC_POWER_ALWAYS_ON_DISABLE                         ((uint32_t)(0))

#define IS_ADC_POWER_ALWAYS_ON(CMD) (((CMD) == ADC_POWER_ALWAYS_ON_ENABLE) || ((CMD) == ADC_POWER_ALWAYS_ON_DISABLE))

/** End of Group ADC_Power_Always_On_Cmd
  * @}
  */


/** End of Group ADC_Exported_Constants
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup ADC_Exported_Functions ADC Exported Functions
  * @{
  */

/**
  * @brief Initializes the ADC peripheral according to the specified
  *   parameters in the ADC_InitStruct
  * @param  ADCx: selected ADC peripheral.
  * @param  ADC_InitStruct: pointer to a ADCInitTypeDef structure that
  *   contains the configuration information for the specified ADC peripheral
  * @retval None
  */
extern void ADC_Init(ADC_TypeDef *ADCx, ADC_InitTypeDef *ADC_InitStruct);


/**
  * @brief  Deinitializes the ADC peripheral registers to their default reset values(turn off ADC clock).
  * @param  ADCx: selected ADC peripheral.
  * @retval None
  */
extern void ADC_DeInit(ADC_TypeDef *ADCx);

/**
  * @brief  Fills each ADC_InitStruct member with its default value.
  * @param  ADC_InitStruct: pointer to an ADC_InitTypeDef structure which will be initialized.
  * @retval None
  */
extern void ADC_StructInit(ADC_InitTypeDef *ADC_InitStruct);

/**
  * @brief  Enables or disables the specified ADC interrupts.
  * @param  ADCx: selected ADC peripheral.
  * @param  ADC_IT: specifies the ADC interrupts sources to be enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg ADC_INT_FIFO_RD_REQ :FIFO read request
  *     @arg ADC_INT_FIFO_RD_ERR :FIFO read error
  *     @arg ADC_INT_FIFO_TH :ADC FIFO size>thd
  *     @arg ADC_INT_FIFO_FULL :ADC FIFO overflow
  *     @arg ADC_INT_ONE_SHOT_DONE :ADC one shot mode done
  * @param  newState: new state of the specified ADC interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
extern void ADC_INTConfig(ADC_TypeDef *ADCx, uint32_t ADC_IT, FunctionalState newState);

/**
  * @brief  read ADC data according to specific channel.
  * @param  ADCx: selected ADC peripheral.
  * @param  ScheduleIndex: can be 0 to 15
  * @retval The 12-bit converted ADC data.
  */
extern uint16_t ADC_ReadByScheduleIndex(ADC_TypeDef *ADCx, uint8_t ScheduleIndex);

/**
  * @brief  Enables or disables the ADC peripheral.
  * @param  ADCx: selected ADC peripheral.
  * @param  adcMode: adc mode select.
        This parameter can be one of the following values:
  *     @arg ADC_One_Shot_Mode: one shot mode.
  *     @arg ADC_Auto_Sensor_Mode: compare mode.
  * @param  NewState: new state of the ADC peripheral.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
extern void ADC_Cmd(ADC_TypeDef *ADCx, uint8_t adcMode, FunctionalState NewState);

/**
  * @brief  Config ADC schedule table.
  * @param  ADCx: selected ADC peripheral.
  * @param  Index: Schedule table index.
  * @param  adcMode: ADC mode.
  *      this parameter can be one of the following values:
  *     @arg EXT_SINGLE_ENDED(index)
  *     @arg EXT_DIFFERENTIAL(index)
  *     @arg VREF_AT_CHANNEL7(index)
  *     @arg INTERNAL_VBAT_MODE
  *     @arg INTERNAL_VADPIN_MODE
  *     @arg INTERNAL_VDDCORE_MODE
  *     @arg INTERNAL_VCORE_MODE
  *     @arg INTERNAL_ICHARGE_MODE
  *     @arg INTERNAL_IDISCHG_MODE
  * @return none.
  */
extern void ADC_SchTableConfig(ADC_TypeDef *ADCx, uint16_t Index,
                               uint8_t adcMode);

/**
  * @brief  Data from ADC FIFO.
  * @param  ADCx: selected ADC peripheral.
  * @param[out]  outBuf: buffer to save data read from ADC FIFO.
  * @param  count: number of data to be read.
  * @retval None
  */
extern void ADC_GetFifoData(ADC_TypeDef *ADCx, uint16_t *outBuf, uint16_t count);

/**
  * @brief  Config ADC bypass resistor.Attention!!!Channels using bypass mode cannot over 0.9V!!!!
  * @param  channelNum: external channel number, can be 0~7.
  * @param  NewState: ENABLE or DISABLE.
  * @retval None
  */
extern void ADC_BypassCmd(uint8_t channelNum, FunctionalState NewState);

/**
  * @brief  Get ADC Result
  * @param  RawData: ADC raw data.
  * @param  vSampleMode:
  *      this parameter can be one of the following values:
  *     @arg DIVIDE_SINGLE_MODE
  *     @arg BYPASS_SINGLE_MODE
  *     @arg DIVIDE_DIFFERENTIAL_MODE
  *     @arg BYPASS_DIFFERENTIAL_MODE
  * @retval ADC result
  */
extern int16_t ADC_GetRes(uint16_t vRawData, uint8_t vSampleMode);

/**
  * @brief  Initialize the ADC analog registers in AON area.
  * @param  None.
  * @retval None
  */
extern void ADC_AnalogCircuitConfig(FunctionalState NewState);

/**
  * @brief  ADC power on.
  * @param  ADCx: selected ADC peripheral.
  * @param  NewState: ENABLE or DISABLE.
  * @retval None
  */
void ADC_PowerCmd(ADC_TypeDef *ADCx, FunctionalState NewState);

/**
  * @brief  Get ADC power on status.
  * @param  ADCx: selected ADC peripheral.
  * @retval status: 1: power on sequence finish; 0: power on sequence not completed.
  */
uint8_t ADC_GetPowerOnStatus(ADC_TypeDef *ADCx);

/**
  * @brief  Clears the ADC interrupt pending bits.
  * @param  ADCx: selected ADC peripheral.
  * @param  ADC_IT: specifies the interrupt pending bit to clear.
  *   This parameter can be any combination of the following values:
  *     @arg ADC_INT_ONE_SHOT_DONE: ADC one convert end interrupt
  *     @arg ADC_INT_FIFO_OVERFLOW: ADC FIFO overflow interrupt
  *     @arg ADC_INT_FIFO_RD_REQ: ADC read FIFO request interrupt
  *     @arg ADC_INT_FIFO_RD_ERR: ADC read FIFO error interrupt
  *     @arg ADC_INT_FIFO_TH: fifo larger than threshold
  * @retval None
  */
__STATIC_INLINE void ADC_ClearINTPendingBit(ADC_TypeDef *ADCx, uint32_t ADC_IT)
{
    /* Check the parameters */
    assert_param(IS_ADC_PERIPH(ADCx));
    assert_param(IS_ADC_IT(ADC_IT));

    ADCx->INTCR |= (ADC_IT << 8);

    return;
}

/**
  * @brief  Checks whether the specified ADC interrupt status flag is set or not.
  * @param  ADCx: selected ADC peripheral.
  * @param  ADC_INT_FLAG: specifies the interrupt status flag to check.
  *   This parameter can be one of the following values:
  *     @arg ADC_INT_ONE_SHOT_DONE: ADC one convert end interrupt flag
  *     @arg ADC_INT_FIFO_FULL: ADC FIFO full interrupt flag
  *     @arg ADC_INT_FIFO_RD_REQ: ADC read FIFO request interrupt flag
  *     @arg ADC_INT_FIFO_RD_ERR: ADC read FIFO error interrupt flag
  *     @arg ADC_INT_FIFO_TH: ADC codec interrupt flag
  * @retval The new state of ADC_CMP_FLAG (SET or RESET).
  */
__STATIC_INLINE FlagStatus ADC_GetIntFlagStatus(ADC_TypeDef *ADCx, uint32_t ADC_INT_FLAG)
{
    FlagStatus bitstatus = RESET;

    /* Check the parameters */
    assert_param(IS_ADC_PERIPH(ADCx));
    assert_param(IS_ADC_IT(ADC_INT_FLAG));

    if ((ADCx->INTCR & (ADC_INT_FLAG << 16)) != 0)
    {
        bitstatus = SET;
    }

    return bitstatus;
}

/**
  * @brief  Set adc schedule table.
  * @param  ADCx: selected ADC peripheral.
  * @param  channelMap: ADC channel map.
  * @param  NewState: new state of the ADC peripheral.
  *   This parameter can be: ENABLE or DISABLE.
  * @return none.
  */
__STATIC_INLINE void ADC_SchTableSet(ADC_TypeDef *ADCx, uint16_t channelMap,
                                     FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_ADC_PERIPH(ADCx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == ENABLE)
    {
        ADCx->SCHCR |= channelMap;
    }
    else
    {
        ADCx->SCHCR &= (~channelMap);
    }

    return;
}

/**
  * @brief  Get one data from ADC fifo.
  * @param  ADCx: selected ADC peripheral.
  * @return adc fifo data.
  */
__STATIC_INLINE uint16_t ADC_ReadFifoData(ADC_TypeDef *ADCx)
{
    /* Check the parameters */
    assert_param(IS_ADC_PERIPH(ADCx));

    return (uint16_t)((ADCx->FIFO) & 0xfff);
}

/**
  * @brief  Get ADC fifo number.
  * @param  ADCx: selected ADC peripheral.
  * @return current data number in adc fifo.
  */
__STATIC_INLINE uint8_t ADC_GetFifoLen(ADC_TypeDef *ADCx)
{
    /* Check the parameters */
    assert_param(IS_ADC_PERIPH(ADCx));

    return ((uint8_t)(((ADCx->SCHCR) >> 16) & 0x3F));
}

/**
  * @brief  Get all adc interrupt status.
  * @param  ADCx: selected ADC peripheral.
  * @return uint8_t interrupt status.
  */
__STATIC_INLINE uint8_t ADC_GetIntStatus(ADC_TypeDef *ADCx)
{
    /* Check the parameters */
    assert_param(IS_ADC_PERIPH(ADCx));

    return ((uint8_t)(((ADCx->INTCR) & (0x1f << 16)) >> 16));
}

/**
  * @brief  Clear ADC fifo.
  * @param  ADCx: selected ADC peripheral.
  * @return none.
  */
__STATIC_INLINE void ADC_ClearFifo(ADC_TypeDef *ADCx)
{
    /* Check the parameters */
    assert_param(IS_ADC_PERIPH(ADCx));

    ADCx->CR |= BIT26;

    return;
}

/** @cond private
  * @defgroup ADC_Delay_Function ADC Delay.
  * @{
  */
__STATIC_INLINE void ADC_delayUS(uint32_t t);

void ADC_delayUS(uint32_t t)
{
    platform_delay_us(t);
}

/**
  * @}
  * @endcond
  */

/**
  * @brief  Enable or disable ADC Power always on.
  * @param  ADCx: selected ADC peripheral.
  * @param  NewState: new state of the ADC Power on mode.
  *   This parameter can be: ENABLE or DISABLE. If enable, ADC power will always on.
  * @return none.
  */
__STATIC_INLINE void ADC_PowerAlwaysOnCmd(ADC_TypeDef *ADCx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_ADC_PERIPH(ADCx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == ENABLE)
    {
        ADCx->PWRDLY |= BIT19;
    }
    else
    {
        ADCx->PWRDLY &= ~(BIT19);
    }
}

/** @} */ /* End of group ADC_Exported_Functions */
/** @} */ /* End of group ADC */


#ifdef __cplusplus
}
#endif

#endif /* _RTL876X_ADC_H_ */


/******************* (C) COPYRIGHT 2015 Realtek Semiconductor *****END OF FILE****/



