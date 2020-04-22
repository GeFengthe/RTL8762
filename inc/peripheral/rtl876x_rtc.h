/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_rtc.h
* @brief
* @details
* @author    elliot_chen
* @date      2016-12-2
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _RTL876X_RTC_H_
#define _RTL876X_RTC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"

/* Peripheral: RTC */
/* Description: Real time counter register defines */

/* Register: sleep control global register0 */
#define RTC_SLEEP_CR                    (0x40000100UL)

/* RTC_SLEEP_CR[1] :RTC_CLK_SEL. sleep clock source sel(RTC & lp).  0x1: from BTAON (xtal or ring osc). 0x0: from 32k sdm(internal 32K after calibration). */
#define RTC_CLK_SEL_Pos                 (1UL)
#define EXTERNAL_CLK                    (0x1UL << RTC_CLK_SEL_Pos)
#define INTERNAL_CLK                    (~(0x1UL << RTC_CLK_SEL_Pos))

/* RTC_SLEEP_CR[0] :RTC_WAKEUP_EN. Enable wakeup signal to AON register (RTC & lp).  0x1: Enable. 0x0: Disable. */
#define RTC_WAKEUP_EN_Pos               (0UL)
#define RTC_WAKEUP_EN_Msk               (0x1UL << RTC_WAKEUP_EN_Pos)

/* Register: CR0 */
/* Description: RTC Control Register 0. Offset: 0x00. Address: 0x40000140. */

/* CR0[16] :RTC_INT_EN. Enable or disable tick interrupt.  0x1: Enable. 0x0: Disable. */
#define RTC_INT_TICK_EN_Pos             (16UL)
#define RTC_INT_TICK_EN_Msk             (0x1UL << RTC_INT_TICK_EN_Pos)

/* CR0[15] :RTC_INT_EN. Enable or disable compare3 interrupt.  0x1: Enable. 0x0: Disable. */
#define RTC_INT_COMP3_EN_Pos            (15UL)
#define RTC_INT_COMP3_EN_Msk            (0x1UL << RTC_INT_COMP3_EN_Pos)

/* CR0[14] :RTC_INT_EN. Enable or disable compare2 interrupt.  0x1: Enable. 0x0: Disable. */
#define RTC_INT_COMP2_EN_Pos            (14UL)
#define RTC_INT_COMP2_EN_Msk            (0x1UL << RTC_INT_COMP2_EN_Pos)

/* CR0[13] :RTC_INT_EN. Enable or disable compare1 interrupt.  0x1: Enable. 0x0: Disable. */
#define RTC_INT_COMP1_EN_Pos            (13UL)
#define RTC_INT_COMP1_EN_Msk            (0x1UL << RTC_INT_COMP1_EN_Pos)

/* CR0[12] :RTC_INT_EN. Enable or disable compare0 interrupt.  0x1: Enable. 0x0: Disable. */
#define RTC_INT_COMP0_EN_Pos            (12UL)
#define RTC_INT_COMP0_EN_Msk            (0x1UL << RTC_INT_COMP0_EN_Pos)

/* CR0[4] :RTC_MASK_TICK_INT. Mask RTC tick interrupt.  0x1: Unmask. 0x0: Mask. */
#define RTC_MASK_TICK_INT_Pos           (4UL)
#define RTC_MASK_TICK_INT_Msk           (0x1UL << RTC_MASK_TICK_INT_Pos)

/* CR0[2] :RTC_DIV_COUNTER_RST. Reset Prescale Counter. 0x1: Reset Counter to 0. */
#define RTC_DIV_COUNTER_RST_Pos         (2UL)
#define RTC_DIV_COUNTER_RST_Msk         (0x1UL << RTC_DIV_COUNTER_RST_Pos)
#define RTC_DIV_COUNTER_RST_CLR         (~(RTC_DIV_COUNTER_RST_Msk))

/* CR0[1] :RTC_COUNTER_RST. Reset 24bit-RTC Counter. 0x1: Reset Counter to 0. */
#define RTC_COUNTER_RST_Pos             (1UL)
#define RTC_COUNTER_RST_Msk             (0x1UL << RTC_COUNTER_RST_Pos)
#define RTC_COUNTER_RST_CLR             (~(RTC_COUNTER_RST_Msk))

