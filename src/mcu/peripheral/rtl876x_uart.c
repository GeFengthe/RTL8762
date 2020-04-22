/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     rtl876x_uart.c
* @brief    This file provides all the UART firmware functions.
* @details
* @author   tifnan_ge
* @date     2015-05-08
* @version  v0.1
*********************************************************************************************************
*/
#include "rtl876x.h"
#include "rtl876x_bitfields.h"
#include "rtl876x_rcc.h"
#include "rtl876x_uart.h"

#define LCR_DLAB_Set                    ((uint32_t)(1 << 7))
#define LCR_DLAB_Reset                  ((uint32_t)~(1 << 7))

/**
  * @brief Initializes the UART peripheral according to the specified
  *   parameters in the UART_InitStruct
  * @param  UARTx: selected UART peripheral.
  * @param  UART_InitStruct: pointer to a UART_InitTypeDef structure that
  *   contains the configuration information for the specified UART peripheral
  * @retval None
  */
void UART_Init(UART_TypeDef *UARTx, UART_InitTypeDef *UART_InitStruct)
{
    assert_param(IS_UART_PERIPH(UARTx));
    assert_param(IS_UART_WORD_LENGTH(UART_InitStruct->wordLen));
    assert_param(IS_UART_PARITY(UART_InitStruct->parity));
    assert_param(IS_UART_STOPBITS(UART_InitStruct->stopBits));
    assert_param(IS_UART_AUTO_FLOW_CTRL(UART_InitStruct->autoFlowCtrl));
    assert_param(IS_UART_DMA_CFG(UART_InitStruct->dmaEn));
    assert_param(IS_UART_RX_FIFO_TRIGGER_LEVEL(UART_InitStruct->rxTriggerLevel));
    assert_param(IS_UART_IDLE_TIME(UART_InitStruct->idle_time));

    //clear DLAB bit
    UARTx->LCR &= LCR_DLAB_Reset;
    //disable all interrupt
    UARTx->DLH_INTCR = 0x00;

    //read to clear Line Status Reg
    UARTx->LSR;
    //clear FIFO
    UARTx->INTID_FCR |= (FCR_CLEAR_RX_FIFO_Set | FCR_CLEAR_TX_FIFO_Set);

    //set baudrate, firstly set DLAB bit
    UARTx->LCR |= LCR_DLAB_Set;
    //set calibration parameters(OVSR)
    UARTx->STSR &= ~0xF0;
    UARTx->STSR |= (UART_InitStruct->ovsr << 4);
    //set calibration parameters(OVSR_adj)
    UARTx->SPR &= (~(0x7ff << 16));
    UARTx->SPR |= (UART_InitStruct->ovsr_adj << 16);
    //set DLL and DLH
    UARTx->DLL = (UART_InitStruct->div & 0x00FF);
    UARTx->DLH_INTCR = ((UART_InitStruct->div & 0xFF00) >> 8);
    //after set baudrate, clear DLAB bit
    UARTx->LCR &= LCR_DLAB_Reset;

    //set LCR reg
    UARTx->LCR = (UART_InitStruct->parity | UART_InitStruct->stopBits | UART_InitStruct->wordLen);
    //set FCR reg, FIFO must enable
    UARTx->INTID_FCR = ((1 << 0) | UART_InitStruct->rxTriggerLevel << 8 | UART_InitStruct->dmaEn);

    /* auto flow control */
    UARTx->MCR &= (~((1 << 5) | (1 << 1)));
    UARTx->MCR |= UART_InitStruct->autoFlowCtrl;

    /* set rx idle time */
    UARTx->RX_IDLE_INTTCR = (UART_InitStruct->idle_time);

    if (UART_InitStruct->dmaEn == UART_DMA_ENABLE)
    {
        /* Config UART Tx dma parameter */
        if (UART_InitStruct->TxDmaEn != DISABLE)
        {
            /* Mask uart TX threshold value */
            UARTx->MISCR &= ~(0x1f << 3);
            UARTx->MISCR |= ((UART_InitStruct->TxWaterlevel) << 3) | BIT(1);
        }
        /* Config UART Rx dma parameter */
        if (UART_InitStruct->RxDmaEn != DISABLE)
        {
            /* Mask uart RX threshold value */
            UARTx->MISCR &= ~(0x3f << 8);
            UARTx->MISCR |= (UART_InitStruct->RxWaterlevel << 8) | BIT(2);
        }
    }

    return;
}

