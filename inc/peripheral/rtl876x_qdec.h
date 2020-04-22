/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_qdec.h
* @brief     header file of uart driver.
* @details
* @author    howie_wang
* @date      2016-05-10
* @version   v1.0
* *********************************************************************************************************
*/


#ifndef _RTL876X_QDECODER_H_
#define _RTL876X_QDECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"
#include "rtl876x_bitfields.h"

/** @addtogroup QDEC QDEC
  * @brief Qdecoder driver module
  * @{
  */

/*============================================================================*
 *                         Types
 *============================================================================*/


/** @defgroup QDEC_Exported_Types QDEC Exported Types
  * @{
  */

/**
 * @brief Qdecoder initialize parameters
 *
 * Qdecoder initialize parameters
 */
typedef struct
{
    uint16_t scanClockDiv;
    uint16_t debounceClockDiv;
    uint8_t axisConfigX;                /*!< Specifies the axis X function.
                                                  This parameter can be a value of ENABLE or DISABLE */
    uint8_t axisConfigY;                /*!< Specifies the axis Y function.
                                                  This parameter can be a value of ENABLE or DISABLE */
    uint8_t axisConfigZ;                /*!< Specifies the axis Z function.
                                                  This parameter can be a value of ENABLE or DISABLE */
    uint8_t autoLoadInitPhase;          /*!< Specifies Auto-load Initphase function .
                                                  This parameter can be a value of ENABLE or DISABLE */
    uint16_t counterScaleX;             /*!< Specifies the axis X conter scale.
                                                  This parameter can be a value of @ref QDEC_Axis_counter_Scale */
    uint16_t debounceEnableX;           /*!< Specifies the axis X debounce.
                                                  This parameter can be a value of @ref Qdec_Debounce */
    uint16_t debounceTimeX;             /*!< Specifies the axis X debounce time. */
    uint16_t initPhaseX;                /*!< Specifies the axis X function.
                                                  This parameter can be a value of @ref Qdec_init_phase */
    uint16_t counterScaleY;             /*!< Specifies the axis Y conter scale.
                                                  This parameter can be a value of @ref QDEC_Axis_counter_Scale */
    uint16_t debounceEnableY;           /*!< Specifies the axis Y debounce.
                                                  This parameter can be a value of @ref Qdec_Debounce */
    uint16_t debounceTimeY;             /*!< Specifies the axis Y debounce time. */
    uint16_t initPhaseY;                /*!< Specifies the axis Y function.
                                                  This parameter can be a value of @ref Qdec_init_phase */
    uint16_t counterScaleZ;             /*!< Specifies the axis Z conter scale.
                                                  This parameter can be a value of @ref QDEC_Axis_counter_Scale */
    uint16_t debounceEnableZ;           /*!< Specifies the axis Z debounce.
                                                  This parameter can be a value of @ref Qdec_Debounce */
    uint16_t debounceTimeZ;             /*!< Specifies the axis Z debounce time. */
    uint16_t initPhaseZ;                /*!< Specifies the axis Z function.
                                                  This parameter can be a value of @ref Qdec_init_phase */
} QDEC_InitTypeDef;

/** End of group QDEC_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/


/** @defgroup QDEC_Exported_Constants QDEC Exported Constants
  * @{
  */

#define IS_QDEC_PERIPH(PERIPH) ((PERIPH) == QDEC)

/** @defgroup QDEC_Interrupts_Definition QDEC Interrupts Definition
  * @{
  */

#define QDEC_X_INT_NEW_DATA                                   BIT(0)//get new data and state change
#define QDEC_X_INT_ILLEAGE                                    BIT(1)//illeage
#define QDEC_Y_INT_NEW_DATA                                   BIT(2)//get new data and state change
#define QDEC_Y_INT_ILLEAGE                                    BIT(3)//illeage
#define QDEC_Z_INT_NEW_DATA                                   BIT(4)//get new data and state change
#define QDEC_Z_INT_ILLEAGE                                    BIT(5)//illeage

