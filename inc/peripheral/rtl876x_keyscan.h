/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_keyscan.h
* @brief     header file of keyscan driver.
* @details
* @author    tifnan_ge
* @date      2015-04-29
* @version   v1.0
* *********************************************************************************************************
*/


#ifndef _RTL876X_KEYSCAN_H_
#define _RTL876X_KEYSCAN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"
#include "rtl876x_bitfields.h"


/** @addtogroup KeyScan KeyScan
  * @brief Keyscan driver module
  * @{
  */

/*============================================================================*
 *                         Types
 *============================================================================*/

/** @defgroup KeyScan_Exported_Types KeyScan Exported Types
  * @{
  */

/**
 * @brief keyscan initialize parameters
 *
 * keyscan initialize parameters
 */
typedef struct
{
    uint16_t        rowSize;             /*!< Specifies Keyscan Row Size.
                                                    This parameter can be a value of @ref Keyscan_Row_Number */
    uint16_t        colSize;            /*!< Specifies Keyscan Column Size.
                                                    This parameter can be a value of @ref Keyscan_Column_Number */

    uint32_t        detectPeriod;       //ms
    uint16_t        timeout;            //sw interrupt timeout  (s)
    uint16_t        scanInterval;       /*!< Specifies Keyscan release time. ReleaseTime = */
    uint32_t        debounceEn;         //debounce enable or disable
    uint32_t        scantimerEn;
    uint32_t        detecttimerEn;
    uint16_t        debounceTime;       /*!< useful when KeyScan_Debounce_Enable.
                                                    debounce time = 31.25us * debounceTime.*/

    uint32_t        detectMode;         /*!< Specifies Key Detect mode.
                                                    This parameter can be a value of @ref Keyscan_Press_Detect_Mode */
    uint32_t        fifoOvrCtrl;        /*!< Specifies Keyscan fifo over flow control.
                                                    This parameter can be a value of @ref Keyscan_Fifo_Overflow_Control */

    uint16_t        maxScanData;        //max scan data allowable in each scan
    uint32_t        scanmode;           /*!< Specifies Keyscan mode.
                                                    This parameter can be a value of @ref Keyscan_scan_mode */

    uint16_t
    clockdiv;           /*!< Specifies Keyscan clock divider.system clock/(SCAN_DIV+1)=scan clock */
    uint8_t
    delayclk;           /*!< Specifies Keyscan delay clock divider.delay clock= scan clock/(DELAY_DIV+1)*/
    uint16_t
    fifotriggerlevel;   /*!< Specifies Keyscan fifo threshold to trigger interrupt KEYSCAN_INT_THRESHOLD.*/
    uint8_t         debouncecnt;        /*!< Specifies Keyscan Debounce time. DebounceTime = */
    uint8_t         releasecnt;         /*!< Specifies Keyscan release time. ReleaseTime = */
    uint8_t         keylimit;           /*!< Specifies max scan data allowable in each scan */
} KEYSCAN_InitTypeDef;

/** End of group KeyScan_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/


/** @defgroup KeyScan_Exported_Constants KeyScan Exported Constants
  * @{
  */

#define IS_KeyScan_PERIPH(PERIPH) ((PERIPH) == KEYSCAN)

/** @defgroup Keyscan_Row_Number Keyscan Row Number
  * @{
  */

#define IS_KEYSCAN_ROW_NUM(ROW) ((ROW) <= 8)

/** End of group Keyscan_Row_Number
  * @}
  */

/** @defgroup Keyscan_Column_Number Keyscan Column Number
  * @{
  */

#define IS_KEYSCAN_COL_NUM(COL) ((COL) <= 20)

/** End of group Keyscan_Column_Number
  * @}
  */

/** @defgroup Keyscan_DebounceTime Keyscan DebounceTime
  * @{
  */

#define IS_KEYSCAN_MAX_SCAN_DATA(DATA_NUM) ((DATA_NUM) <= 26)    //0 means no limit

/** End of group Keyscan_DebounceTime
  * @}
  */