/* CR0[0] :RTC_START. Start or stop RTC 24bit-RTC Counter. 0x1: Start 24bit-RTC Counter.0x0: Stop 24bit-RTC Counter. */
#define RTC_START_Pos                   (0UL)
#define RTC_START_Msk                   (0x1UL << RTC_START_Pos)
#define RTC_START_CLR                   (~(RTC_START_Msk))

/* Register: INT_MASK */
/* Description: Interrupt mask register. Offset: 0x04. Address: 0x40000144. */

/* INT_MASK[5] :RTC_COMP3_CLR. Clear Interrupt Status of Comparator1. */
/* This interrupt is cleared by software.write 1 then write 0 after 2T to clear. */
#define RTC_COMP3_CLR_Pos               (5UL)
#define RTC_COMP3_CLR_SET               (0x1UL << RTC_COMP3_CLR_Pos)
#define RTC_COMP3_CLR_RESET             (~(RTC_COMP3_CLR_SET))

/* INT_MASK[4] :RTC_COMP2_CLR. Clear Interrupt Status of Comparator1. */
/* This interrupt is cleared by software.write 1 then write 0 after 2T to clear. */
#define RTC_COMP2_CLR_Pos               (4UL)
#define RTC_COMP2_CLR_SET               (0x1UL << RTC_COMP2_CLR_Pos)
#define RTC_COMP2_CLR_RESET             (~(RTC_COMP2_CLR_SET))

/* INT_MASK[3] :RTC_COMP1_CLR. Clear Interrupt Status of Comparator1. */
/* This interrupt is cleared by software.write 1 then write 0 after 2T to clear. */
#define RTC_COMP1_CLR_Pos               (3UL)
#define RTC_COMP1_CLR_SET               (0x1UL << RTC_COMP1_CLR_Pos)
#define RTC_COMP1_CLR_RESET             (~(RTC_COMP1_CLR_SET))

/* INT_MASK[2] :RTC_COMP0_CLR. Clear Interrupt Status of Comparator0. */
/* This interrupt is cleared by software.write 1 then write 0 after 2T to clear. */
#define RTC_COMP0_CLR_Pos               (2UL)
#define RTC_COMP0_CLR_SET               (0x1UL << RTC_COMP0_CLR_Pos)
#define RTC_COMP0_CLR_RESET             (~(RTC_COMP0_CLR_SET))

/* INT_MASK[1] :RTC_OVERFLOW_CLR. Clear Interrupt Status of Overflow. */
/* This interrupt is cleared by software.write 1 then write 0 after 2T to clear. */
#define RTC_OVERFLOW_CLR_Pos            (1UL)
#define RTC_OVERFLOW_CLR_SET            (0x1UL << RTC_OVERFLOW_CLR_Pos)
#define RTC_OVERFLOW_CLR_RESET          (~(RTC_OVERFLOW_CLR_SET))

/* INT_MASK[0] :RTC_TICK_CLR. Clear Interrupt Status of Tick. */
/* This interrupt is cleared by software.write 1 then write 0 after 2T to clear. */
#define RTC_TICK_CLR_Pos                (0UL)
#define RTC_TICK_CLR_SET                (0x1UL << RTC_TICK_CLR_Pos)
#define RTC_TICK_CLR_RESET              (~(RTC_TICK_CLR_SET))

#define RTC_COMP_INT_EN_TO_MASK_OFFSET  (RTC_INT_COMP0_EN_Pos - RTC_COMP0_CLR_Pos)


/* Clear all interrupt */
#define RTC_ALL_INT_CLR_SET             (RTC_COMP3_CLR_SET | RTC_COMP2_CLR_SET | \
                                         RTC_COMP1_CLR_SET | RTC_COMP0_CLR_SET | \
                                         RTC_OVERFLOW_CLR_SET | RTC_TICK_CLR_SET)

#define RTC_ALL_INT_CLR_RESET           (RTC_COMP3_CLR_RESET & RTC_COMP2_CLR_RESET & \
                                         RTC_COMP1_CLR_RESET & RTC_COMP0_CLR_RESET & \
                                         RTC_OVERFLOW_CLR_RESET & RTC_TICK_CLR_RESET)

/** @addtogroup RTC RTC
  * @brief RTC driver module
  * @{
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/


/** @defgroup RTC_Exported_Constants RTC Exported Constants
  * @{
  */

/** @defgroup RTC_interrupts_definition RTC Interrupts Definition
  * @{
  */

