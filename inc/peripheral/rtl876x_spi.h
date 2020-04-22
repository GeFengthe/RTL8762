/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_spi.h
* @brief
* @details
* @author    elliot chen
* @date      2015-5-6
* @version   v1.0
* *********************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTL876X_SPI_H
#define __RTL876X_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"
#include "rtl876x_bitfields.h"


/** @addtogroup SPI SPI
  * @brief SPI driver module
  * @{
  */

/*============================================================================*
 *                         Types
 *============================================================================*/


/** @defgroup SPI_Exported_Types SPI Exported Types
  * @{
  */

/**
  * @brief  SPI Init structure definition
  */

typedef struct
{
    uint16_t SPI_Direction;         /*!< Specifies the SPI unidirectional or bidirectional data mode.
                                                    This parameter can be a value of @ref SPI_data_direction */

    uint16_t SPI_Mode;              /*!< Specifies the SPI operating mode.
                                                    This parameter can be a value of @ref SPI_mode */

    uint16_t SPI_DataSize;          /*!< Specifies the SPI data size.
                                                    This parameter can be a value of @ref SPI_data_size */

    uint16_t SPI_CPOL;              /*!< Specifies the serial clock steady state.
                                                    This parameter can be a value of @ref SPI_Clock_Polarity */

    uint16_t SPI_CPHA;              /*!< Specifies the clock active edge for the bit capture.
                                                    This parameter can be a value of @ref SPI_Clock_Phase */

    uint32_t SPI_SwapTxBitEn;              /*!< Specifies whether to swap spi tx data bit */

    uint32_t SPI_SwapRxBitEn;              /*!< Specifies whether to swap spi rx data bit */

    uint32_t SPI_SwapTxByteEn;              /*!< Specifies whether to swap spi tx data bit */

    uint32_t SPI_SwapRxByteEn;              /*!< Specifies whether to swap spi rx data bit */

    uint32_t SPI_ToggleEn;            /*!< Specifies whether to toggle when transfer done */

    uint32_t SPI_BaudRatePrescaler;        /*!< Specifies the speed of SCK clock. SPI Clock Speed = clk source/SPI_ClkDIV
                                                    @note The communication clock is derived from the master
                                                    clock. The slave clock does not need to be set. */
    uint16_t SPI_FrameFormat;       /*!< Specifies which serial protocol transfers the data.
                                                    This parameter can be a value of @ref SPI_frame_format */

    uint32_t SPI_TxThresholdLevel;  /*!<  Specifies the transmit FIFO Threshold */

    uint32_t SPI_RxThresholdLevel;  /*!< Specifies the receive FIFO Threshold */

    uint32_t SPI_NDF;               /*!< Specifies the trigger condition in EEPROM mode.
                                                    This parameter should be the value of the length of read data. */

    uint16_t SPI_TxDmaEn;               /*!< Specifies the Tx dma mode.*/

    uint16_t SPI_RxDmaEn;               /*!< Specifies the Rx dma mode >*/

    uint8_t SPI_TxWaterlevel;           /*!< Specifies the DMA tx water level >*/

    uint8_t SPI_RxWaterlevel;           /*!< Specifies the DMA rx water level >*/

} SPI_InitTypeDef;

/** End of group SPI_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/


/** @defgroup SPI_Exported_Constants SPI Exported Constants
  * @{
  */

#define IS_SPI_ALL_PERIPH(PERIPH) (((PERIPH) == SPI0) || \
                                   ((PERIPH) == SPI1))


/** @defgroup SPI_clock_speed SPI Clock Speed
  * @{
  */

#define IS_SPI_CLOCK_SPEED(SPEED) (((SPEED) >= 0x01) && ((SPEED) <= 40000000))

/** End of group SPI_clock_speed
  * @}
  */

/** @defgroup SPI_data_direction SPI Data Direction
  * @{
  */

#define SPI_Direction_FullDuplex        ((uint16_t)0x0000)
#define SPI_Direction_TxOnly            ((uint16_t)0x0001)
#define SPI_Direction_RxOnly            ((uint16_t)0x0002)
#define SPI_Direction_EEPROM            ((uint16_t)0x0003)