#define IS_QDEC_INT_CONFIG(CONFIG) (((CONFIG) == QDEC_X_INT_NEW_DATA) || ((CONFIG) == QDEC_X_INT_ILLEAGE)\
                                    || ((CONFIG) == QDEC_Y_INT_NEW_DATA) || ((CONFIG) == QDEC_Y_INT_ILLEAGE)\
                                    || ((CONFIG) == QDEC_Z_INT_NEW_DATA) || ((CONFIG) == QDEC_Z_INT_ILLEAGE))
/** End of group QDEC_Interrupts_Definition
  * @}
  */

/** @defgroup QDEC_Interrupts_Mask QDEC Interrupts Mask
  * @{
  */

#define QDEC_X_CT_INT_MASK                                   BIT(0)//get new data and state change
#define QDEC_X_ILLEAGE_INT_MASK                              BIT(4)//illeage
#define QDEC_Y_CT_INT_MASK                                   BIT(1)//get new data and state change
#define QDEC_Y_ILLEAGE_INT_MASK                              BIT(5)//illeage
#define QDEC_Z_CT_INT_MASK                                   BIT(2)//get new data and state change
#define QDEC_Z_ILLEAGE_INT_MASK                              BIT(6)//illeage

#define IS_QDEC_INT_MASK_CONFIG(CONFIG) (((CONFIG) == QDEC_X_CT_INT_MASK) || ((CONFIG) == QDEC_X_ILLEAGE_INT_MASK)\
                                         || ((CONFIG) == QDEC_Y_CT_INT_MASK) || ((CONFIG) == QDEC_Y_ILLEAGE_INT_MASK)\
                                         || ((CONFIG) == QDEC_Z_CT_INT_MASK) || ((CONFIG) == QDEC_Z_ILLEAGE_INT_MASK))
/** End of group QDEC_Interrupts_Mask
  * @}
  */

/** @defgroup QDEC_Axis_counter_Scale Qdec Axis Counter
  * @{
  */

#define CounterScale_2_Phase                      true
#define CounterScale_1_Phase                      false

/** End of group QDEC_Axis_counter_Scale
  * @}
  */

/** @defgroup Qdec_Debounce Qdec Debounce
  * @{
  */

#define Debounce_Enable                        true
#define Debounce_Disable                       false

/** End of group Qdec_Debounce
  * @}
  */

/** @defgroup Qdec_manual_phase Qdec manual phase
  * @{
  */

#define manualPhaseEnable                        true
#define manualPhaseDisable                       false

/** End of group Qdec_manual_phase
  * @}
  */

/** @defgroup Qdec_init_phase Qdec Init phase
  * @{
  */

#define phaseMode0                        0       //phase 00
#define phaseMode1                        1       //phase 01
#define phaseMode2                        2       //phase 10
#define phaseMode3                        3       //phase 11

/** End of group Qdec_init_phase
  * @}
  */

/** @defgroup QDEC_Clr_Flag Qdec clr Flag
  * @{
  */
#define QDEC_CLR_ILLEGAL_CT_X                            ((uint32_t)(1 << 20))
#define QDEC_CLR_ILLEGAL_CT_Y                            ((uint32_t)(1 << 21))
#define QDEC_CLR_ILLEGAL_CT_Z                            ((uint32_t)(1 << 22))

#define QDEC_CLR_ACC_CT_X                                ((uint32_t)(1 << 16))
#define QDEC_CLR_ACC_CT_Y                                ((uint32_t)(1 << 17))
#define QDEC_CLR_ACC_CT_Z                                ((uint32_t)(1 << 18))

#define QDEC_CLR_ILLEGAL_INT_X                           ((uint32_t)(1 << 12))
#define QDEC_CLR_ILLEGAL_INT_Y                           ((uint32_t)(1 << 13))
#define QDEC_CLR_ILLEGAL_INT_Z                           ((uint32_t)(1 << 14))

#define QDEC_CLR_UNDERFLOW_X                             ((uint32_t)(1 << 8))
#define QDEC_CLR_UNDERFLOW_Y                             ((uint32_t)(1 << 9))
#define QDEC_CLR_UNDERFLOW_Z                             ((uint32_t)(1 << 10))