/** @defgroup Keyscan_scan_mode Keyscan scan mode
  * @{
  */

#define KeyScan_Manual_Scan_Mode        ((uint32_t)(0x0 << 30))
#define KeyScan_Auto_Scan_Mode          ((uint32_t)(0x1 << 30))

#define IS_KEYSCAN_DETECT_MODE(MODE)    (((MODE) == KeyScan_Detect_Mode_Edge) || ((MODE) == KeyScan_Detect_Mode_Level))

/** End of group Keyscan_scan_mode
  * @}
  */

/** @defgroup Keyscan_Fifo_Overflow_Control Keyscan Fifo Overflow Control
  * @{
  */

#define KeyScan_FIFO_OVR_CTRL_DIS_ALL   ((uint32_t)(0x0 << 28))   //discard all the new scan data when FIFO is full
#define KeyScan_FIFO_OVR_CTRL_DIS_LAST  ((uint32_t)(0x1 << 28))   //discard the last scan data when FIFO is full

#define IS_KEYSCAN_FIFO_OVR_CTRL(CTRL)  (((CTRL) == KeyScan_FIFO_OVR_CTRL_DIS_ALL) || ((CTRL) == KeyScan_FIFO_OVR_CTRL_DIS_LAST))

/** End of group Keyscan_Fifo_Overflow_Control
  * @}
  */


/** @defgroup Keyscan_Debounce_Config Keyscan Debounce Config
  * @{
  */

#define KeyScan_Debounce_Enable              ((uint32_t)0x1 << 31)
#define KeyScan_Debounce_Disable             ((uint32_t)0x0 << 31)
#define IS_KEYSCAN_DEBOUNCE_EN(EN) (((EN) == KeyScan_Debounce_Enable) || ((EN) == KeyScan_Debounce_Disable))    //0 means no limit

/** End of group Keyscan_Debounce_Config
  * @}
  */

/** @defgroup Keyscanv_scan_interval_en Keyscan scan interval enable
  * @{
  */

#define KeyScan_ScanInterval_Enable              ((uint32_t)(0x1 << 30))
#define KeyScan_ScanInterval_Disable             ((uint32_t)(0x0 << 30))
#define IS_KEYSCAN_SCANINTERVAL_EN(EN) (((EN) == KeyScan_ScanInterval_Enable) || ((EN) == KeyScan_ScanInterval_Disable))    //0 means no limit

/** End of group Keyscanv_scan_interval_en
  * @}
  */

/** @defgroup Keyscan_release_detect_timer_en Keyscan release detect timer en
  * @{
  */

#define KeyScan_Release_Detect_Enable              ((uint32_t)(0x1 << 29))
#define KeyScan_Release_Detect_Disable             ((uint32_t)(0x0 << 29))
#define IS_KEYSCAN_RELEASE_DETECT_EN(EN) (((EN) == KeyScan_Release_Detect_Enable) || ((EN) == KeyScan_Release_Detect_Disable))    //0 means no limit

/** End of group Keyscan_release_detect_timer_en
  * @}
  */

/** @defgroup Keyscan_Press_Detect_Mode Keyscan Press Detect Mode
  * @{
  */

#define KeyScan_Detect_Mode_Edge        ((uint32_t)(0x0 << 29))
#define KeyScan_Detect_Mode_Level       ((uint32_t)(0x1 << 29))

#define IS_KEYSCAN_DETECT_MODE(MODE)    (((MODE) == KeyScan_Detect_Mode_Edge) || ((MODE) == KeyScan_Detect_Mode_Level))

/** End of group Keyscan_Press_Detect_Mode
  * @}
  */


/** @defgroup Keyscan_Interrupt_Definition Keyscan Interrupt Definition
  * @{
  */

#define KEYSCAN_INT_THRESHOLD                    ((uint16_t)(0x1 << 4))
#define KEYSCAN_INT_OVER_READ                    ((uint16_t)(0x1 << 3))
#define KEYSCAN_INT_SCAN_END                     ((uint16_t)(0x1 << 2))
#define KEYSCAN_INT_FIFO_NOT_EMPTY               ((uint16_t)(0x1 << 1))
#define KEYSCAN_INT_ALL_RELEASE                  ((uint16_t)(0x1 << 0))

