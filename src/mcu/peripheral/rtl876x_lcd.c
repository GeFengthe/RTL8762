/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file       rtl876x_lcd.c
* @brief     This file provides all the LCD 8080 parallel interface firmware functions.
* @details
* @author   elliot chen
* @date      2017-11-24
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_rcc.h"
#include "rtl876x_lcd.h"

/* Internal defines -----------------------------------------------------------*/
#define LCD_REG_TEST_MODE           *((volatile uint32_t *)0x400002A8UL)

/**
  * @brief  Deinitializes the LCD peripheral registers to their default values.
  * @param  None.
  * @retval None
  */
void LCD_DeInit(void)
{
    RCC_PeriphClockCmd(APBPeriph_LCD, APBPeriph_LCD_CLOCK, DISABLE);
}

/**
  * @brief  Select the LCD output pin group.
  * @param LCD_PinGroupType:
    This parameter can be one of the following values:
    --LCD_PinGroup_DISABLE : disable LCD interface
    --LCD_PinGroup_1 : CS(P3_3), RD(P3_2), DCX(P3_4), WR(P3_5),
      D0(P0_2), D1(P0_4), D2(P1_3), D3(P1_4), D4(P4_0), D5(P4_1), D6(P4_2), D7(P4_3)
    --LCD_PinGroup_2 : CS(P3_3), DCX(P3_4), WR(P3_2), RD(P2_0)
      D0(P3_5), D1(P0_1), D2(P0_2), D3(P0_4), D4(P4_0), D5(P4_1), D6(P4_2), D7(P4_3)
  * @retval None
  */
void LCD_PinGroupConfig(uint32_t LCD_PinGroupType)
{
    /* Check the parameters */
    assert_param(IS_LCD_PINGROUP_IDX(LCD_PinGroupType));

    /* Dedicated SDIO pin option */
    LCD_REG_TEST_MODE &= ~(0x03 << 28);
    LCD_REG_TEST_MODE |= LCD_PinGroupType;
}

/**
  * @brief Initializes the LCD peripheral according to the specified
  *   parameters in the LCD_InitStruct
  * @param  LCD: selected LCD peripheral.
  * @param  LCD_InitStruct: pointer to a LCD_InitTypeDef structure that
  *   contains the configuration information for the specified LCD peripheral
  * @retval None
  */
void LCD_Init(LCD_InitTypeDef *LCD_InitStruct)
{
    /* Check the parameters */
    assert_param(IS_LCD_CLOCK_DIV(LCD_InitStruct->LCD_ClockDiv));
    assert_param(IS_LCD_GUARD_TIME_CMD(LCD_InitStruct->LCD_GuardTimeCmd));
    assert_param(IS_LCD_GUARD_TIME(LCD_InitStruct->LCD_GuardTime));
    assert_param(IS_LCD_8BitSwap_CMD(LCD_InitStruct->LCD_8BitSwap));
    assert_param(IS_LCD_16BitSwap_CMD(LCD_InitStruct->LCD_16BitSwap));
    assert_param(IS_LCD_TX_DMA_CMD(LCD_InitStruct->LCD_TxDMACmd));

    /* Configure Tx FIFO threshold and mask interrupt */
    LCD->IMR = LCD_InitStruct->LCD_TxThr | LCD_MASK_TX_FIFO_EMPTY_INT_MSK | \
               LCD_MASK_TX_FIFO_OVERFLOW_INT_MSK | LCD_MASK_TX_FIFO_THR_INT_MSK;

    /* Configure clock divider, bypass guard time, frame format */
    LCD->CTRL0 = LCD_InitStruct->LCD_ClockDiv | LCD_InitStruct->LCD_GuardTimeCmd | \
                 LCD_InitStruct->LCD_GuardTime | LCD_InitStruct->LCD_8BitSwap | \
                 LCD_InitStruct->LCD_16BitSwap;

    /* Configure DMA parameter */
    LCD->CTRL1 = LCD_TX_DMA_FIFO_THR | LCD_InitStruct->LCD_TxDMACmd;

    /* Configure LCD mode */
    LCD->CFG = LCD_InitStruct->LCD_Mode | LCD_MANUAL_CS_CTRL_MSK;
}