#define QDEC_CLR_OVERFLOW_X                              ((uint32_t)(1 << 4))
#define QDEC_CLR_OVERFLOW_Y                              ((uint32_t)(1 << 5))
#define QDEC_CLR_OVERFLOW_Z                              ((uint32_t)(1 << 6))

#define QDEC_CLR_NEW_CT_X                                ((uint32_t)(1 << 0))
#define QDEC_CLR_NEW_CT_Y                                ((uint32_t)(1 << 1))
#define QDEC_CLR_NEW_CT_Z                                ((uint32_t)(1 << 2))

#define IS_QDEC_INT_CLR_CONFIG(CONFIG) (((CONFIG) == QDEC_CLR_ACC_CT_X) || ((CONFIG) == QDEC_CLR_ACC_CT_Y)\
                                        || ((CONFIG) == QDEC_CLR_ACC_CT_Z) || ((CONFIG) == QDEC_CLR_ILLEGAL_INT_Y)\
                                        || ((CONFIG) == QDEC_CLR_ILLEGAL_INT_Z) || ((CONFIG) == QDEC_CLR_UNDERFLOW_X)\
                                        || ((CONFIG) == QDEC_CLR_UNDERFLOW_Y) || ((CONFIG) == QDEC_CLR_UNDERFLOW_Z)\
                                        || ((CONFIG) == QDEC_CLR_OVERFLOW_X) || ((CONFIG) == QDEC_CLR_OVERFLOW_Y)\
                                        || ((CONFIG) == QDEC_CLR_OVERFLOW_Z) || ((CONFIG) == QDEC_CLR_NEW_CT_X)\
                                        || ((CONFIG) == QDEC_CLR_NEW_CT_Y) || ((CONFIG) == QDEC_CLR_NEW_CT_Z))
/** End of group QDEC_Clr_Flag
  * @}
  */

/** @defgroup QDEC_Flag QDEC Flag
  * @{
  */

#define QDEC_FLAG_ILLEGAL_STATUS_X                            ((uint32_t)(1 << 12))
#define QDEC_FLAG_ILLEGAL_STATUS_Y                            ((uint32_t)(1 << 13))
#define QDEC_FLAG_ILLEGAL_STATUS_Z                            ((uint32_t)(1 << 14))
#define QDEC_FLAG_NEW_CT_STATUS_X                             ((uint32_t)(1 << 0))
#define QDEC_FLAG_NEW_CT_STATUS_Y                             ((uint32_t)(1 << 1))
#define QDEC_FLAG_NEW_CT_STATUS_Z                             ((uint32_t)(1 << 2))
#define QDEC_FLAG_OVERFLOW_X                                  ((uint32_t)(1 << 3))
#define QDEC_FLAG_OVERFLOW_Y                                  ((uint32_t)(1 << 4))
#define QDEC_FLAG_OVERFLOW_Z                                  ((uint32_t)(1 << 5))
#define QDEC_FLAG_UNDERFLOW_X                                 ((uint32_t)(1 << 6))
#define QDEC_FLAG_UNDERFLOW_Y                                 ((uint32_t)(1 << 7))
#define QDEC_FLAG_UNDERFLOW_Z                                 ((uint32_t)(1 << 8))
#define QDEC_FLAG_AUTO_STATUS_X                               ((uint32_t)(1 << 9))
#define QDEC_FLAG_AUTO_STATUS_Y                               ((uint32_t)(1 << 10))
#define QDEC_FLAG_AUTO_STATUS_Z                               ((uint32_t)(1 << 11))

#define IS_QDEC_CLR_INT_STATUS(INT) (((INT) == QDEC_FLAG_ILLEGAL_STATUS_X) || ((INT) == QDEC_FLAG_ILLEGAL_STATUS_Y)\
                                     || ((INT) == QDEC_FLAG_ILLEGAL_STATUS_Z) || ((INT) == QDEC_FLAG_NEW_CT_STATUS_X)\
                                     || ((INT) == QDEC_FLAG_NEW_CT_STATUS_Y) || ((INT) == QDEC_FLAG_NEW_CT_STATUS_Z)\
                                     || ((INT) == QDEC_FLAG_OVERFLOW_X) || ((INT) == QDEC_FLAG_OVERFLOW_Y)\
                                     || ((INT) == QDEC_FLAG_OVERFLOW_Z) || ((INT) == QDEC_FLAG_UNDERFLOW_X)\
                                     || ((INT) == QDEC_FLAG_UNDERFLOW_Y) || ((INT) == QDEC_FLAG_UNDERFLOW_Z)\
                                     || ((INT) == QDEC_FLAG_AUTO_STATUS_X) || ((INT) == QDEC_FLAG_AUTO_STATUS_Y)\
                                     || ((INT) == QDEC_FLAG_AUTO_STATUS_Z))


