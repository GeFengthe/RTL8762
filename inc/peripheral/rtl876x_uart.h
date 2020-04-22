/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_uart.h
* @brief     header file of uart driver.
* @details
* @author    tifnan_ge
* @date      2015-05-08
* @version   v1.0
* *********************************************************************************************************
*/


#ifndef _RTL876X_UART_H_
#define _RTL876X_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"
#include "rtl876x_bitfields.h"


/** @addtogroup UART UART
  * @brief UART driver module
  * @{
  */

/*============================================================================*
 *                         Types
 *============================================================================*/


/** @defgroup UART_Exported_Types UART Exported Types
  * @{
  */

/**
 * @brief UART initialize parameters
 *
 * UART initialize parameters
 */
typedef struct
{
    //baudrate calibration
    uint16_t ovsr_adj;              /*!< Specifies the Baudrate setting, ovsr_adj, please refer to baudrate setting table.*/
    uint16_t div;                   /*!< Specifies the Baudrate setting, div, please refer to baudrate setting table.*/
    uint16_t ovsr;                  /*!< Specifies the Baudrate setting, ovsr, please refer to baudrate setting table.*/

    uint16_t wordLen;               /*!< Specifies the UART Wordlength
                                        This parameter can be a value of @ref UART_Wrod_Length. */
    uint16_t parity;                /*!< Specifies the Rx dma mode.
                                        This parameter can be a value of @ref UART_Parity. */
    uint16_t stopBits;              /*!< Specifies the Rx dma mode.
                                        This parameter can be a value of @ref UART_Stop_Bits. */
    uint16_t autoFlowCtrl;          /*!< Specifies the Rx dma mode.
                                        This parameter can be a value of @ref UART_Hardware_Flow_Control. */

    uint16_t rxTriggerLevel;        /*!< Specifies the Rx dma mode.
                                        This parameter must range from 1 to 29.*/
    uint16_t dmaEn;                 /*!< Specifies the Rx dma mode.
                                        This parameter must be a value of DISABLE and ENABLE */
    uint16_t idle_time;             /*!< Specifies the Rx dma mode.
                                        This parameter can be a value of @ref UART_Rx_idle_time. */

    uint8_t TxWaterlevel;           /*!< Specifies the DMA tx water level.
                                        This parameter must range from 1 to 16.*/
    uint8_t RxWaterlevel;           /*!< Specifies the DMA rx water level.
                                        This parameter must range from 1 to 31.*/
    uint16_t TxDmaEn;               /*!< Specifies the Tx dma mode.
                                        This parameter must be a value of DISABLE and ENABLE */
    uint16_t RxDmaEn;               /*!< Specifies the Rx dma mode.
                                        This parameter must be a value of DISABLE and ENABLE */

} UART_InitTypeDef;

/** End of group UART_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/


/** @defgroup UART_Exported_Constants UART Exported Constants
  * @{
  */

#define IS_UART_PERIPH(PERIPH) ((PERIPH) == UART)
#define UART_TX_FIFO_SIZE           16
#define UART_RX_FIFO_SIZE           32

/** @defgroup UART_Interrupts_Definition UART Interrupts Definition
  * @{
  */

#define UART_INT_RD_AVA                             ((uint16_t)(1 << 0))     //receive data avaliable
#define UART_INT_FIFO_EMPTY                         ((uint16_t)(1 << 1))
#define UART_INT_LINE_STS                           ((uint16_t)(1 << 2))
#define UART_INT_MODEM_STS                          ((uint16_t)(1 << 3))
#define UART_INT_IDLE                               ((uint16_t)(1 << 7))

#define IS_UART_IT(IT) ((((IT) & 0xFFFFFFF0) == 0x00) && ((IT) != 0x00))

#define IS_UART_GET_IT(IT) ((IT) & (UART_INT_RD_AVA | UART_INT_FIFO_EMPTY | UART_INT_LINE_STS |\
                                    UART_INT_MODEM_STS | UART_INT_IDLE))

/** End of group UART_Interrupts_Definition
  * @}
  */