#define IS_KEYSCAN_CONFIG_IT(IT) ((((IT) & (uint32_t)0xFFF8) == 0x00) && ((IT) != 0x00))

/** End of group Keyscan_Interrupt_Definition
  * @}
  */

/**
  * @defgroup  Keyscan_Flags Keyscan Flags
  * @{
  */
#define KEYSCAN_FLAG_FIFOLIMIT                       ((uint32_t)(0x1 << 20))
#define KEYSCAN_INT_FLAG_THRESHOLD                   ((uint32_t)(0x1 << 19))
#define KEYSCAN_INT_FLAG_OVER_READ                   ((uint32_t)(0x1 << 18))
#define KEYSCAN_INT_FLAG_SCAN_END                    ((uint32_t)(0x1 << 17))
#define KEYSCAN_INT_FLAG_FIFO_NOT_EMPTY              ((uint32_t)(0x1 << 16))
#define KEYSCAN_INT_FLAG_ALL_RELEASE                 ((uint32_t)(0x1 << 15))
#define KEYSCAN_FLAG_DATAFILTER                      ((uint32_t)(0x1 << 3))
#define KEYSCAN_FLAG_OVR                             ((uint32_t)(0x1 << 2))
#define KEYSCAN_FLAG_FULL                            ((uint32_t)(0x1 << 1))
#define KEYSCAN_FLAG_EMPTY                           ((uint32_t)(0x1 << 0))

#define IS_KEYSCAN_FLAG(FLAG)       ((((FLAG) & (uint32_t)0x01FF) == 0x00) && ((FLAG) != (uint32_t)0x00))
#define IS_KEYSCAN_CLEAR_FLAG(FLAG) ((((FLAG) & (uint32_t)0x00C0) == 0x00) && ((FLAG) != (uint32_t)0x00))

/** End of group Keyscan_Flags
  * @}
  */


/** @cond private
  * @defgroup Keyscan_FIFO_AVALIABLE_MASK
  * @{
  */

#define STATUS_FIFO_DATA_NUM_MASK           ((uint32_t)(0x3F << 4))

/**
  * @}
  * @endcond
  */


/** End of group KeyScan_Exported_Constants
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup Keyscan_Exported_functions Keyscan Exported Functions
 * @{
 */

/**
  * @brief Initializes the KeyScan peripheral according to the specified
  *   parameters in the KeyScan_InitStruct
  * @param  KeyScan: selected KeyScan peripheral.
  * @param  KeyScan_InitStruct: pointer to a KEYSCAN_InitTypeDef structure that
  *   contains the configuration information for the specified KeyScan peripheral
  * @retval None
  */
extern void KeyScan_Init(KEYSCAN_TypeDef *KeyScan, KEYSCAN_InitTypeDef *KeyScan_InitStruct);

/**
  * @brief  Deinitializes the Keyscan peripheral registers to their default reset values(turn off keyscan clock).
  * @param  KeyScan: selected KeyScan peripheral.
  * @retval None
  */
extern void KeyScan_DeInit(KEYSCAN_TypeDef *KeyScan);

/**
  * @brief  Fills each Keyscan_InitStruct member with its default value.
  * @param  KeyScan_InitStruct: pointer to an KEYSCAN_InitTypeDef structure which will be initialized.
  * @retval None
  */
extern void KeyScan_StructInit(KEYSCAN_InitTypeDef *KeyScan_InitStruct);