/**
  * @brief  Fills each LCD_InitStruct member with its default value.
  * @param  LCD_InitStruct: pointer to an LCD_InitTypeDef structure which will be initialized.
  * @retval None
  */
void LCD_StructInit(LCD_InitTypeDef *LCD_InitStruct)
{
    LCD_InitStruct->LCD_ClockDiv        = LCD_CLOCK_DIV_2;
    LCD_InitStruct->LCD_Mode            = LCD_MODE_MANUAL;
    LCD_InitStruct->LCD_GuardTimeCmd    = LCD_GUARD_TIME_DISABLE;
    LCD_InitStruct->LCD_GuardTime       = LCD_GUARD_TIME_1T;
    LCD_InitStruct->LCD_8BitSwap        = LCD_8BitSwap_DISABLE;
    LCD_InitStruct->LCD_16BitSwap       = LCD_16BitSwap_ENABLE;
    LCD_InitStruct->LCD_TxThr           = 10;/* !< This value can be 0 to 16 */
    LCD_InitStruct->LCD_TxDMACmd        = LCD_TX_DMA_DISABLE;
}

/**
  * @brief   Enable or disable the selected LCD mode.
  * @param  NewState: new state of the operation mode.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void LCD_Cmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
        LCD->CTRL0 |= LCD_OUTPUT_START_MSK;
    }
    else
    {
        LCD->CTRL0 &= LCD_OUTPUT_START_CLR;
    }
}

/**
  * @brief   Send command in manual mode.
  * @param  cmd: command which to be sent.
  * @retval None
  */
void LCD_SendCommand(uint8_t cmd)
{
    /* Fill command */
    LCD->DR = cmd;

    /* Enable command control */
    LCD->CFG &= LCD_CMD_DATA_CTRL_CLR;

    /* Send command */
    LCD->CFG |= LCD_WR_TRIGGER_CTRL_MSK;

    /* Check  write status */
    while ((LCD->CFG) & LCD_WR_TRIGGER_CTRL_MSK);
}

/**
  * @brief   Send data in manual mode.
  * @param  pBuf: buffer address to be sent.
  * @param  len:  data length.
  * @retval None
  */
void LCD_SendData(uint8_t *pBuf, uint32_t len)
{
    while (len--)
    {
        /* Fill data */
        LCD->DR = *pBuf++;

        /* Send command */
        LCD->CFG |= LCD_CMD_DATA_CTRL_MSK | LCD_WR_TRIGGER_CTRL_MSK;

        /* Check write status */
        while ((LCD->CFG) & LCD_WR_TRIGGER_CTRL_MSK);
    }
}

/**
  * @brief   Receive data in manual mode.
  * @param  pBuf: buffer address to be received.
  * @param  len: data length.
  * @retval None
  */
void LCD_ReceiveData(uint8_t *pBuf, uint32_t len)
{
    uint32_t reg_value = 0;

    while (len--)
    {
        /* Select read data control */
        LCD->CFG |= LCD_CMD_DATA_CTRL_MSK | LCD_RD_TRIGGER_CTRL_MSK;

        /* Check read status */
        while ((LCD->CFG) & LCD_RD_TRIGGER_CTRL_MSK);

        /* Read data  */
        reg_value = LCD->RXDATA;
        if (reg_value & LCD_RX_DATA_VALID_MSK)
        {
            *pBuf++ = reg_value & LCD_RX_DATA_MSK;
        }
    }
}

/**
  * @brief   Send command and data buffer in manual mode.
  * @param cmd: command which to be sent.
  * @param  pBuf: buffer address to be sent.
  * @param  len:  data length.
  * @retval None
  */
void LCD_Write(uint8_t cmd, uint8_t *pBuf, uint32_t len)
{
    /* Pull CS up */
    LCD_SetCS();

    /* Send command */
    LCD_SendCommand(cmd);

    /* Write data */
    LCD_SendData(pBuf, len);

    /* Pull CS up */
    LCD_SetCS();
}