/** @defgroup UART_Interrupt_Identifier UART Interrupt Identifier
  * @{
  */

#define UART_INT_ID_LINE_STATUS                     ((uint16_t)(0x03 << 1))
#define UART_INT_ID_RX_LEVEL_REACH                  ((uint16_t)(0x02 << 1))
#define UART_INT_ID_RX_TMEOUT                       ((uint16_t)(0x06 << 1))
#define UART_INT_ID_TX_EMPTY                        ((uint16_t)(0x01 << 1))
#define UART_INT_ID_MODEM_STATUS                    ((uint16_t)(0x00 << 1))

#define IS_UART_IT_ID(ID) (((ID) == UART_INT_ID_LINE_STATUS) || ((ID) == UART_INT_ID_RX_LEVEL_REACH)\
                           || ((ID) == UART_INT_ID_RX_TMEOUT) || ((ID) == UART_INT_ID_TX_EMPTY)\
                           || ((ID) == UART_INT_ID_MODEM_STATUS))

/** End of group UART_Interrupt_Identifier
  * @}
  */

/** @defgroup UART_Flag UART Flag
  * @{
  */

#define UART_FLAG_INT_PEND                         ((uint16_t)(1 << 0))
#define UART_FLAG_RX_DATA_RDY                      ((uint16_t)(1 << 0))
#define UART_FLAG_RX_OVERRUN                       ((uint16_t)(1 << 1))
#define UART_FLAG_PARTY_ERR                        ((uint16_t)(1 << 2))
#define UART_FLAG_FRAME_ERR                        ((uint16_t)(1 << 3))
#define UART_FLAG_BREAK_ERR                        ((uint16_t)(1 << 4))
#define UART_FLAG_THR_EMPTY                        ((uint16_t)(1 << 5))     //Transmitter Holding Register or Transmitter FIFO empty
#define UART_FLAG_THR_TSR_EMPTY                    ((uint16_t)(1 << 6))     //Transmitter Holding Register(or tx FIFO) and Transmitter shift Register both empty
#define UART_FLAG_RX_FIFO_ERR                      ((uint16_t)(1 << 7))
#define UART_FLAG_RX_IDLE                          ((uint16_t)(1 << 8))     //Only to show difference cause the address of UART RX Ilde flag is isolate

#define IS_UART_GET_FLAG(FLAG) (((FLAG) == UART_FLAG_RX_DATA_RDY)   || ((FLAG) == UART_FLAG_RX_OVERRUN)\
                                || ((FLAG) == UART_FLAG_PARTY_ERR) || ((FLAG) == UART_FLAG_FRAME_ERR)\
                                || ((FLAG) == UART_FLAG_BREAK_ERR) || ((FLAG) == UART_FLAG_THR_EMPTY)\
                                || ((FLAG) == UART_FLAG_THR_TSR_EMPTY) || ((FLAG) == UART_FLAG_RX_FIFO_ERR)\
                                || ((FLAG) == UART_FLAG_RX_IDLE))

/** End of group UART_Flag
  * @}
  */

/** @defgroup UART_RX_FIFO_Level UART RX FIFO Level
  * @{
  */

#define UART_RX_FIFO_TRIGGER_LEVEL_1BYTE            ((uint16_t)(0x01))
#define UART_RX_FIFO_TRIGGER_LEVEL_4BYTE            ((uint16_t)(0x04))
#define UART_RX_FIFO_TRIGGER_LEVEL_8BYTE            ((uint16_t)(0x08))
#define UART_RX_FIFO_TRIGGER_LEVEL_14BYTE           ((uint16_t)(0x0E))

#define IS_UART_RX_FIFO_TRIGGER_LEVEL(LEVEL) (((LEVEL) >= 1) && ((LEVEL) <= 29))

/** End of group UART_RX_FIFO_Level
  * @}
  */


/** @defgroup UART_Rx_idle_time UART Rx idle time
  * @{
  */