#define RTC_INT_TICK                  ((uint32_t)(1 << 0))
#define RTC_INT_OVF                   ((uint32_t)(1 << 1))
#define RTC_INT_CMP0                  ((uint32_t)(1 << 2))
#define RTC_INT_CMP1                  ((uint32_t)(1 << 3))
#define RTC_INT_CMP2                  ((uint32_t)(1 << 4))
#define RTC_INT_CMP3                  ((uint32_t)(1 << 5))
#define IS_RTC_CONFIG_INT(INT) (((INT) == RTC_INT_TICK) || \
                                ((INT) == RTC_INT_OVF) || \
                                ((INT) == RTC_INT_CMP0) || \
                                ((INT) == RTC_INT_CMP1) || \
                                ((INT) == RTC_INT_CMP2) || \
                                ((INT) == RTC_INT_CMP3))

#define IS_RTC_COMP_INT(INT) ( ((INT) == RTC_INT_CMP0) || \
                               ((INT) == RTC_INT_CMP1) || \
                               ((INT) == RTC_INT_CMP2) || \
                               ((INT) == RTC_INT_CMP3))

/** End of group RTC_interrupts_definition
  * @}
  */

/** @defgroup RTC_sleep_mode_clock_definition RTC sleep mode clock Definition
  * @{
  */
#define RTC_EXTERNAL_CLK            EXTERNAL_CLK
#define RTC_INTERNAL_CLK            INTERNAL_CLK
#define IS_RTC_SLEEP_CLK(CLK) ( ((CLK) == EXTERNAL_CLK) || \
                                ((CLK) == RTC_INT_CMP3))

/** End of group RTC_sleep_mode_clock_definition
  * @}
  */

/** End of group RTC_Exported_Constants
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup RTC_Exported_Functions RTC Exported Functions
  * @{
  */

/**
  * @brief  Reset RTC.
  * @param  None.
  * @retval None
  */
void RTC_DeInit(void);

/**
  * @brief  Set RTC prescaler value.
  * @param  value: the prescaler value to be set.should be no more than 12 bits!
  * @retval None
  */
void RTC_SetPrescaler(uint32_t value);

/**
  * @brief  Set RTC comparator value.
  * @param  index: the comparator number.
  * @param  value: the comparator value to be set.
  * @retval None
  */
void RTC_SetComp(uint8_t index, uint32_t value);

/**
  * @brief  Start or stop RTC peripheral.
  * @param  NewState: new state of RTC peripheral.
  *   This parameter can be the following values:
  *     @arg ENABLE: Start RTC.
  *     @arg DISABLE: Stop RTC.
  * @retval None
  */
void RTC_RunCmd(FunctionalState NewState);

/**
  * @brief  Mask or unmask the specified RTC interrupts.
  * @param  RTC_INT: specifies the RTC interrupt source to be enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg RTC_INT_TICK: RTC tick interrupt
  *     @arg RTC_INT_OVF: RT counter overflow interrupt
  *     @arg RTC_INT_CMP0: compare 0 interrupt
  *     @arg RTC_INT_CMP1: compare 1 interrupt
  *     @arg RTC_INT_CMP2: compare 2 interrupt
  *     @arg RTC_INT_CMP3: compare 3 interrupt
  * @param  NewState: new state of the specified RTC interrupt.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RTC_MaskINTConfig(uint32_t RTC_INT, FunctionalState NewState);

/**
  * @brief  Enable or disable the specified RTC interrupts.
  * @param  RTC_INT: specifies the RTC interrupt source to be enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg RTC_INT_CMP0: compare 0 interrupt
  *     @arg RTC_INT_CMP1: compare 1 interrupt
  *     @arg RTC_INT_CMP2: compare 2 interrupt
  *     @arg RTC_INT_CMP3: compare 3 interrupt
  * @param  NewState: new state of the specified RTC interrupt.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RTC_CompINTConfig(uint32_t RTC_INT, FunctionalState NewState);

/**
  * @brief  Enable or disable RTC tick interrupts.
  * @param  NewState: new state of RTC tick interrupt.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RTC_TickINTConfig(FunctionalState NewState);

/**
  * @brief  Checks whether the specified RTC interrupt is set or not.
   * @param  RTC_INT: specifies the RTC interrupt source to be enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg RTC_INT_TICK: RTC tick interrupt
  *     @arg RTC_INT_OVF: RT counter overflow interrupt
  *     @arg RTC_INT_CMP0: compare 0 interrupt
  *     @arg RTC_INT_CMP1: compare 1 interrupt
  *     @arg RTC_INT_CMP2: compare 2 interrupt
  *     @arg RTC_INT_CMP3: compare 3 interrupt
  * @retval The new state of RTC_INT (SET or RESET).
  */