/**
  * @brief  Deinitializes the UART peripheral registers to their default reset values(turn off UART clock).
  * @param  UARTx: selected UART peripheral.
  * @retval None
  */
void UART_DeInit(UART_TypeDef *UARTx)
{
    assert_param(IS_UART_PERIPH(UARTx));

    if (UARTx == UART)
    {
        RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, DISABLE);
    }
    else if (UARTx == UART1)
    {
        RCC_PeriphClockCmd(APBPeriph_UART1, APBPeriph_UART1_CLOCK, DISABLE);
    }
    else if (UARTx == UART2)
    {
        RCC_PeriphClockCmd(APBPeriph_UART2, APBPeriph_UART2_CLOCK, DISABLE);
    }
    return;
}

/**
  * @brief  Fills each UART_InitStruct member with its default value.
  * @param  UART_InitStruct: pointer to an UART_InitTypeDef structure which will be initialized.
  * @retval None
  */
void UART_StructInit(UART_InitTypeDef *UART_InitStruct)
{
    //115200 default
    UART_InitStruct->div            = 20;
    UART_InitStruct->ovsr           = 12;
    UART_InitStruct->ovsr_adj       = 0x252;

    UART_InitStruct->parity         = UART_PARITY_NO_PARTY;
    UART_InitStruct->stopBits       = UART_STOP_BITS_1;
    UART_InitStruct->wordLen        = UART_WROD_LENGTH_8BIT;
    UART_InitStruct->dmaEn          = UART_DMA_DISABLE;
    UART_InitStruct->autoFlowCtrl   = UART_AUTO_FLOW_CTRL_DIS;
    UART_InitStruct->rxTriggerLevel = 16;                       //1~29
    UART_InitStruct->idle_time      = UART_RX_IDLE_2BYTE;      //idle interrupt wait time
    UART_InitStruct->TxWaterlevel   = 15;     //Better to equal TX_FIFO_SIZE(16)- GDMA_MSize
    UART_InitStruct->RxWaterlevel   = 1;      //Better to equal GDMA_MSize
    UART_InitStruct->TxDmaEn   = DISABLE;
    UART_InitStruct->RxDmaEn   = DISABLE;
    return;
}

/**
  * @brief  Receive data from rx FIFO.
  * @param  UARTx: selected UART peripheral.
  * @param[out]  outBuf: buffer to save data read from UART FIFO.
  * @param  count: number of data to be read.
  * @retval None
  */
void UART_ReceiveData(UART_TypeDef *UARTx, uint8_t *outBuf, uint16_t count)
{
    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));

    while (count--)
    {
        *outBuf++ = (uint8_t)UARTx->RB_THR;
    }

    return;
}

/**
  * @brief  Send data to tx FIFO.
  * @param  UARTx: selected UART peripheral.
  * @param  inBuf: buffer to be written to Tx FIFO.
  * @param  count: number of data to be written.
  * @retval None
  */
void UART_SendData(UART_TypeDef *UARTx, const uint8_t *inBuf, uint16_t count)
{
    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));

    while (count--)
    {
        UARTx->RB_THR = *inBuf++;
    }

    return;
}

/**
  * @brief  Enables or disables the specified UART interrupts.
  * @param  UARTx: selected UARTx peripheral.
  * @param  UART_IT: specifies the UART interrupts sources to be enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg UART_INT_RD_AVA: enable Rx data avaliable interrupt.
  *     @arg UART_INT_FIFO_EMPTY: enable FIFO empty interrupt.
  *     @arg UART_INT_LINE_STS: enable line status interrupt.
  *     @arg UART_INT_MODEM_STS: enable modem status interrupt.
  * @param  NewState: new state of the specified UART interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void UART_INTConfig(UART_TypeDef *UARTx, uint32_t UART_IT, FunctionalState newState)
{
    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));
    assert_param(IS_FUNCTIONAL_STATE(newState));
    assert_param(IS_UART_IT(UART_IT));

    if (UART_IT & UART_INT_IDLE)
    {
        if (newState == ENABLE)
        {
            UARTx->STSR  |= BIT3;
            UARTx->STSR  &= (~BIT3);
            UARTx->RXIDLE_INTCR |= BIT0;
            UARTx->RX_IDLE_INTTCR |= BIT31;
        }
        else
        {
            UARTx->RX_IDLE_INTTCR &= (~BIT31);
            UARTx->RX_IDLE_SR |= BIT0;
            UARTx->RXIDLE_INTCR &= (~BIT0);
        }
    }
    if (UART_IT & 0xf)
    {
        if (newState == ENABLE)
        {
            /* Enable the selected UARTx interrupts */
            UARTx->DLH_INTCR |= UART_IT;
        }
        else
        {
            /* Disable the selected UARTx interrupts */
            UARTx->DLH_INTCR &= (uint32_t)~UART_IT;
        }
    }
    return;
}