#define IS_SPI_DIRECTION_MODE(MODE) (((MODE) == SPI_Direction_FullDuplex) || \
                                     ((MODE) == SPI_Direction_RxOnly) || \
                                     ((MODE) == SPI_Direction_TxOnly) || \
                                     ((MODE) == SPI_Direction_EEPROM))

/** End of group SPI_data_direction
  * @}
  */

/** @defgroup SPI_mode SPI Mode
  * @{
  */

#define SPI_Mode_Master                         ((uint16_t)0x0104)
#define SPI_Mode_Slave                          ((uint16_t)0x0000)
#define IS_SPI_MODE(MODE) (((MODE) == SPI_Mode_Master) || \
                           ((MODE) == SPI_Mode_Slave))

/** End of group SPI_mode
  * @}
  */

/** @defgroup SPI_data_size SPI Data Size
  * @{
  */

#define SPI_DataSize_4b                 ((uint16_t)0x0003)
#define SPI_DataSize_5b                 ((uint16_t)0x0004)
#define SPI_DataSize_6b                 ((uint16_t)0x0005)
#define SPI_DataSize_7b                 ((uint16_t)0x0006)
#define SPI_DataSize_8b                 ((uint16_t)0x0007)
#define SPI_DataSize_9b                 ((uint16_t)0x0008)
#define SPI_DataSize_10b                 ((uint16_t)0x0009)
#define SPI_DataSize_11b                 ((uint16_t)0x000a)
#define SPI_DataSize_12b                 ((uint16_t)0x000b)
#define SPI_DataSize_13b                 ((uint16_t)0x000c)
#define SPI_DataSize_14b                 ((uint16_t)0x000d)
#define SPI_DataSize_15b                 ((uint16_t)0x000e)
#define SPI_DataSize_16b                 ((uint16_t)0x000f)
#define SPI_DataSize_17b                 ((uint16_t)0x0010)
#define SPI_DataSize_18b                 ((uint16_t)0x0011)
#define SPI_DataSize_19b                 ((uint16_t)0x0012)
#define SPI_DataSize_20b                 ((uint16_t)0x0013)
#define SPI_DataSize_21b                 ((uint16_t)0x0014)
#define SPI_DataSize_22b                 ((uint16_t)0x0015)
#define SPI_DataSize_23b                 ((uint16_t)0x0016)
#define SPI_DataSize_24b                 ((uint16_t)0x0017)
#define SPI_DataSize_25b                 ((uint16_t)0x0018)
#define SPI_DataSize_26b                 ((uint16_t)0x0019)
#define SPI_DataSize_27b                 ((uint16_t)0x001A)
#define SPI_DataSize_28b                 ((uint16_t)0x001B)
#define SPI_DataSize_29b                 ((uint16_t)0x001C)
#define SPI_DataSize_30b                 ((uint16_t)0x001D)
#define SPI_DataSize_31b                 ((uint16_t)0x001E)
#define SPI_DataSize_32b                 ((uint16_t)0x001F)
#define IS_SPI_DATASIZE(DATASIZE) (((DATASIZE) == SPI_DataSize_4b)  || \
                                   ((DATASIZE) == SPI_DataSize_5b)  || \
                                   ((DATASIZE) == SPI_DataSize_6b)  || \
                                   ((DATASIZE) == SPI_DataSize_7b)  || \
                                   ((DATASIZE) == SPI_DataSize_8b)  || \
                                   ((DATASIZE) == SPI_DataSize_9b)  || \
                                   ((DATASIZE) == SPI_DataSize_10b) || \
                                   ((DATASIZE) == SPI_DataSize_11b) || \
                                   ((DATASIZE) == SPI_DataSize_12b) || \
                                   ((DATASIZE) == SPI_DataSize_13b) || \
                                   ((DATASIZE) == SPI_DataSize_14b) || \
                                   ((DATASIZE) == SPI_DataSize_15b) || \
                                   ((DATASIZE) == SPI_DataSize_16b) || \
                                   ((DATASIZE) == SPI_DataSize_17b) || \
                                   ((DATASIZE) == SPI_DataSize_18b) || \
                                   ((DATASIZE) == SPI_DataSize_19b) || \
                                   ((DATASIZE) == SPI_DataSize_20b) || \
                                   ((DATASIZE) == SPI_DataSize_21b) || \
                                   ((DATASIZE) == SPI_DataSize_22b) || \
                                   ((DATASIZE) == SPI_DataSize_23b) || \
                                   ((DATASIZE) == SPI_DataSize_24b) || \
                                   ((DATASIZE) == SPI_DataSize_25b) || \
                                   ((DATASIZE) == SPI_DataSize_26b) || \
                                   ((DATASIZE) == SPI_DataSize_27b) || \
                                   ((DATASIZE) == SPI_DataSize_28b) || \
                                   ((DATASIZE) == SPI_DataSize_29b) || \
                                   ((DATASIZE) == SPI_DataSize_30b) || \
                                   ((DATASIZE) == SPI_DataSize_31b) || \
                                   ((DATASIZE) == SPI_DataSize_32b))