#define UART_RX_IDLE_1BYTE                 ((uint16_t)(0x00))
#define UART_RX_IDLE_2BYTE                 ((uint16_t)(0x01))
#define UART_RX_IDLE_4BYTE                 ((uint16_t)(0x02))
#define UART_RX_IDLE_8BYTE                 ((uint16_t)(0x03))
#define UART_RX_IDLE_16BYTE                ((uint16_t)(0x04))
#define UART_RX_IDLE_32BYTE                ((uint16_t)(0x05))
#define UART_RX_IDLE_64BYTE                ((uint16_t)(0x06))
#define UART_RX_IDLE_128BYTE               ((uint16_t)(0x07))
#define UART_RX_IDLE_256BYTE               ((uint16_t)(0x08))
#define UART_RX_IDLE_512BYTE               ((uint16_t)(0x09))
#define UART_RX_IDLE_1024BYTE              ((uint16_t)(0x0A))
#define UART_RX_IDLE_2048BYTE              ((uint16_t)(0x0B))
#define UART_RX_IDLE_4096BYTE              ((uint16_t)(0x0C))
#define UART_RX_IDLE_8192BYTE              ((uint16_t)(0x0D))
#define UART_RX_IDLE_16384BYTE             ((uint16_t)(0x0E))
#define UART_RX_IDLE_32768BYTE             ((uint16_t)(0x0F))

#define IS_UART_IDLE_TIME(TIME) ((TIME) <= 0x0F)

/** End of group UART_Rx_idle_time
  * @}
  */

/** @defgroup UART_Parity UART Parity
  * @{
  */

#define UART_PARITY_NO_PARTY                        ((uint16_t)(0x00 << 3))
#define UART_PARITY_ODD                             ((uint16_t)(0x01 << 3))
#define UART_PARITY_EVEN                            ((uint16_t)(0x03 << 3))

#define IS_UART_PARITY(PARITY) (((PARITY) == UART_PARITY_NO_PARTY) || ((PARITY) == UART_PARITY_ODD)\
                                || ((PARITY) == UART_PARITY_EVEN))

/** End of group UART_Parity
  * @}
  */

/** @defgroup UART_DMA UART DMA
  * @{
  */

#define UART_DMA_ENABLE                             ((uint16_t)(1 << 3))
#define UART_DMA_DISABLE                            ((uint16_t)(0 << 3))

#define IS_UART_DMA_CFG(CFG) (((CFG) == UART_DMA_ENABLE) || ((CFG) == UART_DMA_DISABLE))

/** End of group UART_DMA
  * @}
  */

/** @defgroup UART_Hardware_Flow_Control UART Hardware Flow Control
  * @{
  */

#define UART_AUTO_FLOW_CTRL_EN                      ((uint16_t)((1 << 5) | (1 << 1)))
#define UART_AUTO_FLOW_CTRL_DIS                     ((uint16_t)0x00)

#define IS_UART_AUTO_FLOW_CTRL(CTRL) (((CTRL) == UART_AUTO_FLOW_CTRL_EN) || ((CTRL) == UART_AUTO_FLOW_CTRL_DIS))

/** End of group UART_Hardware_Flow_Control
  * @}
  */

/** @defgroup UART_Wrod_Length UART Wrod Length
  * @{
  */

#define UART_WROD_LENGTH_7BIT                       ((uint16_t)(0 << 0))
#define UART_WROD_LENGTH_8BIT                       ((uint16_t)(1 << 0))

#define IS_UART_WORD_LENGTH(LEN) ((((LEN)) == UART_WROD_LENGTH_7BIT) || (((LEN)) == UART_WROD_LENGTH_8BIT))

/** End of group UART_Wrod_Length
  * @}
  */

/** @defgroup UART_Stop_Bits UART Stop Bits
  * @{
  */

#define UART_STOP_BITS_1                           ((uint16_t)(0 << 2))
#define UART_STOP_BITS_2                            ((uint16_t)(1 << 2))

#define IS_UART_STOPBITS(STOP) (((STOP) == UART_STOP_BITS_1) || ((STOP) == UART_STOP_BITS_2))

/** End of group UART_Stop_Bits
  * @}
  */