/**
  * @brief  Checks whether the specified UART flag is set or not.
  * @param  UARTx: selected UART peripheral.
  * @param  UART_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg UART_FLAG_RX_DATA_RDY: rx data is avaliable.
  *     @arg UART_FLAG_RX_OVERRUN: rx overrun.
  *     @arg UART_FLAG_PARTY_ERR: parity error.
  *     @arg UART_FLAG_FRAME_ERR: UARTx frame error.
  *     @arg UART_FLAG_BREAK_ERR: UARTx break error.
  *     @arg UART_FLAG_THR_EMPTY: tx FIFO is empty.
  *     @arg UART_FLAG_THR_TSR_EMPTY: tx FIFO and tx shift reg are both empty.
  *     @arg UART_FLAG_RX_FIFO_ERR: rx FIFO error.
  *     @arg UART_FLAG_RX_IDLE.
  * @retval The new state of UART_FLAG (SET or RESET).
  */
FlagStatus UART_GetFlagState(UART_TypeDef *UARTx, uint32_t UART_FLAG)
{
    FlagStatus bitstatus = RESET;

    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));
    assert_param(IS_UART_GET_FLAG(UART_FLAG));

    if (UART_FLAG != UART_FLAG_RX_IDLE)
    {
        if (UARTx->LSR & UART_FLAG)
        {
            bitstatus = SET;
        }
    }
    else
    {
        if (UARTx->RX_IDLE_SR & BIT(0))
        {
            bitstatus = SET;
        }
    }
    return bitstatus;

}

/**
  *@brief  UART loop back mode config.
  *@param  UARTx: selected UART peripheral.
  *@param  NewState: new state of the DMA Channelx.
  *   This parameter can be: ENABLE or DISABLE.
  *@retval None.
  */
void UART_LoopBackCmd(UART_TypeDef *UARTx, FunctionalState NewState)
{
    assert_param(IS_UART_PERIPH(UARTx));

    if (NewState == ENABLE)
    {
        UARTx->MCR |= BIT4;
    }
    else
    {
        UARTx->MCR &= ~BIT4;
    }
}

/**
  *@brief  Change UART baudrate.
  *@param  UARTx: selected UART peripheral.
  *@param  div: parameter of the selected UART baudrate.
  *@param  ovsr: parameter of the selected UART baudrate.
  *@param  ovsr_adj: parameter of the selected UART baudrate.
  *@retval None.
  */
void UART_ChangeBaudRate(UART_TypeDef *UARTx, uint16_t div, uint16_t ovsr, uint16_t ovsr_adj)
{
    //set baudrate, firstly set DLAB bit
    UARTx->LCR |= LCR_DLAB_Set;

    //set calibration parameters(OVSR)
    UARTx->STSR &= ~0xF0;
    UARTx->STSR |= (ovsr << 4);
    //set calibration parameters(OVSR_adj)
    UARTx->SPR &= (~(0x7ff << 16));
    UARTx->SPR |= (ovsr_adj << 16);
    //set DLL and DLH
    UARTx->DLL = (div & 0x00FF);
    UARTx->DLH_INTCR = ((div & 0xFF00) >> 8);

    //after set baudrate, clear DLAB bit
    UARTx->LCR &= LCR_DLAB_Reset;
}

/**
  *@brief  Change UART parameters.
  *@param  UARTx: selected UART peripheral.
  *@param  parity: parity of selected UART peripheral.
  *@param  stopBits: stop bit of selected UART peripheral.
  *@param  wordLen: data width of selected UART peripheral.
  *@retval None.
  */
void UART_ChangeParams(UART_TypeDef *UARTx, uint16_t parity, uint16_t stopBits, uint16_t wordLen)
{
    //set LCR reg
    UARTx->LCR = (parity | stopBits | wordLen);
}

