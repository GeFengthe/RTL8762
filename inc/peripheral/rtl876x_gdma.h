/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_gdma.h
* @brief
* @details
* @author    elliot chen
* @date      2015-05-08
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __RTL876X_GDMA_H
#define __RTL876X_GDMA_H



#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"

/** @addtogroup GDMA GDMA
  * @brief GDMA driver module
  * @{
  */

/*============================================================================*
 *                         Types
 *============================================================================*/


/** @defgroup GDMA_Exported_Types GDMA Exported Types
  * @{
  */

/**
  * @brief  GDMA Init structure definition
  */
typedef struct
{
    uint8_t GDMA_ChannelNum;         /*!< Specifies channel number for GDMA. */

    uint8_t GDMA_DIR;                /*!< Specifies if the peripheral is the source or destination.
                                                    This parameter can be a value of @ref GDMA_data_transfer_direction */

    uint32_t GDMA_BufferSize;        /*!< Specifies the buffer size(<=4095), in data unit, of the specified Channel.
                                                    The data unit is equal to the configuration set in DMA_PeripheralDataSize
                                                    or DMA_MemoryDataSize members depending in the transfer direction. */

    uint8_t GDMA_SourceInc;          /*!< Specifies whether the source address register is incremented or not.
                                                    This parameter can be a value of @ref GDMA_source_incremented_mode */

    uint8_t GDMA_DestinationInc;     /*!< Specifies whether the destination address register is incremented or not.
                                                    This parameter can be a value of @ref GDMA_destination_incremented_mode */

    uint32_t GDMA_SourceDataSize;    /*!< Specifies the source data width.
                                                    This parameter can be a value of @ref GDMA_data_size */

    uint32_t GDMA_DestinationDataSize;/*!< Specifies the Memory data width.
                                                    This parameter can be a value of @ref GDMA_data_size */

    uint32_t GDMA_SourceMsize;      /*!< Specifies the number of data items to be transferred.
                                                    This parameter can be a value of @ref GDMA_Msize */

    uint32_t GDMA_DestinationMsize; /*!< Specifies  the number of data items to be transferred.
                                                    This parameter can be a value of @ref GDMA_Msize */

    uint32_t GDMA_SourceAddr;       /*!< Specifies the source base address for GDMA Channelx. */

    uint32_t GDMA_DestinationAddr;  /*!< Specifies the destination base address for GDMA Channelx. */

    uint32_t GDMA_ChannelPriority;   /*!< Specifies the software priority for the GDMA Channelx. */

    uint32_t GDMA_Multi_Block_Struct; /*!< Pointer to the first struct of LLI. */

    uint8_t  GDMA_Multi_Block_En;           /*!< Enable or disable Multi_block function. */

    uint8_t  GDMA_Scatter_En;                   /*!< Enable or disable Scatter function. */

    uint8_t  GDMA_Gather_En;                    /*!< Enable or disable Gather function. NOTE:4 bytes ALIGN.*/

    uint32_t GDMA_GatherCount;              /*!< Specifies the GatherCount.NOTE:4 bytes ALIGN.*/

    uint32_t GDMA_GatherInterval;           /*!< Specifies the GatherInterval. */

    uint32_t GDMA_ScatterCount;             /*!< Specifies the ScatterCount. */

    uint32_t GDMA_ScatterInterval;      /*!< Specifies the ScatterInterval. */

    uint32_t GDMA_Multi_Block_Mode;      /*!< Specifies the multi block transfer mode.
                                                        This parameter can be a value of @ref GDMA_Multiblock_Mode */

    uint8_t  GDMA_SourceHandshake;       /*!< Specifies the handshake index in source.
                                                        This parameter can be a value of @ref GDMA_Handshake_Type */

    uint8_t  GDMA_DestHandshake;          /*!< Specifies the handshake index in Destination.
                                                        This parameter can be a value of @ref GDMA_Handshake_Type */

} GDMA_InitTypeDef;

/**
  * @brief  GDMA Link List Item structure definition
  */