/** End of group SPI_data_size
  * @}
  */

/** @defgroup SPI_BaudRate_Prescaler_
  * @{
  */

#define SPI_BaudRatePrescaler_2         ((uint32_t)0x0002)
#define SPI_BaudRatePrescaler_4         ((uint32_t)0x0004)
#define SPI_BaudRatePrescaler_6         ((uint32_t)0x0006)
#define SPI_BaudRatePrescaler_8         ((uint32_t)0x0008)
#define SPI_BaudRatePrescaler_10        ((uint32_t)0x000A)
#define SPI_BaudRatePrescaler_12        ((uint32_t)0x000C)
#define SPI_BaudRatePrescaler_14        ((uint32_t)0x000E)
#define SPI_BaudRatePrescaler_16        ((uint32_t)0x0010)
#define SPI_BaudRatePrescaler_32        ((uint32_t)0x0020)
#define SPI_BaudRatePrescaler_64        ((uint32_t)0x0040)
#define SPI_BaudRatePrescaler_128       ((uint32_t)0x0080)
#define SPI_BaudRatePrescaler_256       ((uint32_t)0x0100)
#define IS_SPI_BAUDRATE_PRESCALER(PRESCALER) (((PRESCALER) == SPI_BaudRatePrescaler_2) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_4) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_8) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_10) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_12) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_14) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_16) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_32) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_64) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_128) || \
                                              ((PRESCALER) == SPI_BaudRatePrescaler_256))
/**
  * @}
  */

/** @defgroup SPI_Clock_Polarity SPI Clock Polarity
  * @{
  */

#define SPI_CPOL_Low                    ((uint16_t)0x0000)
#define SPI_CPOL_High                   ((uint16_t)0x0001)
#define IS_SPI_CPOL(CPOL) (((CPOL) == SPI_CPOL_Low) || \
                           ((CPOL) == SPI_CPOL_High))

/** End of group SPI_Clock_Polarity
  * @}
  */

/** @defgroup SPI_Clock_Phase SPI Clock Phase
  * @{
  */

#define SPI_CPHA_1Edge                  ((uint16_t)0x0000)
#define SPI_CPHA_2Edge                  ((uint16_t)0x0001)
#define IS_SPI_CPHA(CPHA) (((CPHA) == SPI_CPHA_1Edge) || \
                           ((CPHA) == SPI_CPHA_2Edge))

/** End of group SPI_Clock_Phase
  * @}
  */

/** @defgroup SPI_frame_format SPI Frame Format
  * @{
  */

#define SPI_Frame_Motorola              ((uint16_t)0x0000)
#define SPI_Frame_TI_SSP                ((uint16_t)0x0001)
#define SPI_Frame_NS_MICROWIRE          ((uint16_t)0x0002)
#define SPI_Frame_Reserve               ((uint16_t)0x0003)
#define IS_SPI_FRAME_FORMAT(FRAME) (((FRAME) == SPI_Frame_Motorola) || \
                                    ((FRAME) == SPI_Frame_TI_SSP) || \
                                    ((FRAME) == SPI_Frame_NS_MICROWIRE) || \
                                    ((FRAME) == SPI_Frame_Reserve))

/** End of group SPI_frame_format
  * @}
  */