/** End of group QDEC_Flag
  * @}
  */

/** @defgroup QDEC_Axis QDEC Axis
  * @{
  */

#define QDEC_AXIS_X                                     ((uint32_t)(1 << 0))
#define QDEC_AXIS_Y                                     ((uint32_t)(1 << 2))
#define QDEC_AXIS_Z                                     ((uint32_t)(1 << 3))

/** End of group QDEC_Axis
  * @}
  */

/** @defgroup QDEC_Axis_Direction QDEC Axis Direction
  * @{
  */

#define QDEC_AXIS_DIR_UP                                 ((uint16_t)0x01)
#define QDEC_AXIS_DIR_DOWN                               ((uint16_t)0x00)

#define IS_QDEC_AXIS_DIR(QDEC_AXIS)     ((QDEC_AXIS == QDEC_AXIS_DIR_UP) || (QDEC_AXIS == QDEC_AXIS_DIR_DOWN))

/** End of group QDEC_Axis_Direction
  * @}
  */


/** End of group QDEC_Exported_Constants
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup QDEC_Exported_Functions QDEC Exported Functions
  * @{
  */
/**
  * @brief Initializes the Qdecoder peripheral according to the specified
  *   parameters in the QDEC_InitStruct
  * @param  QDECx: selected Qdecoder peripheral.
  * @param  QDEC_InitStruct: pointer to a QDEC_InitStruct structure that
  *     contains the configuration information for the specified Qdecoder peripheral
  * @retval None
  */
extern void QDEC_Init(QDEC_TypeDef *QDECx, QDEC_InitTypeDef *QDEC_InitStruct);

/**
  * @brief  Deinitializes the Qdecoder peripheral registers to their default reset values(turn off Qdecoder clock).
  * @param  QDECx: selected Qdecoder peripheral.
  * @retval None
  */
extern void QDEC_DeInit(QDEC_TypeDef *QDECx);

/**
  * @brief  Fills each QDEC_InitStruct member with its default value.
  * @param  QDEC_InitStruct: pointer to an QDEC_InitStruct structure which will be initialized.
  * @retval None
  */
extern void QDEC_StructInit(QDEC_InitTypeDef *QDEC_InitStruct);

/**
  * @brief  Enables or disables the specified Qdecoder interrupts.
  * @param  QDECx: selected Qdecoder peripheral.
  * @param  QDEC_IT: specifies the QDECODER interrupts sources to be enabled or disabled.
  *   This parameter can be one of the following values:
  *     @arg QDEC_X_INT_NEW_DATA:
  *     @arg QDEC_X_INT_ILLEAGE:
  *     @arg QDEC_Y_INT_NEW_DATA:
  *     @arg QDEC_Y_INT_ILLEAGE:
  *     @arg QDEC_Z_INT_NEW_DATA:
  *     @arg QDEC_Z_INT_ILLEAGE:
  * @param  newState: new state of the specified QDECODER interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
extern void QDEC_INTConfig(QDEC_TypeDef *QDECx, uint32_t QDEC_IT, FunctionalState newState);

/**
  * @brief  Checks whether the specified Qdecoder flag is set or not.
  * @param  QDECx: selected Qdecoder peripheral.
  * @param  QDEC_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg QDEC_FLAG_OVERFLOW_X:
  *     @arg QDEC_FLAG_OVERFLOW_Y:
  *     @arg QDEC_FLAG_OVERFLOW_Z:
  *     @arg QDEC_FLAG_UNDERFLOW_X:
  *     @arg QDEC_FLAG_UNDERFLOW_Y:
  *     @arg QDEC_FLAG_UNDERFLOW_Z:
  *     @arg QDEC_FLAG_AUTO_STATUS_X:
  *     @arg QDEC_FLAG_AUTO_STATUS_Y:
  *     @arg QDEC_FLAG_AUTO_STATUS_Z:
  *     @arg QDEC_FLAG_ILLEGAL_STATUS_X:
  *     @arg QDEC_FLAG_ILLEGAL_STATUS_Y:
  *     @arg QDEC_FLAG_ILLEGAL_STATUS_Z:
  *     @arg QDEC_FLAG_NEW_STATUS_X:
  *     @arg QDEC_FLAG_NEW_STATUS_Y:
  *     @arg QDEC_FLAG_NEW_STATUS_Z:
  * @retval The new state of QDEC_FLAG (SET or RESET).
  */