/** @cond private
  * @defgroup Uart_Tx_Rx_FIFO_CLEAR_BIT Uart TRx Fifo Clear Bits
  * @{
  */

#define FCR_CLEAR_RX_FIFO_Set           ((uint32_t)(1 << 1))
#define FCR_CLEAR_RX_FIFO_Reset         ((uint32_t)~(1 << 1))

#define FCR_CLEAR_TX_FIFO_Set           ((uint32_t)(1 << 2))
#define FCR_CLEAR_TX_FIFO_Reset         ((uint32_t)~(1 << 2))

/**
  * @}
  * @endcond
  */

/** End of group UART_Exported_Constants
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup UART_Exported_Functions UART Exported Functions
  * @{
  */
/**
  * @brief Initializes the UART peripheral according to the specified
  *   parameters in the UART_InitStruct
  * @param  UARTx: selected UART peripheral.
  * @param  UART_InitStruct: pointer to a UART_InitTypeDef structure that
  *   contains the configuration information for the specified UART peripheral
  * @retval None
  */
extern void UART_Init(UART_TypeDef *UARTx, UART_InitTypeDef *UART_InitStruct);

/**
  * @brief  Deinitializes the UART peripheral registers to their default reset values(turn off UART clock).
  * @param  UARTx: selected UART peripheral.
  * @retval None
  */
extern void UART_DeInit(UART_TypeDef *UARTx);

/**
  * @brief  Fills each UART_InitStruct member with its default value.
  * @param  UART_InitStruct: pointer to an UART_InitTypeDef structure which will be initialized.
  * @retval None
  */
extern void UART_StructInit(UART_InitTypeDef *UART_InitStruct);

/**
  * @brief  Receive data from rx FIFO.
  * @param  UARTx: selected UART peripheral.
  * @param[out]  outBuf: buffer to save data read from UART FIFO.
  * @param  count: number of data to be read.
  * @retval None
  */
extern void UART_ReceiveData(UART_TypeDef *UARTx, uint8_t *outBuf, uint16_t count);

/**
  * @brief  Send data to tx FIFO.
  * @param  UARTx: selected UART peripheral.
  * @param  inBuf: buffer to be written to Tx FIFO.
  * @param  count: number of data to be written.
  * @retval None
  */
extern void UART_SendData(UART_TypeDef *UARTx, const uint8_t *inBuf, uint16_t count);

/**
  * @brief  Enables or disables the specified UART interrupts.
  * @param  UARTx: selected UARTx peripheral.
  * @param  UART_IT: specifies the UART interrupts sources to be enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg UART_INT_RD_AVA: enable Rx data avaliable interrupt.
  *     @arg UART_INT_FIFO_EMPTY: enable FIFO empty interrupt.
  *     @arg UART_INT_LINE_STS: enable line status interrupt.
  *     @arg UART_INT_MODEM_STS: enable modem status interrupt.
  * @param  newState: new state of the specified UART interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
extern void UART_INTConfig(UART_TypeDef *UARTx, uint32_t UART_IT, FunctionalState newState);

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
extern FlagStatus UART_GetFlagState(UART_TypeDef *UARTx, uint32_t UART_FLAG);

/**
  *@brief  UART loop back mode config.
  *@param  UARTx: selected UART peripheral.
  *@param  NewState: new state of the DMA Channelx.
  *   This parameter can be: ENABLE or DISABLE.
  *@retval None.
  */
extern void UART_LoopBackCmd(UART_TypeDef *UARTx, FunctionalState NewState);

/**
  *@brief  Change UART baudrate.
  *@param  UARTx: selected UART peripheral.
  *@param  div: parameter of the selected UART baudrate.
  *@param  ovsr: parameter of the selected UART baudrate.
  *@param  ovsr_adj: parameter of the selected UART baudrate.
  *@retval None.
  */
extern void UART_ChangeBaudRate(UART_TypeDef *UARTx, uint16_t div, uint16_t ovsr,
                                uint16_t ovsr_adj);