/** @defgroup SPI_flags_definition SPI Flags Definition
  * @{
  */

#define SPI_FLAG_BUSY                   ((uint16_t)0x0001)
#define SPI_FLAG_TFNF                   ((uint16_t)0x0002)
#define SPI_FLAG_TFE                    ((uint16_t)0x0004)
#define SPI_FLAG_RFNE                   ((uint16_t)0x0008)
#define SPI_FLAG_RFF                    ((uint16_t)0x0010)
#define SPI_FLAG_TXE                    ((uint16_t)0x0020)
#define SPI_FLAG_DCOL                   ((uint16_t)0x0040)
#define IS_SPI_GET_FLAG(FLAG)   (((FLAG) == SPI_FLAG_DCOL) || ((FLAG) == SPI_FLAG_TXE) || \
                                 ((FLAG) == SPI_FLAG_RFF) || ((FLAG) == SPI_FLAG_RFNE) || \
                                 ((FLAG) == SPI_FLAG_TFE) || ((FLAG) == SPI_FLAG_TFNF) || \
                                 ((FLAG) == SPI_FLAG_BUSY))

/** End of group SPI_flags_definition
  * @}
  */

/** @defgroup SPI_interrupts_definition SPI Interrupts Definition
  * @{
  */

#define SPI_INT_TXE                  ((uint8_t)BIT(0))
#define SPI_INT_TXO                  ((uint8_t)BIT(1))
#define SPI_INT_RXU                  ((uint8_t)BIT(2))
#define SPI_INT_RXO                  ((uint8_t)BIT(3))
#define SPI_INT_RXF                  ((uint8_t)BIT(4))
#define SPI_INT_MST                  ((uint8_t)BIT(5))
#define SPI_INT_TUF                  ((uint8_t)BIT(6))
#define SPI_INT_RIG                  ((uint8_t)BIT(7))

#define IS_SPI_CONFIG_IT(IT) (((IT) == SPI_INT_TXE) || \
                              ((IT) == SPI_INT_TXO) || \
                              ((IT) == SPI_INT_RXU) || \
                              ((IT) == SPI_INT_RXO) || \
                              ((IT) == SPI_INT_RXF) || \
                              ((IT) == SPI_INT_MST) || \
                              ((IT) == SPI_INT_TUF) || \
                              ((IT) == SPI_INT_RIG) )
/**
  * @}
  */

/** @defgroup SPI_GDMA_transfer_requests  SPI GDMA transfer requests
  * @{
  */

#define SPI_GDMAReq_Tx               ((uint16_t)0x0002)
#define SPI_GDMAReq_Rx               ((uint16_t)0x0001)
#define IS_SPI_GDMAREQ(GDMAREQ) ((((GDMAREQ) & (uint16_t)0xFFFC) == 0x00) && ((GDMAREQ) != 0x00))

/** End of group SPI_GDMA_transfer_requests
  * @}
  */

/** @defgroup SPI_GDMA_transfer_requests  SPI GDMA transfer requests
  * @{
  */

#define SPI_SWAP_ENABLE                ((uint32_t)0x0001)
#define SPI_SWAP_DISABLE               ((uint32_t)0x0000)
#define IS_SPI_SWAPMODE(mode) (((mode) == SPI_SWAP_ENABLE) || \
                               ((mode) == SPI_SWAP_DISABLE))

/** End of group SPI_GDMA_transfer_requests
  * @}
  */

/** End of group SPI_Exported_Constants
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup SPI_Exported_functions SPI Exported Functions
 * @{
 */
/**
  * @brief  Deinitializes the SPIx peripheral registers to their default reset values.
  * @param  SPIx: where x can be 0 or 1 to select the SPI peripheral.
  * @retval None
  */

void SPI_DeInit(SPI_TypeDef *SPIx);

/**
  * @brief  Initializes the SPIx peripheral according to the specified
  *   parameters in the SPI_InitStruct.
  * @param  SPIx: where x can be 0 or 1 to select the SPI peripheral.
  * @param  SPI_InitStruct: pointer to a SPI_InitTypeDef structure that
  *   contains the configuration information for the specified SPI peripheral.
  * @retval None
  */