ITStatus RTC_GetINTStatus(uint32_t RTC_INT);

/**
  * @brief  Enable or disable system wake up of RTC or LPC.
  * @param  NewState: new state of the wake up function.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RTC_SystemWakeupConfig(FunctionalState NewState);

/**
  * @brief  Fast write RTC register.
  * @param  offset: the offset of RTC register.
  * @param  data: data which write to register.
  * @retval None
  */
void RTC_WriteReg(uint32_t offset, uint32_t data);

/**
  * @brief  Reset prescaler counter value of RTC.
  * @param  None
  * @retval the counter value.
  */
void RTC_ResetPrescalerCounter(void);

/**
  * @brief  Get counter value of RTC.
  * @param  None
  * @retval the counter value.
  */
__STATIC_INLINE uint32_t RTC_GetCounter(void)
{
    return RTC->CNT;
}

/**
  * @brief  Reset counter value of RTC.
  * @param  None
  * @retval the counter value.
  */
__STATIC_INLINE void RTC_ResetCounter(void)
{
    RTC_WriteReg((uint32_t)(&(RTC->CR0)), (RTC->CR0) | RTC_COUNTER_RST_Msk);
    __NOP();
    __NOP();
    RTC_WriteReg((uint32_t)(&(RTC->CR0)), (RTC->CR0) & RTC_COUNTER_RST_CLR);
}

/**
  * @brief  Get RTC comparator value.
  * @param  index: the comparator number.
  * @retval the comparator value.
  */
__STATIC_INLINE uint32_t RTC_GetComp(uint8_t index)
{
    return *((volatile uint32_t *)(&(RTC->COMP0) + index));
}

/**
  * @brief  Clear interrupt of the select comparator of RTC.
  * @param  index: the comparator number.
  * @retval None
  */
__STATIC_INLINE void RTC_ClearCompINT(uint8_t index)
{
    RTC_WriteReg((uint32_t)(&(RTC->INT_MASK)), BIT(RTC_COMP0_CLR_Pos + index));
    __NOP();
    __NOP();
    RTC_WriteReg((uint32_t)(&(RTC->INT_MASK)), 0);
}

/**
  * @brief  Clear overflow interrupt of RTC.
  * @param  None
  * @retval None.
  */
__STATIC_INLINE void RTC_ClearOverFlowINT(void)
{
    RTC_WriteReg((uint32_t)(&(RTC->INT_MASK)), RTC_OVERFLOW_CLR_SET);
    __NOP();
    __NOP();
    RTC_WriteReg((uint32_t)(&(RTC->INT_MASK)), 0);
}

/**
  * @brief  Clear tick interrupt of RTC.
  * @param  None
  * @retval None.
  */
__STATIC_INLINE void RTC_ClearTickINT(void)
{
    RTC_WriteReg((uint32_t)(&(RTC->INT_MASK)), RTC_TICK_CLR_SET);
    __NOP();
    __NOP();
    RTC_WriteReg((uint32_t)(&(RTC->INT_MASK)), 0);
}

/**
  * @brief  Select sleep clock source .
  * @param  clock: sleep clock source.
  *   This parameter can be the following values:
  *     @arg RTC_EXTERNAL_CLK: from BTAON (xtal or ring osc)
  *     @arg RTC_INTERNAL_CLK: from 32k sdm(internal 32K after calibration)
  * @retval None
  */
__STATIC_INLINE void RTC_SleepModeClkConfig(uint32_t clock)
{
    /* Check the parameters */
    assert_param(IS_RTC_SLEEP_CLK(clock));

    RTC_WriteReg(RTC_SLEEP_CR, *(volatile uint32_t *)(RTC_SLEEP_CR) & clock);
}

#ifdef __cplusplus
}
#endif

#endif /* _RTL876X_RTC_H_ */

/** @} */ /* End of group RTC_Exported_Functions */
/** @} */ /* End of group RTC */

/******************* (C) COPYRIGHT 2016 Realtek Semiconductor *****END OF FILE****/