typedef struct
{
    __IO uint32_t SAR;
    __IO uint32_t DAR;
    __IO uint32_t LLP;
    __IO uint32_t CTL_LOW;
    __IO uint32_t CTL_HIGH;
} GDMA_LLIDef;


/** End of Group GDMA_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/


/** @defgroup GDMA_Exported_Constants GDMA Exported Constants
  * @{
  */

#define IS_GDMA_ALL_PERIPH(PERIPH) (((PERIPH) == GDMA_Channel0) || \
                                    ((PERIPH) == GDMA_Channel1) || \
                                    ((PERIPH) == GDMA_Channel2) || \
                                    ((PERIPH) == GDMA_Channel3) || \
                                    ((PERIPH) == GDMA_Channel4) || \
                                    ((PERIPH) == GDMA_Channel5))
#define IS_GDMA_ChannelNum(NUM) ((NUM) < 8)


/** @defgroup GDMA_data_size GDMA Data Size
  * @{
  */

#define GDMA_DataSize_Byte            ((uint32_t)0x00000000)
#define GDMA_DataSize_HalfWord        ((uint32_t)0x00000001)
#define GDMA_DataSize_Word            ((uint32_t)0x00000002)
#define IS_GDMA_DATA_SIZE(SIZE) (((SIZE) == GDMA_DataSize_Byte) || \
                                 ((SIZE) == GDMA_DataSize_HalfWord) || \
                                 ((SIZE) == GDMA_DataSize_Word))

/** End of Group GDMA_data_size
  * @}
  */

/** @defgroup GDMA_Msize GDMA Msize
  * @{
  */

#define GDMA_Msize_1            ((uint32_t)0x00000000)
#define GDMA_Msize_4            ((uint32_t)0x00000001)
#define GDMA_Msize_8            ((uint32_t)0x00000002)
#define GDMA_Msize_16           ((uint32_t)0x00000003)
#define GDMA_Msize_32           ((uint32_t)0x00000004)
#define GDMA_Msize_64           ((uint32_t)0x00000005)
#define GDMA_Msize_128          ((uint32_t)0x00000006)
#define GDMA_Msize_256          ((uint32_t)0x00000007)
#define IS_GDMA_MSIZE(SIZE) (((SIZE) == GDMA_Msize_1) || \
                             ((SIZE) == GDMA_Msize_4) || \
                             ((SIZE) == GDMA_Msize_8) || \
                             ((SIZE) == GDMA_Msize_16) || \
                             ((SIZE) == GDMA_Msize_32) || \
                             ((SIZE) == GDMA_Msize_64) || \
                             ((SIZE) == GDMA_Msize_128) || \
                             ((SIZE) == GDMA_Msize_256))

/** End of Group GDMA_Msize
  * @}
  */

/** @defgroup GDMA_Handshake_Type GDMA Handshake Type
  * @{
  */
#define GDMA_Handshake_UART0_TX          (0)
#define GDMA_Handshake_UART0_RX          (1)
#define GDMA_Handshake_UART2_TX          (2)
#define GDMA_Handshake_UART2_RX          (3)
#define GDMA_Handshake_SPI0_TX           (4)
#define GDMA_Handshake_SPI0_RX           (5)
#define GDMA_Handshake_SPI1_TX           (6)
#define GDMA_Handshake_SPI1_RX           (7)
#define GDMA_Handshake_I2C0_TX           (8)
#define GDMA_Handshake_I2C0_RX           (9)
#define GDMA_Handshake_I2C1_TX           (10)
#define GDMA_Handshake_I2C1_RX           (11)
#define GDMA_Handshake_ADC               (12)
#define GDMA_Handshake_AES_TX            (13)
#define GDMA_Handshake_AES_RX            (14)
#define GDMA_Handshake_UART1_TX          (15)
#define GDMA_Handshake_SPORT0_TX         (16)
#define GDMA_Handshake_SPORT0_RX         (17)
#define GDMA_Handshake_SPORT1_TX         (18)
#define GDMA_Handshake_SPORT1_RX         (19)
#define GDMA_Handshake_SPIC_TX           (20)
#define GDMA_Handshake_SPIC_RX           (21)
#define GDMA_Handshake_LCD               (22)
#define GDMA_Handshake_UART1_RX          (23)
#define GDMA_Handshake_TIM0              (24)
#define GDMA_Handshake_TIM1              (25)
#define GDMA_Handshake_TIM2              (26)
#define GDMA_Handshake_TIM3              (27)
#define GDMA_Handshake_TIM4              (28)
#define GDMA_Handshake_TIM5              (29)
#define GDMA_Handshake_TIM6              (30)
#define GDMA_Handshake_TIM7              (31)