/**
  *@brief  Change UART parameters.
  *@param  UARTx: selected UART peripheral.
  *@param  parity: parity of selected UART peripheral.
  *@param  stopBits: stop bit of selected UART peripheral.
  *@param  wordLen: data width of selected UART peripheral.
  *@retval None.
  */
extern void UART_ChangeParams(UART_TypeDef *UARTx, uint16_t parity, uint16_t stopBits,
                              uint16_t wordLen);

/**
  * @brief  Send one byte to tx FIFO.
  * @param  UARTx: selected UART peripheral.
  * @param  data: byte to send.
  * @retval None
  */
__STATIC_INLINE void UART_SendByte(UART_TypeDef *UARTx, uint8_t data)
{
    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));

    UARTx->RB_THR = data;

    return;
}

/**
  * @brief  read one byte in rx FIFO.
  * @param  UARTx: selected UART peripheral.
  * @retval the byte read.
  */
__STATIC_INLINE uint8_t UART_ReceiveByte(UART_TypeDef *UARTx)
{
    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));

    return (uint8_t)(UARTx->RB_THR);
}

/**
  * @brief  Get interrupt identifier.
  * @param  UARTx: selected UART peripheral.
  * @retval The interrupt identifier value.
  *   This return value can be one of the following values:
  *     @arg UART_INT_ID_LINE_STATUS: interrupt identifier--line status interrupt.
  *     @arg UART_INT_ID_RX_LEVEL_REACH: interrupt identifier--rx trigger level reached interrupt.
  *     @arg UART_INT_ID_RX_TMEOUT: interrupt identifier--line status interrupt.
  *     @arg UART_INT_ID_TX_EMPTY: interrupt identifier--line status interrupt.
  *     @arg UART_INT_ID_MODEM_STATUS: interrupt identifier--line status interrupt.
  */
__STATIC_INLINE uint16_t UART_GetIID(UART_TypeDef *UARTx)
{
    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));

    return (uint16_t)(UARTx->INTID_FCR & (0x0000000E));
}


/**
  * @brief  Clear UART tx FIFO.
  * @param  UARTx: selected UART peripheral.
  * @retval None
  */
__STATIC_INLINE void UART_ClearTxFifo(UART_TypeDef *UARTx)
{
    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));

    UARTx->INTID_FCR = (((UARTx->STSR & BIT24) >> 21) | ((UARTx->STSR & 0x7C000000) >> 18) | ((
                            UARTx->STSR & BIT25) >> 25) | FCR_CLEAR_TX_FIFO_Set);

    return;
}

/**
  * @brief  Clear UART rx FIFO.
  * @param  UARTx: selected UART peripheral.
  * @retval None
  */
__STATIC_INLINE void UART_ClearRxFifo(UART_TypeDef *UARTx)
{
    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));

    UARTx->INTID_FCR = (((UARTx->STSR & BIT24) >> 21) | ((UARTx->STSR & 0x7C000000) >> 18) | ((
                            UARTx->STSR & BIT25) >> 25) | FCR_CLEAR_RX_FIFO_Set);

    return;
}


/**
  * @brief  read  data length in Tx FIFO through the UARTx peripheral.
  * @param  UARTx: where x can be 0 or 1
  * @retval None
  */
__STATIC_INLINE uint8_t UART_GetTxFIFOLen(UART_TypeDef *UARTx)
{
    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));

    return (uint8_t)(UARTx->FIFO_LEVEL & 0x1F);
}

/**
  * @brief  read data length in Rx FIFO through the UARTx peripheral.
  * @param  UARTx: where x can be 0 or 1
  * @retval None
  */
__STATIC_INLINE uint8_t UART_GetRxFIFOLen(UART_TypeDef *UARTx)
{
    /* Check the parameters */
    assert_param(IS_UART_PERIPH(UARTx));

    return (uint8_t)((UARTx->FIFO_LEVEL >> 8) & 0x3F);
}

#ifdef __cplusplus
}
#endif

#endif /* _RTL876X_UART_H_ */

/** @} */ /* End of group UART_Exported_Functions */
/** @} */ /* End of group UART */

/******************* (C) COPYRIGHT 2015 Realtek Semiconductor *****END OF FILE****/