/**
  * @brief  Enables or disables the specified KeyScan interrupts.
  * @param  KeyScan: selected KeyScan peripheral.
  * @param  KeyScan_IT: specifies the KeyScan interrupts sources to be enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg KEYSCAN_INT_TIMEOUT: KeyScan timeout interrupt mask
  *     @arg KEYSCAN_INT_OVER_THRESHOLD: Kescan FIFO data over threshold interrupt mask
  *     @arg KEYSCAN_INT_SCAN_END: KeyScan scan end interrupt mask
  * @param  newState: new state of the specified KeyScan interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
extern void KeyScan_INTConfig(KEYSCAN_TypeDef *KeyScan, uint32_t KeyScan_IT,
                              FunctionalState newState);

/**
  * @brief  Enables or disables the specified KeyScan interrupts mask.
  * @param  KeyScan: selected KeyScan peripheral.
  * @param  KeyScan_IT: specifies the KeyScan interrupts sources to be enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg KEYSCAN_INT_TIMEOUT: KeyScan timeout interrupt mask
  *     @arg KEYSCAN_INT_OVER_THRESHOLD: Kescan FIFO data over threshold interrupt mask
  *     @arg KEYSCAN_INT_SCAN_END: KeyScan scan end interrupt mask
  * @param  newState: new state of the specified KeyScan interrupts mask.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
extern void KeyScan_INTMask(KEYSCAN_TypeDef *KeyScan, uint32_t KeyScan_IT,
                            FunctionalState newState);

/**
  * @brief  Read data from keyscan FIFO.
  * @param  KeyScan: selected KeyScan peripheral.
  * @param[out]  outBuf: buffer to save data read from KeyScan FIFO.
  * @param  count: number of data to be read.
  * @retval None
  */
extern void KeyScan_Read(KEYSCAN_TypeDef *KeyScan, uint16_t *outBuf, uint16_t count);

/**
  * @brief  Enables or disables the KeyScan peripheral.
  * @param  KeyScan: selected KeyScan peripheral.
  * @param  NewState: new state of the KeyScan peripheral.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
extern void KeyScan_Cmd(KEYSCAN_TypeDef *KeyScan, FunctionalState NewState);

/**
  * @brief  Set filter data.
  * @param  KeyScan: selected KeyScan peripheral.
  * @param  data: config the data to be filtered.
  *   This parameter should not be more than 9 bits
  * @param  NewState: new state of the KeyScan peripheral.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval none.
  */
extern void KeyScan_FilterDataConfig(KEYSCAN_TypeDef *KeyScan, uint16_t data,
                                     FunctionalState NewState);

/**
  * @brief  KeyScan debounce time config.
  * @param  KeyScan: selected KeyScan peripheral.
  * @param time keyscan hardware debounce time
  * @param  NewState: new state of the KeyScan debounce function.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
__STATIC_INLINE void KeyScan_debounceConfig(KEYSCAN_TypeDef *KeyScan, uint8_t time,
                                            FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_KeyScan_PERIPH(KeyScan));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    KeyScan->TIMERCR &= ~((0xff << 16) | BIT31);
    KeyScan->TIMERCR |= ((NewState << 31) | time << 16);

}

/**
  * @brief  Get KeyScan FIFO data num.
  * @param  KeyScan: selected KeyScan peripheral.
  * @retval number of data in FIFO.
  */
__STATIC_INLINE uint16_t KeyScan_GetFifoDataNum(KEYSCAN_TypeDef *KeyScan)
{
    assert_param(IS_KeyScan_PERIPH(KeyScan));

    return (uint16_t)((KeyScan->STATUS & STATUS_FIFO_DATA_NUM_MASK) >> 4);
}

/**
  * @brief  Clears the KEyScan interrupt pending bits.
  * @param  KeyScan: selected KeyScan peripheral.
  * @param  KeyScan_IT: specifies the interrupt pending bit to clear.
  *   This parameter can be any combination of the following values:
  *     @arg KEYSCAN_INT_TIMEOUT: KeyScan timeout interrupt mask
  *     @arg KEYSCAN_INT_THRESHOLD
  *     @arg KEYSCAN_INT_OVER_READ
  *     @arg KEYSCAN_INT_SCAN_END
  *     @arg KEYSCAN_INT_FIFO_NOT_EMPTY
  *     @arg KEYSCAN_INT_ALL_RELEASE
  * @retval None
  */
