/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_gpio.h
* @brief
* @details
* @author    elliot chen
* @date      2015-05-20
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __RTL876X_GPIO_H
#define __RTL876X_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"
#include "rtl876x_bitfields.h"

/** @cond private
  * @defgroup GPIO Debounce register
  * @{
  */

/* GPIO Private Defines */
#define GPIO_DBCLK_DIV *((volatile uint32_t *)0x40000344UL)

/**
  * @}
  * @endcond
  */

/** @
  * @brief GPIO_Number GPIO Number
  */

#define GPIO0   0
#define GPIO1   1
#define GPIO2   2
#define GPIO3   3
#define GPIO4   4
#define GPIO5   5
#define GPIO6   6
#define GPIO7   7
#define GPIO8   8
#define GPIO9   9
#define GPIO10  10
#define GPIO11  11
#define GPIO12  12
#define GPIO13  13
#define GPIO14  14
#define GPIO15  15
#define GPIO16  16
#define GPIO17  17
#define GPIO18  18
#define GPIO19  19
#define GPIO20  20
#define GPIO21  21
#define GPIO22  22
#define GPIO23  23
#define GPIO24  24
#define GPIO25  25
#define GPIO26  26
#define GPIO27  27
#define GPIO28  28
#define GPIO29  29
#define GPIO30  30
#define GPIO31  31


/** @addtogroup GPIO GPIO
  * @brief GPIO driver module
  * @{
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/


/** @defgroup GPIO_Exported_Constants GPIO Exported Constants
  * @{
  */

/**
  * @brief GPIO mode enumeration
  */

typedef enum
{
    GPIO_Mode_IN   = 0x00, /*!< GPIO Input Mode             */
    GPIO_Mode_OUT  = 0x01, /*!< GPIO Output Mode                */
} GPIOMode_TypeDef;

#define IS_GPIO_MODE(MODE) (((MODE) == GPIO_Mode_IN)|| ((MODE) == GPIO_Mode_OUT))

/**
 * @brief Setting interrupt's trigger type
 *
 * Setting interrupt's trigger type
 */
typedef enum
{
    GPIO_INT_Trigger_LEVEL = 0x0, /**< This interrupt is level trigger  */
    GPIO_INT_Trigger_EDGE  = 0x1, /**< This interrupt is edge trigger  */
    GPIO_INT_BOTH_EDGE = 0x2,     /**< This interrupt is both edge trigger  */
} GPIOIT_LevelType;

#define IS_GPIOIT_LEVEL_TYPE(TYPE) (((TYPE) == GPIO_INT_Trigger_LEVEL)\
                                    || ((TYPE) == GPIO_INT_Trigger_EDGE)\
                                    || ((TYPE) == GPIO_INT_BOTH_EDGE))

/**
 * @brief Setting interrupt active mode
 *
 * Setting interrupt active mode
 */
typedef enum
{
    GPIO_INT_POLARITY_ACTIVE_LOW  = 0x0, /**< Setting interrupt to low active  */
    GPIO_INT_POLARITY_ACTIVE_HIGH = 0x1, /**< Setting interrupt to high active */
} GPIOIT_PolarityType;

#define IS_GPIOIT_POLARITY_TYPE(TYPE) (((TYPE) == GPIO_INT_POLARITY_ACTIVE_LOW)\
                                       || ((TYPE) == GPIO_INT_POLARITY_ACTIVE_HIGH))

/**
 * @brief Enable/Disable interrupt debounce mode
 *
 * Enable/Disable interrupt debounce mode
 */
typedef enum
{
    GPIO_INT_DEBOUNCE_DISABLE = 0x0, /**< Disable interrupt debounce  */
    GPIO_INT_DEBOUNCE_ENABLE  = 0x1, /**< Enable interrupt debounce   */
} GPIOIT_DebounceType;

#define IS_GPIOIT_DEBOUNCE_TYPE(TYPE) (((TYPE) == GPIO_INT_DEBOUNCE_DISABLE)\
                                       || ((TYPE) == GPIO_INT_DEBOUNCE_ENABLE))