void SPI_Init(SPI_TypeDef *SPIx, SPI_InitTypeDef *SPI_InitStruct);

/**
  * @brief  Fills each SPI_InitStruct member with its default value.
  * @param  SPI_InitStruct : pointer to a SPI_InitTypeDef structure which will be initialized.
  * @retval None
  */
void SPI_StructInit(SPI_InitTypeDef *SPI_InitStruct);

/**
  * @brief  Enables or disables the specified SPI peripheral.
  * @param  SPIx: where x can be 0 or 1 to select the SPI peripheral.
  * @param  NewState: new state of the SPIx peripheral.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SPI_Cmd(SPI_TypeDef *SPIx, FunctionalState NewState);

/**
 * @brief  Transmits a number of bytes through the SPIx peripheral.
 * @param  SPIx: where x can be 0 or 1
 * @param  pBuf : bytes to be transmitted.
 * @param  len: byte length to be transmitted
 * @retval None
 */
void SPI_SendBuffer(SPI_TypeDef *SPIx, uint8_t *pBuf, uint16_t len);

/**
  * @brief  Transmits a number of words through the SPIx peripheral.
  * @param  SPIx: where x can be 0 or 1
  * @param  pBuf : words to be transmitted.
  * @param  len: word length to be transmitted
  * @retval None
  */
void SPI_SendWord(SPI_TypeDef *SPIx, uint32_t *pBuf, uint16_t len);

/**
   * @brief  Transmits a number of halfWords through the SPIx peripheral.
   * @param  SPIx: where x can be 0 or 1
   * @param  pBuf : Halfwords to be transmitted.
   * @param  len: Halfwords length to be transmitted
   * @retval None
   */
void SPI_SendHalfWord(SPI_TypeDef *SPIx, uint16_t *pBuf, uint16_t len);

/**
  * @brief  Enables or disables the specified SPI/I2S interrupts.
  * @param  SPIx: where x can be 0 or 1
  * @param  SPI_IT: specifies the SPI/I2S interrupt source to be enabled or disabled.
  *   This parameter can be one of the following values:
  *     @arg SPI_INT_TXE: Tx buffer empty interrupt mask
  *     @arg SPI_INT_TXO: Tx buffer overflow interrupt mask
  *     @arg SPI_INT_RXU: receive FIFO Underflow Interrupt mask
  *     @arg SPI_INT_RXO: receive FIFO Overflow Interrupt mask
  *     @arg SPI_INT_RXF: receive FIFO Full Interrupt mask which equal RXNE Interrupt!!!
  *     @arg SPI_INT_MST: multi-Master Contention Interrupt mask
  * @param  NewState: new state of the specified SPI interrupt.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SPI_INTConfig(SPI_TypeDef *SPIx, uint8_t SPI_IT, FunctionalState NewState);

/**
  * @brief  Clear the specified SPI interrupt.
  * @param  SPIx: where x can be 0 or 1
  * @param  SPI_IT: specifies the SPI interrupt to clear.
  *   This parameter can be one of the following values:
  *     @arg SPI_INT_MST: Multi-Master Contention Interrupt.
  *     @arg SPI_INT_RXO: Receive FIFO Overflow Interrupt.
  *     @arg SPI_INT_RXU: Receive FIFO Underflow Interrupt.
  *     @arg SPI_INT_TXO: Transmit FIFO Overflow Interrupt .
  * @retval None.
  */
void SPI_ClearINTPendingBit(SPI_TypeDef *SPIx, uint16_t SPI_IT);

/**
  * @brief  Transmits a Data through the SPIx peripheral.
  * @param  SPIx: where x can be 0 or 1
  * @param  Data : Data to be transmitted.
  * @retval None
  */
__STATIC_INLINE void SPI_SendData(SPI_TypeDef *SPIx, uint32_t Data)
{
    /* Check the parameters */
    assert_param(IS_SPI_ALL_PERIPH(SPIx));

    SPIx->DR[0] = Data;
    while (!(SPIx->SR & BIT(1)));
}

/**
  * @brief  Returns the most recent received data by the SPIx/I2Sx peripheral.
  * @param  SPIx: where x can be 0 or 1
  * @retval The value of the received data.
  */