#define IS_GDMA_TransferType(Type) (((Type) == GDMA_Handshake_UART0_TX) || \
                                    ((Type) == GDMA_Handshake_UART0_RX) || \
                                    ((Type) == GDMA_Handshake_UART2_TX) || \
                                    ((Type) == GDMA_Handshake_UART2_RX) || \
                                    ((Type) == GDMA_Handshake_SPI0_TX) || \
                                    ((Type) == GDMA_Handshake_SPI0_RX) || \
                                    ((Type) == GDMA_Handshake_SPI1_TX) || \
                                    ((Type) == GDMA_Handshake_SPI1_RX) || \
                                    ((Type) == GDMA_Handshake_I2C0_TX) || \
                                    ((Type) == GDMA_Handshake_I2C0_RX) || \
                                    ((Type) == GDMA_Handshake_I2C1_TX) || \
                                    ((Type) == GDMA_Handshake_I2C1_RX) || \
                                    ((Type) == GDMA_Handshake_ADC) || \
                                    ((Type) == GDMA_Handshake_AES_TX) || \
                                    ((Type) == GDMA_Handshake_AES_RX) || \
                                    ((Type) == GDMA_Handshake_UART1_TX) || \
                                    ((Type) == GDMA_Handshake_SPORT0_TX) || \
                                    ((Type) == GDMA_Handshake_SPORT0_RX) || \
                                    ((Type) == GDMA_Handshake_SPORT1_TX) || \
                                    ((Type) == GDMA_Handshake_SPORT1_RX) || \
                                    ((Type) == GDMA_Handshake_SPIC_TX) || \
                                    ((Type) == GDMA_Handshake_SPIC_RX) ||\
                                    ((Type) == GDMA_Handshake_LCD) ||\
                                    ((Type) == GDMA_Handshake_TIM0)||\
                                    ((Type) == GDMA_Handshake_TIM1)||\
                                    ((Type) == GDMA_Handshake_TIM2)||\
                                    ((Type) == GDMA_Handshake_TIM3)||\
                                    ((Type) == GDMA_Handshake_TIM4)||\
                                    ((Type) == GDMA_Handshake_TIM5)||\
                                    ((Type) == GDMA_Handshake_TIM6)||\
                                    ((Type) == GDMA_Handshake_TIM7))

/** End of Group GDMA_Handshake_Type
  * @}
  */

/** @defgroup GDMA_data_transfer_direction GDMA Data Transfer Direction
  * @{
  */

#define GDMA_DIR_MemoryToMemory              ((uint32_t)0x00000000)
#define GDMA_DIR_MemoryToPeripheral          ((uint32_t)0x00000001)
#define GDMA_DIR_PeripheralToMemory          ((uint32_t)0x00000002)
#define GDMA_DIR_PeripheralToPeripheral      ((uint32_t)0x00000003)

#define IS_GDMA_DIR(DIR) (((DIR) == GDMA_DIR_MemoryToMemory) || \
                          ((DIR) == GDMA_DIR_MemoryToPeripheral) || \
                          ((DIR) == GDMA_DIR_PeripheralToMemory) ||\
                          ((DIR) == GDMA_DIR_PeripheralToPeripheral))

/** End of Group GDMA_data_transfer_direction
  * @}
  */

/** @defgroup GDMA_source_incremented_mode GDMA Source Incremented Mode
  * @{
  */