extern FlagStatus QDEC_GetFlagState(QDEC_TypeDef *QDECx, uint32_t QDEC_FLAG);

/**
  * @brief  Enables or disables mask the specified Qdecoder axis interrupts.
  * @param  QDECx: selected Qdecoder peripheral.
  * @param  QDEC_AXIS: specifies the Qdecoder axis.
  *   This parameter can be one or logical OR of the following values:
  *     @arg QDEC_X_CT_INT_MASK: The qdecoder X axis.
  *     @arg QDEC_X_ILLEAGE_INT_MASK: The qdecoder Y axis.
  *     @arg QDEC_Y_CT_INT_MASK: The qdecoder Z axis.
  *     @arg QDEC_Y_ILLEAGE_INT_MASK: The qdecoder X axis.
  *     @arg QDEC_Z_CNT_INT_MASK: The qdecoder Y axis.
  *     @arg QDEC_AXIS_ILLEAGE_Z: The qdecoder Z axis.
  * @param  newState: new state of the specified Qdecoder interrupts mask.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
extern void QDEC_INTMask(QDEC_TypeDef *QDECx, uint32_t QDEC_AXIS, FunctionalState newState);

/**
  * @brief  Get Qdecoder Axis(x/y/z) direction.
  * @param  QDECx: selected Qdecoder peripheral.
  * @param  QDEC_AXIS: specifies the Qdecoder axis.
  *   This parameter can be one of the following values:
  *     @arg QDEC_AXIS_X: The qdecoder X axis.
  *     @arg QDEC_AXIS_Y: The qdecoder Y axis.
  *     @arg QDEC_AXIS_Z: The qdecoder Z axis.
  * @param  newState
  *   This parameter can be one of the following values:
  *     @arg ENABLE: Pause.
  *     @arg DISABLE: Resume.
  * @retval The count of the axis.
  */
extern void QDEC_Cmd(QDEC_TypeDef *QDECx, uint32_t QDEC_AXIS,
                     FunctionalState newState);

/**
  * @brief  Clears Qdecoder pending flags.
  * @param  QDECx: selected Qdecoder peripheral.
  * @param  QDEC_FLAG: specifies the flag to clear.
  *   This parameter can be one of the following values:
  *     @arg QDEC_CLR_OVERFLOW_X:
  *     @arg QDEC_CLR_OVERFLOW_Y:
  *     @arg QDEC_CLR_OVERFLOW_Z:
  *     @arg QDEC_CLR_ILLEGAL_INT_X:
  *     @arg QDEC_CLR_ILLEGAL_INT_Y:
  *     @arg QDEC_CLR_ILLEGAL_INT_Z:
  *     @arg QDEC_CLR_UNDERFLOW_X:
  *     @arg QDEC_CLR_UNDERFLOW_Y:
  *     @arg QDEC_CLR_UNDERFLOW_Z:
  *     @arg QDEC_CLR_NEW_CT_X:
  *     @arg QDEC_CLR_NEW_CT_Y:
  *     @arg QDEC_CLR_NEW_CT_Z:
  * @retval None.
  */