/**
* @brief hardware/software mode select
*
* Select hardware mode or software mode
*/
typedef enum
{
    GPIO_SOFTWARE_MODE = 0x0, /**< Gpio Software mode(default) */
    GPIO_HARDWARE_MODE  = 0x1, /**< Gpio Hardware control mode  */
} GPIOControlMode_Typedef;

#define IS_GPIOIT_MODDE(TYPE) (((TYPE) == GPIO_SOFTWARE_MODE)\
                               || ((TYPE) == GPIO_HARDWARE_MODE))


/**
  * @brief  Bit_SET and Bit_RESET enumeration
  */

typedef enum
{
    Bit_RESET = 0,
    Bit_SET
} BitAction;

#define IS_GPIO_BIT_ACTION(ACTION) (((ACTION) == Bit_RESET) || ((ACTION) == Bit_SET))

/** End of group GPIO_Exported_Constants
  * @}
  */

/*============================================================================*
 *                         Types
 *============================================================================*/


/** @defgroup GPIO_Exported_Types GPIO Exported Types
* @{
*/

/**
  * @brief  GPIO Init structure definition
  */

typedef struct
{
    uint32_t                  GPIO_Pin;        /*!< Specifies the GPIO pins to be configured.
                                                             This parameter can be any value of @ref GPIO_pins_define */
    GPIOMode_TypeDef          GPIO_Mode;       /*!< Specifies the operating mode for the selected pins.
                                                             This parameter can be a value of @ref GPIOMode_TypeDef */
    FunctionalState           GPIO_ITCmd;      /**< Enable or disable GPIO interrupt.
                                                             This parameter can be a value of DISABLE or ENABLE */

    GPIOIT_LevelType          GPIO_ITTrigger;  /**< Interrupt mode is level or edge trigger.
                                                             This parameter can be a value of DISABLE or ENABLE */

    GPIOIT_PolarityType       GPIO_ITPolarity; /**< Interrupt mode is high or low active trigger */

    GPIOIT_DebounceType       GPIO_ITDebounce; /**< Enable or disable de-bounce for interrupt */

    GPIOControlMode_Typedef   GPIO_ControlMode; /**< Specifies the gpio mode */

    uint32_t GPIO_DebounceTime;                  /**< per.(ms) Specifies the gpio debounce time setting */
} GPIO_InitTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup GPIO_Exported_Constants GPIO Exported Constants
  * @{
  */

/** @defgroup GPIO_pins_define GPIO Pins Define
  * @{
  */
#define GPIO_Pin_0                 ((uint32_t)0x00000001)  /*!< Pin 0 selected    */
#define GPIO_Pin_1                 ((uint32_t)0x00000002)  /*!< Pin 1 selected    */
#define GPIO_Pin_2                 ((uint32_t)0x00000004)  /*!< Pin 2 selected    */
#define GPIO_Pin_3                 ((uint32_t)0x00000008)  /*!< Pin 3 selected    */
#define GPIO_Pin_4                 ((uint32_t)0x00000010)  /*!< Pin 4 selected    */
#define GPIO_Pin_5                 ((uint32_t)0x00000020)  /*!< Pin 5 selected    */
#define GPIO_Pin_6                 ((uint32_t)0x00000040)  /*!< Pin 6 selected    */
#define GPIO_Pin_7                 ((uint32_t)0x00000080)  /*!< Pin 7 selected    */
#define GPIO_Pin_8                 ((uint32_t)0x00000100)  /*!< Pin 8 selected    */
#define GPIO_Pin_9                 ((uint32_t)0x00000200)  /*!< Pin 9 selected    */
#define GPIO_Pin_10                ((uint32_t)0x00000400)  /*!< Pin 10 selected   */
#define GPIO_Pin_11                ((uint32_t)0x00000800)  /*!< Pin 11 selected   */
#define GPIO_Pin_12                ((uint32_t)0x00001000)  /*!< Pin 12 selected   */
#define GPIO_Pin_13                ((uint32_t)0x00002000)  /*!< Pin 13 selected   */
#define GPIO_Pin_14                ((uint32_t)0x00004000)  /*!< Pin 14 selected   */
#define GPIO_Pin_15                ((uint32_t)0x00008000)  /*!< Pin 15 selected   */
#define GPIO_Pin_16                ((uint32_t)0x00010000)  /*!< Pin 16 selected    */
#define GPIO_Pin_17                ((uint32_t)0x00020000)  /*!< Pin 17 selected    */
#define GPIO_Pin_18                ((uint32_t)0x00040000)  /*!< Pin 18 selected    */
#define GPIO_Pin_19                ((uint32_t)0x00080000)  /*!< Pin 19 selected    */
#define GPIO_Pin_20                ((uint32_t)0x00100000)  /*!< Pin 20 selected    */
#define GPIO_Pin_21                ((uint32_t)0x00200000)  /*!< Pin 21 selected    */
#define GPIO_Pin_22                ((uint32_t)0x00400000)  /*!< Pin 22 selected    */
#define GPIO_Pin_23                ((uint32_t)0x00800000)  /*!< Pin 23 selected    */
#define GPIO_Pin_24                ((uint32_t)0x01000000)  /*!< Pin 24 selected    */
#define GPIO_Pin_25                ((uint32_t)0x02000000)  /*!< Pin 25 selected    */
#define GPIO_Pin_26                ((uint32_t)0x04000000)  /*!< Pin 26 selected   */
#define GPIO_Pin_27                ((uint32_t)0x08000000)  /*!< Pin 27 selected   */
#define GPIO_Pin_28                ((uint32_t)0x10000000)  /*!< Pin 28 selected   */
#define GPIO_Pin_29                ((uint32_t)0x20000000)  /*!< Pin 29 selected   */
#define GPIO_Pin_30                ((uint32_t)0x40000000)  /*!< Pin 30 selected   */
#define GPIO_Pin_31                ((uint32_t)0x80000000)  /*!< Pin 31 selected   */
#define GPIO_Pin_All               ((uint32_t)0xFFFFFFFF)  /*!< All pins selected */

#define IS_GPIO_PIN(PIN) ((PIN) != (uint32_t)0x00)

#define IS_PIN_NUM(NUM) ((NUM) <= (uint8_t)P4_1)

#define IS_GET_GPIO_PIN(PIN) (((PIN) == GPIO_Pin_0) || \
                              ((PIN) == GPIO_Pin_1) || \
                              ((PIN) == GPIO_Pin_2) || \
                              ((PIN) == GPIO_Pin_3) || \
                              ((PIN) == GPIO_Pin_4) || \
                              ((PIN) == GPIO_Pin_5) || \
                              ((PIN) == GPIO_Pin_6) || \
                              ((PIN) == GPIO_Pin_7) || \
                              ((PIN) == GPIO_Pin_8) || \
                              ((PIN) == GPIO_Pin_9) || \
                              ((PIN) == GPIO_Pin_10) || \
                              ((PIN) == GPIO_Pin_11) || \
                              ((PIN) == GPIO_Pin_12) || \
                              ((PIN) == GPIO_Pin_13) || \
                              ((PIN) == GPIO_Pin_14) || \
                              ((PIN) == GPIO_Pin_15) || \
                              ((PIN) == GPIO_Pin_16) || \
                              ((PIN) == GPIO_Pin_17) || \
                              ((PIN) == GPIO_Pin_18) || \
                              ((PIN) == GPIO_Pin_19) || \
                              ((PIN) == GPIO_Pin_20) || \
                              ((PIN) == GPIO_Pin_21) || \
                              ((PIN) == GPIO_Pin_22) || \
                              ((PIN) == GPIO_Pin_23) || \
                              ((PIN) == GPIO_Pin_24) || \
                              ((PIN) == GPIO_Pin_25) || \
                              ((PIN) == GPIO_Pin_26) || \
                              ((PIN) == GPIO_Pin_27) || \
                              ((PIN) == GPIO_Pin_28) || \
                              ((PIN) == GPIO_Pin_29) || \
                              ((PIN) == GPIO_Pin_30) || \
                              ((PIN) == GPIO_Pin_31))