#define DMA_SourceInc_Inc          ((uint32_t)0x00000000)
#define DMA_SourceInc_Dec          ((uint32_t)0x00000001)
#define DMA_SourceInc_Fix          ((uint32_t)0x00000002)

#define IS_GDMA_SourceInc(STATE) (((STATE) == DMA_SourceInc_Inc) || \
                                  ((STATE) == DMA_SourceInc_Dec) || \
                                  ((STATE) == DMA_SourceInc_Fix))

/** End of Group GDMA_source_incremented_mode
  * @}
  */

/** @defgroup GDMA_destination_incremented_mode GDMA Destination Incremented Mode
  * @{
  */

#define DMA_DestinationInc_Inc          ((uint32_t)0x00000000)
#define DMA_DestinationInc_Dec          ((uint32_t)0x00000001)
#define DMA_DestinationInc_Fix          ((uint32_t)0x00000002)

#define IS_GDMA_DestinationInc(STATE) (((STATE) == DMA_DestinationInc_Inc) || \
                                       ((STATE) == DMA_DestinationInc_Dec) || \
                                       ((STATE) == DMA_DestinationInc_Fix))

/** End of Group GDMA_destination_incremented_mode
  * @}
  */

/** @defgroup DMA_interrupts_definition DMA Interrupts Definition
  * @{
  */

#define GDMA_INT_Transfer                               ((uint32_t)0x00000001)
#define GDMA_INT_Block                                  ((uint32_t)0x00000002)
#define GDMA_INT_SrcTransfer                            ((uint32_t)0x00000004)
#define GDMA_INT_DstTransfer                            ((uint32_t)0x00000008)
#define GDMA_INT_Error                                  ((uint32_t)0x000000010)
#define IS_GDMA_CONFIG_IT(IT) ((((IT) & 0xFFFFFE00) == 0x00) && ((IT) != 0x00))

/** End of Group DMA_interrupts_definition
  * @}
  */

/** @defgroup DMA_interrupts_definition DMA Interrupts Definition
  * @{
  */

#define GDMA_SUSPEND_TRANSMISSSION                  (BIT(8))
#define GDMA_FIFO_STATUS                            (BIT(9))
#define GDMA_SUSPEND_CHANNEL_STATUS                 (BIT(0))
#define GDMA_SUSPEND_CMD_STATUS                     (BIT(2) | BIT(1))

/** End of Group DMA_interrupts_definition
  * @}
  */

/** @defgroup GDMA_Multiblock_Mode GDMA Multi-block Mode
  * @{
  */

#define AUTO_RELOAD_WITH_CONTIGUOUS_SAR                            (BIT30)
#define AUTO_RELOAD_WITH_CONTIGUOUS_DAR                            (BIT31)
#define AUTO_RELOAD_TRANSFER                                       (BIT30 | BIT31)
#define LLI_WITH_CONTIGUOUS_SAR                                    (BIT27)
#define LLI_WITH_AUTO_RELOAD_SAR                                   (BIT27 | BIT30)
#define LLI_WITH_CONTIGUOUS_DAR                                    (BIT28)
#define LLI_WITH_AUTO_RELOAD_DAR                                   (BIT28 | BIT31)
#define LLI_TRANSFER                                               (BIT27 | BIT28)

#define IS_GDMA_MULTIBLOCKMODE(MODE) (((MODE) == AUTO_RELOAD_WITH_CONTIGUOUS_SAR) || ((MODE) == AUTO_RELOAD_WITH_CONTIGUOUS_DAR)\
                                      ||((MODE) == AUTO_RELOAD_TRANSFER) || ((MODE) == LLI_WITH_CONTIGUOUS_SAR)\
                                      ||((MODE) == LLI_WITH_AUTO_RELOAD_SAR) || ((MODE) == LLI_WITH_CONTIGUOUS_DAR)\
                                      ||((MODE) == LLI_WITH_AUTO_RELOAD_DAR) || ((MODE) == LLI_TRANSFER))

/** End of Group GDMA_Multiblock_Mode
  * @}
  */

/** @cond private
  * @defgroup GDMA_Multiblock_Select_Bit multi-block select bit
  * @{
  */