__STATIC_INLINE void QDEC_ClearINTPendingBit(QDEC_TypeDef *QDECx, uint32_t QDEC_CLR_INT)
{
    /* Check the parameters */
    assert_param(IS_QDEC_PERIPH(QDECx));
    assert_param(IS_QDEC_CLR_INT_STATUS(QDEC_CLR_INT));

    QDECx->INT_CLR |= QDEC_CLR_INT;

    return;
}

/**
  * @brief  Get Qdecoder Axis(x/y/z) direction.
  * @param  QDECx: selected Qdecoder peripheral.
  * @param  QDEC_AXIS: specifies the Qdecoder axis.
  *   This parameter can be one of the following values:
  *     @arg QDEC_AXIS_X: The qdecoder X axis.
  *     @arg QDEC_AXIS_Y: The qdecoder Y axis.
  *     @arg QDEC_AXIS_Z: The qdecoder Z axis.
  * @retval The direction of the axis.
  *   This parameter can be one of the following values:
  *     @arg QDEC_AXIS_DIR_UP: The axis is rolling up.
  *     @arg QDEC_AXIS_DIR_DOWN: The axis is rolling down.
  */
__STATIC_INLINE uint16_t QDEC_GetAxisDirection(QDEC_TypeDef *QDECx, uint32_t QDEC_AXIS)
{
    /* Check the parameters */
    assert_param(IS_QDEC_PERIPH(QDECx));
    assert_param(IS_QDEC_AXIS_DIR(QDEC_AXIS));

    return ((*((volatile uint32_t *)(&QDECx->REG_SR_X) + QDEC_AXIS / 2) & (1 << 16)) == BIT(16));
}

/**
  * @brief  Get Qdecoder Axis(x/y/z) direction.
  * @param  QDECx: selected Qdecoder peripheral.
  * @param  QDEC_AXIS: specifies the Qdecoder axis.
  *   This parameter can be one of the following values:
  *     @arg QDEC_AXIS_X: The qdecoder X axis.
  *     @arg QDEC_AXIS_Y: The qdecoder Y axis.
  *     @arg QDEC_AXIS_Z: The qdecoder Z axis.
  * @retval The count of the axis.
  */
__STATIC_INLINE uint16_t QDEC_GetAxisCount(QDEC_TypeDef *QDECx, uint32_t QDEC_AXIS)
{
    /* Check the parameters */
    assert_param(IS_QDEC_PERIPH(QDECx));
    assert_param(IS_QDEC_AXIS_DIR(QDEC_AXIS));

    return ((uint16_t)(*((volatile uint32_t *)(&QDECx->REG_SR_X) + QDEC_AXIS / 2)));
}

/**
  * @brief  Get Qdecoder Axis(x/y/z) direction.
  * @param  QDECx: selected Qdecoder peripheral.
  * @param  QDEC_AXIS: specifies the Qdecoder axis.
  *   This parameter can be one of the following values:
  *     @arg QDEC_AXIS_X: The qdecoder X axis.
  *     @arg QDEC_AXIS_Y: The qdecoder Y axis.
  *     @arg QDEC_AXIS_Z: The qdecoder Z axis.
  * @param  newState
  *   This parameter can be one of the following values:
  *     @arg ENABLE: Pause.
  *     @arg DISABLE: Resume.
  * @retval The count of the axis.
  */
__STATIC_INLINE void QDEC_CounterPauseCmd(QDEC_TypeDef *QDECx, uint32_t QDEC_AXIS,
                                          FunctionalState newState)
{
    /* Check the parameters */
    assert_param(IS_QDEC_PERIPH(QDECx));
    assert_param(IS_QDEC_AXIS_DIR(QDEC_AXIS));

    if (newState == ENABLE)
    {
        *((volatile uint32_t *)(&QDECx->REG_CR_X) + QDEC_AXIS / 2) |= BIT3;
    }
    else
    {
        *((volatile uint32_t *)(&QDECx->REG_CR_X) + QDEC_AXIS / 2) &= ~BIT3;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* _RTL876X_QDECODER_H_ */

/** @} */ /* End of group QDEC_Exported_Functions */
/** @} */ /* End of group QDEC */

/******************* (C) COPYRIGHT 2016 Realtek Semiconductor *****END OF FILE****/