/** End of group GPIO_pins_define
  * @}
  */

/** End of group GPIO_Exported_Constants
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup GPIO_Exported_Functions GPIO Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the GPIO peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void GPIO_DeInit(void);

/**
  * @brief  Initializes the GPIO peripheral according to the specified
  *         parameters in the GPIO_InitStruct.
  * @param  GPIO_InitStruct: pointer to a GPIO_InitTypeDef structure that
  *         contains the configuration information for the specified GPIO peripheral.
  * @retval None
  */
void GPIO_Init(GPIO_InitTypeDef *GPIO_InitStruct);

/**
  * @brief    Fills each GPIO_InitStruct member with its default value.
  * @param  GPIO_InitStruct : pointer to a GPIO_InitTypeDef structure which will
  *    be initialized.
  * @retval None
  */
void GPIO_StructInit(GPIO_InitTypeDef *GPIO_InitStruct);

/**
  * @brief enable the specified GPIO interrupt.
  * @param  GPIO_Pin: where x can be 0 or 31.
  * @param  NewState: enable or disable interrupt
  * @retval None
  */
void GPIO_INTConfig(uint32_t GPIO_Pin, FunctionalState NewState);

/**
  * @brief clear the specified GPIO interrupt.
  * @param  GPIO_Pin: where x can be 0 or 31.
  * @retval None
  */