#define AUTO_RELOAD_SELECTED_BIT                                   (uint32_t)(0xC0000000)
#define LLP_SELECTED_BIT                                           (uint32_t)(0x18000000)
/** End of Group GDMA_Multiblock_Select_Bit
  * @}
  * @endcond
  */

/** End of Group GDMA_Exported_Constant
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup GDMA_Exported_Functions GDMA Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the GDMA registers to their default reset
  *         values.
  * @param  None
  * @retval None
  */

void GDMA_DeInit(void);

/**
  * @brief  Initializes the GDMA Channelx according to the specified
  *         parameters in the GDMA_InitStruct.
  * @param  GDMA_Channelx: where x can be 0 to 7  to select the DMA Channel.
  * @param  GDMA_InitStruct: pointer to a GDMA_InitTypeDef structure that
  *         contains the configuration information for the specified DMA Channel.
  * @retval None
  */
void GDMA_Init(GDMA_ChannelTypeDef *GDMA_Channelx, GDMA_InitTypeDef *GDMA_InitStruct);

/**
  * @brief  Fills each GDMA_InitStruct member with its default value.
  * @param  GDMA_InitStruct : pointer to a GDMA_InitTypeDef structure which will
  *         be initialized.
  * @retval None
  */
void GDMA_StructInit(GDMA_InitTypeDef *GDMA_InitStruct);

/**
  * @brief  Enables or disables the specified GDMA Channelx.
  * @param  GDMA_Channel_Num: GDMA channel number
  * @param  NewState: new state of the DMA Channelx.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void GDMA_Cmd(uint8_t GDMA_Channel_Num, FunctionalState NewState);

/**
  * @brief  Enables or disables the specified DMAy Channelx interrupts.
  * @param  GDMA_Channel_Num: GDMA channel number.
  * @param  GDMA_IT: specifies the GDMA interrupts sources to be enabled
  *   or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg GDMA_INT_Transfer:  Transfer complete interrupt unmask
  *     @arg GDMA_INT_Block:  Block transfer interrupt unmask
  *     @arg GDMA_INT_SrcTransfer:  SourceTransfer interrupt unmask
  *     @arg GDMA_INT_DstTransfer:  Destination Transfer interrupt unmask
  *     @arg GDMA_INT_Error:  Transfer error interrupt unmask
  * @param  NewState: new state of the specified DMA interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void GDMA_INTConfig(uint8_t GDMA_Channel_Num, uint32_t GDMA_IT, FunctionalState NewState);

/**
  * @brief  Enables or disables the specified DMAy Channelx interrupts.
  * @param  GDMA_Channel_Num: GDMA channel number.
  * @param  GDMA_IT: specifies the GDMA interrupts sources to be enabled
  *   or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg GDMA_INT_Transfer:  clear transfer complete interrupt
  *     @arg GDMA_INT_Block:  clear Block transfer interrupt
  *     @arg GDMA_INT_SrcTransfer:  clear SourceTransfer interrupt
  *     @arg GDMA_INT_DstTransfer:  clear Destination Transfer interrupt
  *     @arg GDMA_INT_Error:  clear Transfer error interrupt
  * @retval None
  */
void GDMA_ClearINTPendingBit(uint8_t GDMA_Channel_Num, uint32_t GDMA_IT);


/**
  * @brief  Checks selected GDMA Channel status.
  * @param  GDMA_Channel_Num: GDMA channel number.
  * @return GDMA channel status: SET: channel is be used, RESET: channel is free.
  */
__STATIC_INLINE FlagStatus GDMA_GetChannelStatus(uint8_t GDMA_Channel_Num)
{
    FlagStatus bit_status = RESET;

    /* Check the parameters */
    assert_param(IS_GDMA_ChannelNum(GDMA_Channel_Num));

    if ((GDMA_BASE->ChEnReg & BIT(GDMA_Channel_Num)) != (uint32_t)RESET)
    {

        bit_status = SET;
    }

    /* Return the selected channel status */
    return  bit_status;
}

