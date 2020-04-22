/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     rtl876x_rtc.c
* @brief    This file provides all the RTC firmware functions.
* @details
* @author   elliot_chen
* @date     2016-12-2
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_rtc.h"

/* Internal defines ------------------------------------------------------------*/
#define REG_RTC_FAST_WRITE_BASE_ADDR    (0x40000100UL)
#define REG_RTC_FAST_WDATA              (0x400001f0UL)
#define REG_RTC_FAST_ADDR               (0x400001f4UL)
#define REG_RTC_WR_STROBE               (0x400001f8UL)

/**
  * @brief  Fast write RTC register.
  * @param  offset: the offset of RTC register.
  * @param  data: data which write to register.
  * @retval None
  */
void RTC_WriteReg(uint32_t offset, uint32_t data)
{
    static bool is_called = false;

    if (is_called == false)
    {
        *((volatile uint32_t *)0x40000014) |= BIT(9);//no need run this every time
        is_called = true;
    }

    /* Write data */
    *((volatile uint32_t *)REG_RTC_FAST_WDATA) = data;
    /* Write RTC register address. Only offset */
    *((volatile uint32_t *)REG_RTC_FAST_ADDR) = offset - REG_RTC_FAST_WRITE_BASE_ADDR;
    *((volatile uint32_t *)REG_RTC_WR_STROBE) = 1;
}

/**
  * @brief  Reset RTC.
  * @param  None.
  * @retval None
  */
void RTC_DeInit(void)
{
    /* Stop RTC counter */
    RTC_WriteReg((uint32_t)(&(RTC->CR0)), (RTC->CR0 & RTC_START_CLR));

    /* Disable wakeup signal */
    RTC_WriteReg(RTC_SLEEP_CR, 0x0);

    /* Clear all RTC interrupt */
    RTC_WriteReg((uint32_t)(&(RTC->INT_MASK)), RTC_ALL_INT_CLR_SET);
    __NOP();
    __NOP();
    RTC_WriteReg((uint32_t)(&(RTC->INT_MASK)), RTC_ALL_INT_CLR_RESET);

    /* Clear prescale register */
    RTC_WriteReg((uint32_t)(&(RTC->PRESCALER)), 0);
    /* Clear all comparator register */
    RTC_WriteReg((uint32_t)(&(RTC->COMP0)), 0);
    RTC_WriteReg((uint32_t)(&(RTC->COMP1)), 0);
    RTC_WriteReg((uint32_t)(&(RTC->COMP2)), 0);
    RTC_WriteReg((uint32_t)(&(RTC->COMP3)), 0);

    /* Reset prescale counter and counter */
    RTC_WriteReg((uint32_t)(&(RTC->CR0)), RTC_DIV_COUNTER_RST_Msk | RTC_COUNTER_RST_Msk);
    __NOP();
    __NOP();
    RTC_WriteReg((uint32_t)(&(RTC->CR0)), 0x0);
}

/**
  * @brief  Set RTC prescaler value.
  * @param  PrescaleValue: the prescaler value to be set.should be no more than 12 bits!
  * @retval None
  */
void RTC_SetPrescaler(uint32_t value)
{
    RTC_WriteReg((uint32_t)(&(RTC->PRESCALER)), value);
}

/**
  * @brief  Set RTC comparator value.
  * @param  COMPNum: the comparator number.
  * @param  COMPValue: the comparator value to be set.
  * @retval None
  */
void RTC_SetComp(uint8_t index, uint32_t value)
{
    RTC_WriteReg((uint32_t)(&(RTC->COMP0) + index), value);
}

/**
  * @brief  Start or stop RTC peripheral.
  * @param  NewState: new state of RTC peripheral.
  *   This parameter can be the following values:
  *     @arg ENABLE: Start RTC.
  *     @arg DISABLE: Stop RTC.
  * @retval None
  */
void RTC_RunCmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == ENABLE)
    {
        /* Start RTC */
        RTC_WriteReg((uint32_t)(&(RTC->CR0)), RTC->CR0 | RTC_START_Msk);
    }
    else
    {
        /* Stop RTC */
        RTC_WriteReg((uint32_t)(&(RTC->CR0)), RTC->CR0 & RTC_START_CLR);
    }
}

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
void RTC_MaskINTConfig(uint32_t RTC_INT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_RTC_CONFIG_INT(RTC_INT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != ENABLE)
    {
        /* Unmask the selected RTC interrupt */
        RTC_WriteReg((uint32_t)(&(RTC->CR0)), (RTC->CR0) | (RTC_INT << RTC_MASK_TICK_INT_Pos));
    }
    else
    {
        /* Mask the selected RTC interrupt */
        RTC_WriteReg((uint32_t)(&(RTC->CR0)), (RTC->CR0) & (~(RTC_INT << RTC_MASK_TICK_INT_Pos)));
    }
}

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
void RTC_CompINTConfig(uint32_t RTC_INT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_RTC_COMP_INT(RTC_INT));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == ENABLE)
    {
        /* Enable the selected RTC comparator interrupt */
        RTC_WriteReg((uint32_t)(&(RTC->CR0)), (RTC->CR0) | (RTC_INT << RTC_COMP_INT_EN_TO_MASK_OFFSET));
    }
    else
    {
        /* Disable the selected RTC comparator interrupt */
        RTC_WriteReg((uint32_t)(&(RTC->CR0)), (RTC->CR0) & (~(RTC_INT << RTC_COMP_INT_EN_TO_MASK_OFFSET)));
    }
}

/**
  * @brief  Enable or disable RTC tick interrupts.
  * @param  NewState: new state of RTC tick interrupt.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RTC_TickINTConfig(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == ENABLE)
    {
        /* Enable RTC tick interrupt */
        RTC_WriteReg((uint32_t)(&(RTC->CR0)), RTC->CR0 | RTC_INT_TICK_EN_Msk);
    }
    else
    {
        /* Disable RTC tick interrupt */
        RTC_WriteReg((uint32_t)(&(RTC->CR0)), RTC->CR0 & (~RTC_INT_TICK_EN_Msk));
    }
}

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
ITStatus RTC_GetINTStatus(uint32_t RTC_INT)
{
    ITStatus bit_status = RESET;

    /* Check the parameters */
    assert_param(IS_RTC_CONFIG_INT(RTC_INT));

    if ((RTC->INT_SR & RTC_INT) != (uint32_t)RESET)
    {
        bit_status = SET;
    }

    /* Return the RTC_INT status */
    return  bit_status;
}

/**
  * @brief  Enable or disable system wake up of RTC or LPC.
  * @param  NewState: new state of the wake up function.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void RTC_SystemWakeupConfig(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == ENABLE)
    {
        /* Enable system wake up */
        RTC_WriteReg(RTC_SLEEP_CR, (*(volatile uint32_t *)(RTC_SLEEP_CR)) | RTC_WAKEUP_EN_Msk);
    }
    else
    {
        /* Disable system wake up */
        RTC_WriteReg(RTC_SLEEP_CR, (*(volatile uint32_t *)(RTC_SLEEP_CR)) & (~RTC_WAKEUP_EN_Msk));
    }
}

/******************* (C) COPYRIGHT 2016 Realtek Semiconductor Corporation *****END OF FILE****/