void GPIO_ClearINTPendingBit(uint32_t GPIO_Pin);

/**
  * @brief mask the specified GPIO interrupt.
  * @param  GPIO_Pin: where x can be 0 or 31.
  * @param  NewState: disable or enable interrupt.
  * @retval None
  */
void GPIO_MaskINTConfig(uint32_t GPIO_Pin, FunctionalState NewState);

/**
  * @brief get the specified GPIO pin.
  * @param  Pin_num: This parameter is from ADC_0 to P4_1, please refer to rtl876x.h "Pin_Number" part.
  * @retval  GPIO pin for GPIO initialization.
  */
uint32_t GPIO_GetPin(uint8_t Pin_num);

/**
  * @brief get the specified GPIO pin number.
  * @param  Pin_num: This parameter is from ADC_0 to P4_1, please refer to rtl876x.h "Pin_Number" part.
  * @retval  GPIO pin number.
  */
uint8_t GPIO_GetNum(uint8_t Pin_num);

/**
  * @brief get the specified GPIO pin number.
  * @param  Pin_num: This parameter is from ADC_0 to P4_1, please refer to rtl876x.h "Pin_Number" part.
  * @retval  GPIO pin number.
  */
void GPIO_DBClkCmd(FunctionalState NewState);

/**
  * @brief    Reads the specified input port pin.
  * @param  GPIO_Pin:  specifies the port bit to read.
  *   This parameter can be GPIO_Pin_x where x can be (0..31).
  * @retval The input port pin value.
  */
__STATIC_INLINE uint8_t GPIO_ReadInputDataBit(uint32_t GPIO_Pin)
{
    uint8_t bitstatus = RESET;

    /* Check the parameters */
    assert_param(IS_GET_GPIO_PIN(GPIO_Pin));

    if (GPIO->DATAIN & GPIO_Pin)
    {
        bitstatus = (uint8_t)SET;
    }

    return bitstatus;
}

/**
  * @brief  Reads value of all  GPIO input data port.
  * @param  None
  * @retval GPIO input data port value.
  */
__STATIC_INLINE uint32_t GPIO_ReadInputData(void)
{
    return GPIO->DATAIN;
}

/**
  * @brief  Reads the specified output port pin.
  * @param  GPIO_Pin:  specifies the port bit to read.
  *   This parameter can be GPIO_Pin_x where x can be (0..31).
  * @retval The output port pin value.
  */
__STATIC_INLINE uint8_t GPIO_ReadOutputDataBit(uint32_t GPIO_Pin)
{
    uint8_t bitstatus = RESET;

    /* Check the parameters */
    assert_param(IS_GET_GPIO_PIN(GPIO_Pin));

    if (GPIO->DATAOUT & GPIO_Pin)
    {
        bitstatus = (uint8_t)SET;
    }

    return bitstatus;
}

/**
  * @brief  Reads value of all  GPIO output data port.
  * @param  None
  * @retval GPIO output data port value.
  */
__STATIC_INLINE uint32_t GPIO_ReadOutputData(void)
{
    return ((uint32_t)GPIO->DATAOUT);
}

/**
  * @brief  Sets the selected data port bits.
  * @param  GPIO_Pin: specifies the port bits to be written.
  *   This parameter can be GPIO_Pin_x where x can be (0..31) or GPIO_Pin_All.
  * @retval None
  */