/**
  * @brief  Checks GDMA Channel transfer interrupt.
  * @param  GDMA_Channel_Num: GDMA channel number.
  * @return transfer type interrupt status value.
  */
__STATIC_INLINE ITStatus GDMA_GetTransferINTStatus(uint8_t GDMA_Channel_Num)
{
    ITStatus bit_status = RESET;

    /* Check the parameters */
    assert_param(IS_GDMA_ChannelNum(GDMA_Channel_Num));

    if ((GDMA_BASE->STATUS_TFR & BIT(GDMA_Channel_Num)) != (uint32_t)RESET)
    {

        bit_status = SET;
    }

    /* Return the transfer interrupt status */
    return  bit_status;
}

/**
  * @brief  clear the GDMA Channelx all interrupt.
  * @param  GDMA_Channel_Num: GDMA channel number.
  * @retval None
  */
__STATIC_INLINE void GDMA_ClearAllTypeINT(uint8_t GDMA_Channel_Num)
{
    /* Check the parameters */
    assert_param(IS_GDMA_ChannelNum(GDMA_Channel_Num));

    GDMA_BASE->CLEAR_TFR = BIT(GDMA_Channel_Num);
    GDMA_BASE->CLEAR_BLOCK = BIT(GDMA_Channel_Num);
    GDMA_BASE->CLEAR_DST_TRAN = BIT(GDMA_Channel_Num);
    GDMA_BASE->CLEAR_SRC_TRAN = BIT(GDMA_Channel_Num);
    GDMA_BASE->CLEAR_ERR = BIT(GDMA_Channel_Num);
}

/**
  * @brief  set GDMA source address .
  * @param  GDMA_Channelx: where x can be 0 to 7  to select the DMA Channel.
  * @param  Address: destination address.
  * @retval None
  */
__STATIC_INLINE void GDMA_SetSourceAddress(GDMA_ChannelTypeDef *GDMA_Channelx, uint32_t Address)
{
    /* Check the parameters */
    assert_param(IS_GDMA_ALL_PERIPH(GDMA_Channelx));

    GDMA_Channelx->SAR = Address;
}

/**
  * @brief  set GDMA destination address .
  * @param  GDMA_Channelx: where x can be 0 to 5  to select the GDMA Channel.
  * @param  Address: destination address.
  * @retval None
  */
__STATIC_INLINE void GDMA_SetDestinationAddress(GDMA_ChannelTypeDef *GDMA_Channelx,
                                                uint32_t Address)
{
    /* Check the parameters */
    assert_param(IS_GDMA_ALL_PERIPH(GDMA_Channelx));

    GDMA_Channelx->DAR = Address;
}

/**
  *@brief set GDMA buffer size.
  *@param GDMA_Channelx: where x can be 0 to 5  to select the GDMA Channel.
  *@param buffer_size: set size of GDMA_BufferSize.
  *@param
  */
__STATIC_INLINE void GDMA_SetBufferSize(GDMA_ChannelTypeDef *GDMA_Channelx, uint32_t buffer_size)
{
    /* Check the parameters */
    assert_param(IS_GDMA_ALL_PERIPH(GDMA_Channelx));

    /* configure high 32 bit of CTL register */
    GDMA_Channelx->CTL_HIGH = buffer_size;
}

/**
  *@brief  Suspend GDMA transmission from the source.Please check GDMA FIFO empty to guarnatee without losing data.
  *@param  GDMA_Channelx: where x can be 0 to 5  to select the GDMA Channel.
  *@param  NewState: new state of the DMA Channelx.
  *   This parameter can be: ENABLE or DISABLE.
  *@retval None.
  */
__STATIC_INLINE void GDMA_SuspendCmd(GDMA_ChannelTypeDef *GDMA_Channelx,
                                     FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_GDMA_ALL_PERIPH(GDMA_Channelx));
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == DISABLE)
    {
        /* Not suspend transmission*/
        GDMA_Channelx->CFG_LOW &= ~(GDMA_SUSPEND_TRANSMISSSION);
    }
    else
    {
        /* Suspend transmission */
        GDMA_Channelx->CFG_LOW |= GDMA_SUSPEND_TRANSMISSSION;
    }
}