/**
  * @brief   Send command and read data buffer in manual mode.
  * @param cmd: command which to be sent.
  * @param  pBuf: buffer address to be sent.
  * @param  len:  data length.
  * @retval None
  */
void LCD_Read(uint8_t cmd, uint8_t *pBuf, uint32_t len)
{
    /* Pull CS up */
    LCD_SetCS();

    /* Send command */
    LCD_SendCommand(cmd);

    /* Read data */
    LCD_ReceiveData(pBuf, len);

    /* Pull CS up */
    LCD_SetCS();
}

/**
  * @brief   Configure command sequences in auto mode.
 * @param  pCmdBuf: buffer address which store command sequence.
  * @param  len:  command length.
  * @retval None
  */
FlagStatus LCD_SetCmdSequence(uint8_t *pCmdBuf, uint8_t len)
{
    uint32_t i = 0;

    /* Check parameters */
    if ((len > 4) || (len < 1))
    {
        return RESET;
    }

    /* Configure command number */
    LCD->CFG &= LCD_AUTO_MODE_CMD_NUM_CLR;
    LCD->CFG |= (len - 1) & LCD_AUTO_MODE_CMD_NUM_MSK;

    LCD->DR = 0;
    /* Fill command sequence */
    for (i = 0; i < len; i++)
    {
        LCD->DR |= (*pCmdBuf++) << (i * 8);
    }

    return SET;
}

/**
  * @brief  Mask or unmask the specified LCD interrupts.
  * @param  LCD_INT_MSK: specifies the LCD interrupts sources to be mask or unmask.
  *   This parameter can be the following values:
  *     @arg LCD_INT_TF_EMPTY_MSK: Mask TX FIFO empty interrupt.
  *     @arg LCD_INT_TF_OF_MSK: Mask TX FIFO overflow interrupt.
  *     @arg LCD_INT_TF_LEVEL_MSK: Mask TX FIFO threshold interrupt.
  * @param  NewState: new state of the specified LCD interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void LCD_MaskINTConfig(uint32_t LCD_INT_MSK, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_LCD_INT_MSK_CONFIG(LCD_INT_MSK));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == ENABLE)
    {
        LCD->IMR |= LCD_INT_MSK;
    }
    else
    {
        LCD->IMR &= ~LCD_INT_MSK;
    }
}

/**
  * @brief Get the specified LCD interrupt status.
  * @param  LCD_INT: the specified LCD interrupts.
  * This parameter can be one of the following values:
  *       @arg LCD_INT_SR_AUTO_DONE: Auto done interrupt.
  *     @arg LCD_INT_SR_TF_EMPTY: TX FIFO empty interrupt.
  *     @arg LCD_INT_SR_TF_OF: TX FIFO overflow interrupt.
  *     @arg LCD_INT_SR_TF_LEVEL: TX FIFO threshold interrupt.
  * @retval The new state of LCD_INT (SET or RESET).
  */
ITStatus LCD_GetINTStatus(uint32_t LCD_INT)
{
    ITStatus bit_status = RESET;

    /* Check the parameters */
    assert_param(IS_LCD_INT_CONFIG(LCD_INT));

    if (LCD->SR & LCD_INT)
    {
        bit_status = SET;
    }

    /* Return the LCD_INT status */
    return  bit_status;
}

/**
  * @brief Get the specified LCD flag status.
  * @param  LCD_INT: the specified LCD flag.
  * This parameter can be one of the following values:
  *   This parameter can be the following values:
  *     @arg LCD_FLAG_TF_EMPTY:  FIFO empty flag.
  *     @arg LCD_FLAG_TF_FULL:  FIFO full flag.
  * @retval The new state of LCD_FLAG (SET or RESET).
  */
FlagStatus LCD_GetFlagStatus(uint32_t LCD_FLAG)
{
    FlagStatus bit_status = RESET;

    /* Check the parameters */
    assert_param(IS_LCD_FLAG_CONFIG(LCD_FLAG));

    if (LCD->SR & LCD_FLAG)
    {
        bit_status = SET;
    }

    /* Return the LCD_FLAG status */
    return  bit_status;
}

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