__STATIC_INLINE uint32_t SPI_ReceiveData(SPI_TypeDef *SPIx)
{
    /* Check the parameters */
    assert_param(IS_SPI_ALL_PERIPH(SPIx));

    return (uint32_t)SPIx->DR[0];
}

/**
  * @brief  read  data length in Rx FIFO through the SPIx peripheral.
  * @param  SPIx: where x can be 0 or 1
  * @retval None
  */
__STATIC_INLINE uint8_t SPI_GetRxFIFOLen(SPI_TypeDef *SPIx)
{
    /* Check the parameters */
    assert_param(IS_SPI_ALL_PERIPH(SPIx));
    return (uint8_t)SPIx->RXFLR;
}

/**
  * @brief  read data length in Tx FIFO through the SPIx peripheral.
  * @param  SPIx: where x can be 0 or 1
  * @retval None
  */
__STATIC_INLINE uint8_t SPI_GetTxFIFOLen(SPI_TypeDef *SPIx)
{
    /* Check the parameters */
    assert_param(IS_SPI_ALL_PERIPH(SPIx));

    return (uint8_t)SPIx->TXFLR;
}

/**
 *@brief change SPI direction mode.
 *@param SPIx: where x can be 0 or 1
 *@param dir: parameter of direction mode
 *@retval None
 */
__STATIC_INLINE void SPI_ChangeDirection(SPI_TypeDef *SPIx, uint16_t dir)
{
    /* Check the parameters */
    assert_param(IS_SPI_ALL_PERIPH(SPIx));
    assert_param(IS_SPI_DIRECTION_MODE(dir));

    /* Disable the selected SPI peripheral */
    SPIx->SSIENR &= ~0x01;

    /* Change SPI direction mode */
    SPIx->CTRLR0 &= ~(0x03 << 8);
    SPIx->CTRLR0 |= dir << 8;

    /* Enable the selected SPI peripheral */
    SPIx->SSIENR |= 0x01;
}

/**
  * @brief  set read Data length only in EEPROM mode through the SPIx peripheral,which
    enables you to receive up to 64 KB of data in a continuous transfer.
  * @param  SPIx: where x can be 0 or 1
  * @param  len : length of read data which can be 1 to 65536.
  * @retval None
  */
__STATIC_INLINE void SPI_SetReadLen(SPI_TypeDef *SPIx, uint16_t len)
{
    /* Check the parameters */
    assert_param(IS_SPI_ALL_PERIPH(SPIx));

    /* Disable the selected SPI peripheral */
    SPIx->SSIENR &= ~0x01;
    /* set read length in SPI EEPROM mode */
    SPIx->CTRLR1 = len - 1;
    /* Enable the selected SPI peripheral */
    SPIx->SSIENR |= 0x01;
}

/**
  * @brief  set cs number through the SPIx peripheral.
  * @param  SPIx: where x can be 0 or 1
  * @param  number: if SPIx is SPI0, number must be 0. if SPIx is SPI1, number can be 0 to 2.
  * @retval None
  */
__STATIC_INLINE void SPI_SetCSNumber(SPI_TypeDef *SPIx, uint8_t number)
{
    /* Check the parameters */
    assert_param(IS_SPI_ALL_PERIPH(SPIx));

    /* set cs number */
    SPIx->SER = BIT(number);
}

/**
  * @brief  Checks whether the specified SPI interrupt is set or not.
  * @param  SPIx: where x can be 0 or 1
  * @param  SPI_IT: specifies the SPI interrupt to check.
  *   This parameter can be one of the following values:
  *     @arg SPI_INT_MST: Multi-Master Contention Interrupt.
  *     @arg SPI_INT_RXF: Receive FIFO Full Interrupt.
  *     @arg SPI_INT_RXO: Receive FIFO Overflow Interrupt.
  *     @arg SPI_INT_RXU: Receive FIFO Underflow Interrupt.
  *     @arg SPI_INT_TXO: Transmit FIFO Overflow Interrupt .
  *     @arg SPI_INT_TXE: Transmit FIFO Empty Interrupt.
  * @retval The new state of SPI_IT (SET or RESET).
  */