/**
  *@brief  Check GDMA FIFO status.
  *@param  GDMA_Channelx: where x can be 0 to 5  to select the GDMA Channel.
  *@return GDMA FIFO status: SET: empty, RESET:not empty.
  */
__STATIC_INLINE FlagStatus GDMA_GetFIFOStatus(GDMA_ChannelTypeDef *GDMA_Channelx)
{
    FlagStatus bit_status = RESET;

    /* Check the parameters */
    assert_param(IS_GDMA_ALL_PERIPH(GDMA_Channelx));

    if ((GDMA_Channelx->CFG_LOW & GDMA_FIFO_STATUS) != (uint32_t)RESET)
    {

        bit_status = SET;
    }

    /* Return the selected channel status */
    return  bit_status;
}

/**
  *@brief  get GDMA FIFO Length.
  *@param  GDMA_Channelx: where x can be 0 to 5  to select the GDMA Channel.
  *@return GDMA FIFO length.
  */
__STATIC_INLINE uint16_t GDMA_GetTransferLen(GDMA_ChannelTypeDef *GDMA_Channelx)
{
    /* Check the parameters */
    assert_param(IS_GDMA_ALL_PERIPH(GDMA_Channelx));

    return (uint16_t)(GDMA_Channelx->CTL_HIGH & 0xfff);
}

/**
  * @brief  set GDMA LLP address .
  * @param  GDMA_Channelx: Only for GDMA_Channel0&2.
  * @param  Address: destination address.
  * @retval None
  */
__STATIC_INLINE void GDMA_SetLLPAddress(GDMA_ChannelTypeDef *GDMA_Channelx, uint32_t Address)
{
    /* Check the parameters */
    assert_param(IS_GDMA_ALL_PERIPH(GDMA_Channelx));

    if ((GDMA_Channelx == GDMA_Channel0) | (GDMA_Channelx == GDMA_Channel2))
    {
        GDMA_Channelx->LLP = Address;
    }
}

/**
  *@brief  Check GDMA suspend channel status.
  *@param  GDMA_Channelx: where x can be 0 to 5 to select the GDMA Channel.
  *@return GDMA suspend status: SET: inactive, RESET: active.
  */
__STATIC_INLINE FlagStatus GDMA_GetSuspendChannelStatus(GDMA_ChannelTypeDef *GDMA_Channelx)
{
    FlagStatus bit_status = RESET;

    /* Check the parameters */
    assert_param(IS_GDMA_ALL_PERIPH(GDMA_Channelx));

    if ((GDMA_Channelx->CFG_LOW & GDMA_SUSPEND_CHANNEL_STATUS) == GDMA_SUSPEND_CHANNEL_STATUS)
    {
        bit_status = SET;
    }

    /* Return the selected channel suspend status */
    return  bit_status;
}

/**
  *@brief  Check GDMA suspend status.
  *@param  GDMA_Channelx: where x can be 0 to 5 to select the GDMA Channel.
  *@return GDMA suspend status: SET: suspend, RESET:not suspend.
  */
__STATIC_INLINE FlagStatus GDMA_GetSuspendCmdStatus(GDMA_ChannelTypeDef *GDMA_Channelx)
{
    FlagStatus bit_status = RESET;

    /* Check the parameters */
    assert_param(IS_GDMA_ALL_PERIPH(GDMA_Channelx));

    if ((GDMA_Channelx->CFG_LOW & GDMA_SUSPEND_CMD_STATUS) == GDMA_SUSPEND_CMD_STATUS)
    {
        bit_status = SET;
    }

    /* Return the selected channel suspend status */
    return  bit_status;
}

#ifdef __cplusplus
}
#endif

#endif /*__RTL8762X_GDMA_H*/

/** @} */ /* End of group GDMA_Exported_Functions */
/** @} */ /* End of group GDMA */


/******************* (C) COPYRIGHT 2015 Realtek Semiconductor Corporation *****END OF FILE****/