__STATIC_INLINE void KeyScan_ClearINTPendingBit(KEYSCAN_TypeDef *KeyScan, uint32_t KeyScan_IT)
{
    /* Check the parameters */
    assert_param(IS_KeyScan_PERIPH(KeyScan));
    assert_param(IS_KEYSCAN_CONFIG_IT(KeyScan_IT));

    KeyScan->INTCLR |= KeyScan_IT;

    return;
}

/**
  * @brief  Clears KeyScan pending flags.
  * @param  KeyScan: selected KeyScan peripheral.
  * @param  KeyScan_FLAG: specifies the flag to clear.
  *   This parameter can be one of the following values:
  *     @arg KEYSCAN_FLAG_FIFOLIMIT
  *     @arg KEYSCAN_FLAG_DATAFILTER
  *     @arg KEYSCAN_FLAG_OVR
  * @note
  *   - KEYSCAN_FLAG_FULL and KEYSCAN_FLAG_EMPTY can't be cleared manually. They
  *     are cleared by hardware automatically.
  */
__STATIC_INLINE void KeyScan_ClearFlags(KEYSCAN_TypeDef *KeyScan, uint32_t KeyScan_FLAG)
{
    /* Check the parameters */
    assert_param(IS_KeyScan_PERIPH(KeyScan));
    assert_param(IS_KEYSCAN_CLEAR_FLAG(KeyScan_FLAG));

    if (KeyScan_FLAG & KEYSCAN_FLAG_FIFOLIMIT)
    {
        KeyScan->INTCLR |= BIT8;
    }
    if (KeyScan_FLAG & KEYSCAN_FLAG_DATAFILTER)
    {
        KeyScan->INTCLR |= BIT7;
    }
    if (KeyScan_FLAG & KEYSCAN_FLAG_OVR)
    {
        KeyScan->INTCLR |= BIT5;
    }

    return;
}

/**
  * @brief  Checks whether the specified KeyScan flag is set or not.
  * @param  KeyScan: selected KeyScan peripheral.
  * @param  KeyScan_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg KEYSCAN_FLAG_FIFOLIMIT
  *     @arg KEYSCAN_FLAG_THRESHOLD
  *     @arg KEYSCAN_FLAG_OVER_READ
  *     @arg KEYSCAN_FLAG_SCAN_END
  *     @arg KEYSCAN_FLAG_FIFO_NOT_EMPTY
  *     @arg KEYSCAN_FLAG_ALL_RELEASE
  *     @arg KEYSCAN_FLAG_DATAFILTER
  *     @arg KEYSCAN_FLAG_OVR
  *     @arg KEYSCAN_FLAG_FULL
  *     @arg KEYSCAN_FLAG_EMPTY
  * @retval The new state of KeyScan_FLAG (SET or RESET).
  */
__STATIC_INLINE FlagStatus KeyScan_GetFlagState(KEYSCAN_TypeDef *KeyScan, uint32_t KeyScan_FLAG)
{
    FlagStatus bitstatus = RESET;

    /* Check the parameters */
    assert_param(IS_KeyScan_PERIPH(KeyScan));
    assert_param(IS_KEYSCAN_FLAG(KeyScan_FLAG));

    if ((KeyScan->STATUS & KeyScan_FLAG) != 0)
    {
        bitstatus = SET;
    }

    return bitstatus;
}

/**
  * @brief  Checks whether the specified KeyScan flag is set or not.
  * @param  KeyScan: selected KeyScan peripheral.
  * @retval Keyscan fifo data.
  */
__STATIC_INLINE uint16_t KeyScan_ReadFifoData(KEYSCAN_TypeDef *KeyScan)
{
    /* Check the parameters */
    assert_param(IS_KeyScan_PERIPH(KeyScan));

    return (uint16_t)(KeyScan->FIFODATA);
}

#ifdef __cplusplus
}
#endif

#endif /* _RTL876X_KEYSCAN_H_ */

/** @} */ /* End of group KeyScan_Exported_Functions */
/** @} */ /* End of group KeyScan */

/******************* (C) COPYRIGHT 2015 Realtek Semiconductor *****END OF FILE****/