__STATIC_INLINE ITStatus SPI_GetINTStatus(SPI_TypeDef *SPIx, uint32_t SPI_IT)
{
    ITStatus bit_status = RESET;

    /* Check the parameters */
    assert_param(IS_SPI_ALL_PERIPH(SPIx));
    assert_param(IS_SPI_CONFIG_IT(SPI_IT));

    if ((SPIx->ISR & SPI_IT) != (uint32_t)RESET)
    {
        bit_status = SET;
    }

    /* Return the SPI_IT status */
    return  bit_status;
}

/**
  * @brief  Checks whether the specified SPI flag is set or not.
  * @param  SPIx: where x can be 0 or 1
  * @param  SPI_FLAG: specifies the SPI flag to check.
  *   This parameter can be one of the following values:
  *     @arg SPI_FLAG_DCOL: Data Collision Error flag.Set if it is actively transmitting in master mode when another master selects this device as a slave.
  *     @arg SPI_FLAG_TXE: Transmission error flag.Set if the transmit FIFO is empty when a transfer is started in slave mode.
  *     @arg SPI_FLAG_RFF: Receive FIFO full flag. Set if the receive FIFO is completely full.
  *     @arg SPI_FLAG_RFNE: Receive FIFO Not Empty flag.Set if receive FIFO is not empty.
  *     @arg SPI_FLAG_TFE: Transmit FIFO Empty flag.Set if transmit FIFO is empty.
  *     @arg SPI_FLAG_TFNF: Transmit FIFO Not Full flag.Set if transmit FIFO is not full.
  *     @arg SPI_FLAG_BUSY: SPI Busy flag.Set if it is actively transferring data.reset if it is idle or disabled.
  * @retval The new state of SPI_FLAG (SET or RESET).
  */
__STATIC_INLINE FlagStatus SPI_GetFlagState(SPI_TypeDef *SPIx, uint8_t SPI_FLAG)
{
    FlagStatus bitstatus = RESET;

    /* Check the parameters */
    assert_param(IS_SPI_ALL_PERIPH(SPIx));
    assert_param(IS_SPI_GET_FLAG(SPI_FLAG));

    /* Check the status of the specified SPI flag */
    if ((SPIx->SR & SPI_FLAG) != (uint8_t)RESET)
    {
        /* SPI_FLAG is set */
        bitstatus = SET;
    }

    /* Return the SPI_FLAG status */
    return  bitstatus;
}

/**
  * @brief  Enables or disables the SPIx GDMA interface.
  * @param  SPIx: where x can be 0 or 1
  * @param  SPI_GDMAReq: specifies the SPI GDMA transfer request to be enabled or disabled.
  *   This parameter can be one of the following values:
  *     @arg SPI_GDMAReq_Tx: Tx buffer DMA transfer request
  *     @arg SPI_GDMAReq_Rx: Rx buffer DMA transfer request
  * @param  NewState: new state of the selected SPI GDMA transfer request.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
__STATIC_INLINE void SPI_GDMACmd(SPI_TypeDef *SPIx, uint16_t SPI_GDMAReq, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_SPI_ALL_PERIPH(SPIx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));
    assert_param(IS_SPI_GDMAREQ(SPI_GDMAReq));

    if (NewState != DISABLE)
    {
        /* Enable the selected SPI GDMA request */
        SPIx->DMACR |= SPI_GDMAReq;
    }
    else
    {
        /* Disable the selected SPI GDMA request */
        SPIx->DMACR &= (uint16_t)~(SPI_GDMAReq);
    }
}
/**
  * @brief  Change SPi speed daynamic add by howie
  * @param  SPIx: where x can be 0 or 1
  * @param  precalser: value of prescaler
  * @retval None
  */
__STATIC_INLINE void SPI_Change_CLK(SPI_TypeDef *SPIx, uint32_t prescaler)
{
    SPIx->BAUDR = prescaler % 0xFFFF;
}


#ifdef __cplusplus
}
#endif

#endif /*__RTL876X_SPI_H*/

/** @} */ /* End of group SPI_Exported_Functions */
/** @} */ /* End of group SPI */

/******************* (C) COPYRIGHT 2015 Realtek Semiconductor Corporation *****END OF FILE****/