__STATIC_INLINE void GPIO_SetBits(uint32_t GPIO_Pin)
{
    /* Check the parameters */
    assert_param(IS_GPIO_PIN(GPIO_Pin));

    GPIO->DATAOUT |= GPIO_Pin;
}

/**
  * @brief  Resets the selected data port bits.
  * @param  GPIO_Pin: specifies the port bits to be written.
  *   This parameter can be GPIO_Pin_0 to GPIO_Pin_31 or GPIO_Pin_All.
  * @retval None
  */
__STATIC_INLINE void GPIO_ResetBits(uint32_t GPIO_Pin)
{
    /* Check the parameters */
    assert_param(IS_GPIO_PIN(GPIO_Pin));

    GPIO->DATAOUT &= ~(GPIO_Pin);
}

/**
  * @brief  Sets or clears the selected data port bit.
  * @param  GPIO_Pin: specifies the port bit to be written.
  *   This parameter can be one of GPIO_Pin_x where x can be (0..31).
  * @param  BitVal: specifies the value to be written to the selected bit.
  *   This parameter can be one of the BitAction enum values:
  *     @arg Bit_RESET: to clear the port pin
  *     @arg Bit_SET: to set the port pin
  * @retval None
  */
__STATIC_INLINE void GPIO_WriteBit(uint32_t GPIO_Pin, BitAction BitVal)
{
    /* Check the parameters */
    assert_param(IS_GPIO_PIN(GPIO_Pin));
    assert_param(IS_GPIO_BIT_ACTION(BitVal));

    if (BitVal != Bit_RESET)
    {
        GPIO->DATAOUT |= GPIO_Pin;
    }
    else
    {
        GPIO->DATAOUT &= ~(GPIO_Pin);
    }
}

/**
  * @brief  Sets or clears the selected data port .
  * @param  PortVal: specifies the value to be written to the selected bit.
  * @retval None
  */
__STATIC_INLINE void GPIO_Write(uint32_t PortVal)
{
    GPIO->DATAOUT = PortVal;
}

/**
  * @brief  return  GPIO interrupt status.
  * @param  GPIO_Pin: specifies the port bit to be written.
  *   This parameter can be one of GPIO_Pin_x where x can be (0..31).
  * @retval ITStatus The new state of GPIO_IT (SET or RESET).
  */
__STATIC_INLINE ITStatus GPIO_GetINTStatus(uint32_t GPIO_Pin)
{
    /* Check the parameters */
    assert_param(IS_GET_GPIO_PIN(GPIO_Pin));

    if ((GPIO->INTSTATUS & GPIO_Pin) == GPIO_Pin)
    {
        return SET;
    }
    else
    {
        return RESET;
    }
}

/**
  * @brief  Set debounce time.
  * @param  DebounceTime: specifies interrupt debounce time
  *   This parameter can be 1ms ~ 64ms
  * @retval none
  */
__STATIC_INLINE void GPIO_Debounce_Time(uint32_t DebounceTime)
{
    uint8_t count = 0;

    if (DebounceTime < 1)
    {
        DebounceTime = 1;
    }
    if (DebounceTime > 64)
    {
        DebounceTime = 64;
    }
#ifdef _IS_ASIC_
    //div = 14;//0xd  0b1101<<8
    GPIO_DBCLK_DIV = ((0xd << 8) | (1 << 12));
#else
    //div = 13;//0xc  0b1100<<8
    GPIO_DBCLK_DIV |= ((1 << 11) | (1 << 10) | (1 << 12));
    GPIO_DBCLK_DIV &= (~((1 << 9) | (1 << 8)));
#endif
    count = (244 * DebounceTime) / 100 - 1;
    GPIO_DBCLK_DIV &= (~((0xff << 0)));
    GPIO_DBCLK_DIV = GPIO_DBCLK_DIV + count;
}

/** @} */ /* End of group GPIO_Exported_Functions */
/** @} */ /* End of group GPIO */


#ifdef __cplusplus
}
#endif

#endif /*__RTL876X_GPIO_H*/



/******************* (C) COPYRIGHT 2015 Realtek Semiconductor Corporation *****END OF FILE****/

