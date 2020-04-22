/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_3wire_spi.h
* @brief    This file provides all the 3 wire SPI firmware functions.
* @details
* @author    elliot chen
* @date      2016-12-13
* @version   v1.0
* *********************************************************************************************************
*/


#ifndef _RTL876x_3WIRE_SPI_H_
#define _RTL876x_3WIRE_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"

/* Peripheral: SPI3WIRE */
/* Description: SPI3WIRE register defines */

/* Register: CFGR ------------------------------------------------------------*/
/* Description: SPI3WIRE configuration register. Offset: 0x30. Address: 0x40004030. */

/* CFGR[31] :SPI3WIRE_SSI_EN. 0x1: enable SPI3WIRE mode. 0x0: disable SPI3WIRE mode. */
#define SPI3WIRE_SSI_EN_Pos                 (31UL)
#define SPI3WIRE_SSI_EN_Msk                 (0x1UL << SPI3WIRE_SSI_EN_Pos)
#define SPI3WIRE_SSI_EN_CLR                 (~SPI3WIRE_SSI_EN_Msk)
/* CFGR[30] :SPI3WIRE_SSI_CS_EN. 0x1: enable CS signal. 0x0: disable CS signal. */
#define SPI3WIRE_SSI_CS_EN_Pos              (30UL)
#define SPI3WIRE_SSI_CS_EN_Msk              (0x1UL << SPI3WIRE_SSI_CS_EN_Pos)
#define SPI3WIRE_SSI_CS_EN_CLR              (~SPI3WIRE_SSI_CS_EN_Msk)
/* CFGR[29] :SPI3WIRE_SSI_END_EXT_EN. 0x1: extend mode. 0x0: normal mode. */
/* Note: Using this control bit can extend the timing window for SPI output enable = 0, The extend time is 1/(2*SPI_CLK) */
#define SPI3WIRE_SSI_END_EXT_EN_Pos         (29UL)
#define SPI3WIRE_SSI_END_EXT_EN_Msk         (0x1UL << SPI3WIRE_SSI_END_EXT_EN_Pos)
#define SPI3WIRE_SSI_END_EXT_EN_CLR         (~SPI3WIRE_SSI_END_EXT_EN_Msk)
/* CFGR[28] :SPI3WIRE_SSI_OE_DLY_EN. 0x1: enable. 0x0: disable. */
/* Note: Extend 1T of ssi_OE(output-> intput switch delay 4T->5T  ) */
#define SPI3WIRE_SSI_OE_DLY_EN_Pos          (28UL)
#define SPI3WIRE_SSI_OE_DLY_EN_Msk          (0x1UL << SPI3WIRE_SSI_OE_DLY_EN_Pos)
#define SPI3WIRE_SSI_OE_DLY_EN_CLR          (~SPI3WIRE_SSI_OE_DLY_EN_Msk)
/* CFGR[21] :SPI3WIRE_SSI_RESYNC_TIME. */
/* output n*T low pulse, write 1 to 0x38 bit[31] to output;The T time is 1/(2*ssi_CLK) */
#define SPI3WIRE_SSI_RESYNC_TIME_Pos        (21UL)
#define SPI3WIRE_SSI_RESYNC_TIME_Msk        (0xfUL << SPI3WIRE_SSI_RESYNC_TIME_Pos)
#define SPI3WIRE_SSI_RESYNC_TIME_CLR        (~SPI3WIRE_SSI_RESYNC_TIME_Msk)
/* CFGR[20] :SPI3WIRE_SSI_BURST_READ_EN. 0x1: enable burst read. 0x0: disable. */
#define SPI3WIRE_SSI_BURST_READ_EN_Pos      (20UL)
#define SPI3WIRE_SSI_BURST_READ_EN_Msk      (0x1UL << SPI3WIRE_SSI_BURST_READ_EN_Pos)
#define SPI3WIRE_SSI_BURST_READ_EN_CLR      (~SPI3WIRE_SSI_BURST_READ_EN_Msk)
/* CFGR[16] :SPI3WIRE_SSI_BURST_READ_NUM. The total number of read data bytes in burst mode */
#define SPI3WIRE_SSI_BURST_READ_NUM_Pos     (16UL)
#define SPI3WIRE_SSI_BURST_READ_NUM_Msk     (0xfUL << SPI3WIRE_SSI_BURST_READ_NUM_Pos)
#define SPI3WIRE_SSI_BURST_READ_NUM_CLR     (~SPI3WIRE_SSI_BURST_READ_NUM_Msk)
/* CFGR[8] :SPI3WIRE_SSI_DIV_NUM. The divider number to generate 2x SPI_CLK */
#define SPI3WIRE_SSI_DIV_NUM_Pos            (8UL)
#define SPI3WIRE_SSI_DIV_NUM_Msk            (0xffUL << SPI3WIRE_SSI_DIV_NUM_Pos)
#define SPI3WIRE_SSI_DIV_NUM_CLR            (~SPI3WIRE_SSI_DIV_NUM_Msk)
/* CFGR[3] :SPI3WIRE_SSI_DLY_CYCLE. The delay time from the end of address phase to the start of read data phase. */
#define SPI3WIRE_SSI_DLY_CYCLE_Pos          (3UL)
#define SPI3WIRE_SSI_DLY_CYCLE_Msk          (0x1fUL << SPI3WIRE_SSI_DLY_CYCLE_Pos)
#define SPI3WIRE_SSI_DLY_CYCLE_CLR          (~SPI3WIRE_SSI_DLY_CYCLE_Msk)
/* CFGR[1] :SPI3WIRE_SSI_INT_MASK. 0x1: mask. 0x0: no mask. */
#define SPI3WIRE_SSI_INT_MASK_Pos           (1UL)
#define SPI3WIRE_SSI_INT_MASK_Msk           (0x1UL << SPI3WIRE_SSI_INT_MASK_Pos)
#define SPI3WIRE_SSI_INT_MASK_CLR           (~SPI3WIRE_SSI_INT_MASK_Msk)
/* CFGR[0] :SPI3WIRE_SSI_INT_EN. 0x1: enable. 0x0: disable. */
#define SPI3WIRE_SSI_INT_EN_Pos             (0UL)
#define SPI3WIRE_SSI_INT_EN_Msk             (0x1UL << SPI3WIRE_SSI_INT_EN_Pos)
#define SPI3WIRE_SSI_INT_EN_CLR             (~SPI3WIRE_SSI_INT_EN_Msk)

/* Register: CR ------------------------------------------------------------*/
/* Description: SPI3WIRE control register. Offset: 0x34. Address: 0x40004034. */

/* CR[15] :SPI3WIRE_RW_MODE. 0x1: write mode. 0x0: read mode. */
#define SPI3WIRE_RW_MODE_Pos                (15UL)
#define SPI3WIRE_RW_MODE_Msk                (0x1UL << SPI3WIRE_RW_MODE_Pos)
#define SPI3WIRE_RW_MODE_CLR                (~SPI3WIRE_RW_MODE_Msk)
/* CR[8] :SPI3WIRE_ADDRESS. */
#define SPI3WIRE_ADDRESS_Pos                (8UL)
#define SPI3WIRE_ADDRESS_Msk                (0x7fUL << SPI3WIRE_ADDRESS_Pos)
#define SPI3WIRE_ADDRESS_CLR                (~SPI3WIRE_ADDRESS_Msk)
/* CR[0] :SPI3WIRE_DATA. write data ; in read mode, this data byte is useless */
#define SPI3WIRE_DATA_Pos                   (0UL)
#define SPI3WIRE_DATA_Msk                   (0xffUL << SPI3WIRE_ADDRESS_Pos)
#define SPI3WIRE_DATA_CLR                   (~SPI3WIRE_ADDRESS_Msk)

/* Register: INTCR ------------------------------------------------------------*/
/* Description: SPI3WIRE interrupt clear register. Offset: 0x38. Address: 0x40004038. */

/* INTCR[31] :SPI3WIRE_RESYNV_EN. 0x1: trriger resync signal. 0x0: disable resync signal. */
#define SPI3WIRE_RESYNV_EN_Pos              (30UL)
#define SPI3WIRE_RESYNV_EN_Msk              (0x1UL << SPI3WIRE_RESYNV_EN_Pos)
#define SPI3WIRE_RESYNV_EN_CLR              (~SPI3WIRE_RESYNV_EN_Msk)
/* INTCR[2] :SPI3WIRE_RD_DATA_CLEAR. 0x1: write clear. */
#define SPI3WIRE_RD_DATA_CLEAR_Pos          (2UL)
#define SPI3WIRE_RD_DATA_CLEAR_Msk          (0x1UL << SPI3WIRE_RD_DATA_CLEAR_Pos)
/* INTCR[1] :SPI3WIRE_RD_NUM_CLEAR. 0x1: write clear. */
#define SPI3WIRE_RD_NUM_CLEAR_Pos           (1UL)
#define SPI3WIRE_RD_NUM_CLEAR_Msk           (0x1UL << SPI3WIRE_RD_NUM_CLEAR_Pos)
/* INTCR[0] :SPI3WIRE_INT_CLEAR. 0x1: write clear. */
#define SPI3WIRE_INT_CLEAR_Pos              (0UL)
#define SPI3WIRE_INT_CLEAR_Msk              (0x1UL << SPI3WIRE_INT_CLEAR_Pos)

#define SPI3WIRE_FIFO_INT_ALL_CLR           (SPI3WIRE_RD_DATA_CLEAR_Msk | \
                                             SPI3WIRE_RD_NUM_CLEAR_Msk | \
                                             SPI3WIRE_INT_CLEAR_Msk)

/* Register: SR ------------------------------------------------------------*/
/* Description: SPI3WIRE status register. Offset: 0x3C. Address: 0x4000403C. */

/* SR[6] :SPI3WIRE_RESYNV_BUSY. 0x1: Resync busy. 0x0: Resync idle. */
#define SPI3WIRE_RESYNV_BUSY_Pos                (6UL)
#define SPI3WIRE_RESYNV_BUSY_Msk                (0x1UL << SPI3WIRE_RESYNV_BUSY_Pos)
#define SPI3WIRE_RESYNV_BUSY_CLR                (~SPI3WIRE_RESYNV_BUSY_Msk)
/* SR[5] :SPI3WIRE_INT_STATUS. 0x1: Interrupt. 0x0: No interrupt. */
#define SPI3WIRE_INT_STATUS_Pos                 (5UL)
#define SPI3WIRE_INT_STATUS_Msk                 (0x1UL << SPI3WIRE_INT_STATUS_Pos)
#define SPI3WIRE_INT_STATUS_CLR                 (~SPI3WIRE_INT_STATUS_Msk)
/* SR[4] :SPI3WIRE_BUSY. 0x1: Busy. 0x0:Idle. */
#define SPI3WIRE_BUSY_Pos                       (4UL)
#define SPI3WIRE_BUSY_Msk                       (0x1UL << SPI3WIRE_BUSY_Pos)
#define SPI3WIRE_BUSY_CLR                       (~SPI3WIRE_BUSY_Msk)
/* SR[0] :SPI3WIRE_RDATA_NUM. The total number of data byte in each SPI read transaction */
#define SPI3WIRE_RDATA_NUM_Pos                  (0UL)
#define SPI3WIRE_RDATA_NUM_Msk                  (0xfUL << SPI3WIRE_RDATA_NUM_Pos)
#define SPI3WIRE_RDATA_NUM_CLR                  (~SPI3WIRE_RDATA_NUM_Msk)

/** @addtogroup THREE_WIRE_SPI 3WIRE SPI
  * @brief 3WIRE SPI driver module
  * @{
  */

/*============================================================================*
 *                         Types
 *============================================================================*/


/** @defgroup THREE_WIRE_SPI_Exported_Types 3WIRE SPI Exported Types
  * @{
  */

/**
 * @brief THREE_WIRE_SPI initialize parameters 3WIRE SPI initialize parameters
 *
 *
 */
typedef struct
{
    uint32_t SPI3WIRE_SysClock;     /*!< Specifies system clock */
    uint32_t SPI3WIRE_Speed;        /*!< Specifies SPI clock. */
    uint32_t SPI3WIRE_Mode;         /*!< Specifies SPI operation mode.
                                This parameter can be a value of @ref THREE_WIRE_SPI_mode */
    uint32_t SPI3WIRE_ReadDelay;    /*!< Specifies the delay time from the end of address phase to the start of read data phase.
                                    This parameter can be a value of 0x0 to 0x1f, delay time = (SPI3WIRE_ReadDelay+1)/(2*SPI3WIRE_Speed) */
    uint32_t SPI3WIRE_OutputDelay;  /*!< Specifies SPI output delay 1T or not.
                                 This parameter can be a value of @ref THREE_WIRE_SPI_OE_delay_config */

    uint32_t SPI3WIRE_ExtMode;      /*!< Specifies extended timing window for SPI output enable = 0. */
} SPI3WIRE_InitTypeDef;

/** End of THREE_WIRE_SPI_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/

/** @defgroup THREE_WIRE_SPI_Exported_Constants 3WIRE SPI Exported Constants
  * @{
  */

/** @defgroup THREE_WIRE_SPI_mode 3WIRE SPI mode
  * @{
  */

#define SPI3WIRE_3WIRE_MODE                     ((uint32_t)(1 << SPI3WIRE_SSI_CS_EN_Pos))
#define SPI3WIRE_2WIRE_MODE                     ((uint32_t)(0 << SPI3WIRE_SSI_CS_EN_Pos))

#define IS_SPI3WIRE_MODE(MODE) ((MODE) == SPI3WIRE_2WIRE_MODE || (MODE) == SPI3WIRE_3WIRE_MODE)

/** End of THREE_WIRE_SPI_mode
  * @}
  */

/** @defgroup THREE_WIRE_SPI_OE_delay_config 3WIRE SPI OE delay config
  * @{
  */

#define SPI3WIRE_OE_DELAY_1T                     ((uint32_t)(1 << SPI3WIRE_SSI_OE_DLY_EN_Pos))
#define SPI3WIRE_OE_DELAY_NONE                   ((uint32_t)(0 << SPI3WIRE_SSI_OE_DLY_EN_Pos))

#define IS_SPI3WIRE_OE_DELAY_CFG(CFG) ((CFG) == SPI3WIRE_OE_DELAY_1T || (CFG) == SPI3WIRE_OE_DELAY_NONE)

/** End of THREE_WIRE_SPI_OE_delay_config
  * @}
  */


/** @defgroup THREE_WIRE_SPI_end_extend_mode 3WIRE SPI end extend mode
  * @{
  */

#define SPI3WIRE_EXTEND_MODE                     ((uint32_t)(1 << SPI3WIRE_SSI_END_EXT_EN_Pos))
#define SPI3WIRE_NORMAL_MODE                     ((uint32_t)(0 << SPI3WIRE_SSI_END_EXT_EN_Pos))

#define IS_SPI3WIRE_END_EXTEND_MODE(MODE) ((MODE) == SPI3WIRE_EXTEND_MODE || (MODE) == SPI3WIRE_NORMAL_MODE)

/** End of THREE_WIRE_SPI_end_extend_mode
  * @}
  */

/** @defgroup THREE_WIRE_SPI_read_cycle_delay 3WIRE SPI read cycle delay
  * @{
  */

#define IS_SPI3WIRE_READ_CYCLE_DELAY(DELAY) (DELAY <= 0x1f)

/** End of THREE_WIRE_SPI_read_cycle_delay
  * @}
  */


/** @defgroup THREE_WIRE_SPI_FLAG 3WIRE SPI FLAG
  * @{
  */

#define SPI3WIRE_FLAG_BUSY                       ((uint32_t)SPI3WIRE_BUSY_Msk)
#define SPI3WIRE_FLAG_INT_IND                    ((uint32_t)SPI3WIRE_INT_STATUS_Msk)
#define SPI3WIRE_FLAG_RESYNC_BUSY                ((uint32_t)SPI3WIRE_RESYNV_BUSY_Msk)

#define IS_SPI3WIRE_FLAG(FLAG) (((FLAG) == SPI3WIRE_FLAG_BUSY) || \
                                ((FLAG) == SPI3WIRE_FLAG_INT_IND) || \
                                ((FLAG) == SPI3WIRE_FLAG_RESYNC_BUSY))

/** End of THREE_WIRE_SPI_FLAG
  * @}
  */

/** @defgroup THREE_WIRE_SPI_interrupt_definition 3WIRE SPI interrupt definition
  * @{
  */

#define SPI3WIRE_INT_BIT                          ((uint32_t)SPI3WIRE_SSI_INT_EN_Msk)

#define IS_SPI3WIRE_INT(INT) ((INT) == SPI3WIRE_INT_BIT)

/** End of THREE_WIRE_SPI_interrupt_definition
  * @}
  */

/** End of THREE_WIRE_SPI_Exported_Constants
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup THREE_WIRE_SPI_Exported_Functions 3WIRE SPI Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the 3WIRE SPI peripheral registers to their default reset values.
  * @retval None
  */
void SPI3WIRE_DeInit(void);

/**
  * @brief Initializes the SPI_3WIRE peripheral according to the specified
  *   parameters in the SPI_3WIRE_InitStruct
  * @param  SPI3WIRE_InitStruct: pointer to a SPI3WIRE_InitTypeDef structure that
  *   contains the configuration information for the specified SPI_3WIRE peripheral
  * @retval None
  */
void SPI3WIRE_Init(SPI3WIRE_InitTypeDef *SPI3WIRE_InitStruct);

/**
  * @brief  Fills each SPI3WIRE_InitStruct member with its default value.
  * @param  SPI3WIRE_InitStruct: pointer to an SPI3WIRE_InitTypeDef structure which will be initialized.
  * @retval None
  */
void SPI3WIRE_StructInit(SPI3WIRE_InitTypeDef *SPI3WIRE_InitStruct);

/**
  * @brief  Configure resync signal time value.
  * @param  value: Resync signal time value whose uint is 1/(2*SPI3WIRE_Speed).
  *   This parameter can be only be the following value: 0x0 to 0xf.
  * @retval None
  */
void SPI3WIRE_SetResyncTime(uint32_t value);

/**
  * @brief  Send resync signal or not .Must send when SPI3WIRE is disable.
  * @param  NewState: new state of the SPI3WIRE peripheral.
  *   This parameter can be only be the following value:
  *     @arg ENABLE: trigger resync signal.
  *     @arg DISABLE: disable resync signal.
  * @retval None
  */
void SPI3WIRE_ResyncSignalCmd(FunctionalState NewState);

/**
  * @brief  Enables or disables the specified SPI3WIRE peripheral.
  * @param  NewState: new state of the SPI3WIRE peripheral.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SPI3WIRE_Cmd(FunctionalState NewState);

/**
  * @brief  Get total number of data byte in each SPI reading.
  * @param  None
  * @retval The total number of data byte in each SPI reading.
  */
uint8_t SPI3WIRE_GetRxDataLen(void);

/**
  * @brief  Clear read data number status.
  * @param  None
  * @retval None
  */
void SPI3WIRE_ClearRxDataLen(void);

/**
  * @brief  Clear all read data registers.
  * @param  None
  * @retval None
  */
void SPI3WIRE_ClearRxFIFO(void);

/**
  * @brief  start to write data.
  * @param  address: write address.
  * @param  data: write data.
  * @retval None
  */
void SPI3WIRE_StartWrite(uint8_t address, uint8_t data);

/**
  * @brief  start read.
  * @param  address: read address.
  * @param  len: number of data to read.This value can be 0x1 to 0xf.
  * @retval None.
  */
void SPI3WIRE_StartRead(uint8_t address, uint32_t len);

/**
  * @brief  write data.
  * @param  pBuf: buffer to store read datas.
  * @param  readNum: read number.
  * @retval None
  */
void SPI3WIRE_ReadBuf(uint8_t *pBuf, uint8_t readNum);

/**
  * @brief  Checks whether the specified 3WIRE_SPI flag is set or not.
  * @param  SPI3WIRE_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg SPI3WIRE_FLAG_BUSY: 3wire spi is busy.
  *     @arg SPI3WIRE_FLAG_INT_IND: there is 3wire spi interrupt.
  *     @arg SPI3WIRE_FLAG_RESYNC_BUSY: resync busy or not.
  * @return The new state of SPI3W Flag status.
  * @retval SET: SPI3WIRE Flag status is pending.
  * @retval RESET: SPI3WIRE Flag status is not pending.
  */
FlagStatus SPI3WIRE_GetFlagStatus(uint32_t SPI3WIRE_FLAG);

/**
  * @brief  Enables or disables the specified SPI3WIRE interrupts.
  * @param  SPI3WIRE_INT: specifies the SPI3WIRE interrupts sources to be enabled or disabled.
  *   This parameter can be only be the following value:
  *     @arg SPI3WIRE_INT_BIT: enable SPI3WIRE interrupt.
  * @param  newState: new state of the specified SPI3WIRE interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SPI3WIRE_INTConfig(uint32_t SPI3WIRE_INT, FunctionalState newState);

/**
  * @brief  Clears the 3WIRE_SPI interrupt pending bits.
  * @param  SPI3WIRE_INT: specifies the SPI3WIRE interrupts sources to be enabled or disabled.
  *   This parameter can be only be the following value:
  *     @arg SPI3WIRE_INT_BIT: enable SPI3WIRE interrupt.
  * @retval None
  */

void SPI3WIRE_ClearINTPendingBit(uint32_t SPI3WIRE_INT);

#ifdef __cplusplus
}
#endif

#endif /* _RTL8762_3WIRE_SPI_H_ */

/** @} */ /* End of group THREE_WIRE_SPI_Exported_Functions */
/** @} */ /* End of group THREE_WIRE_SPI */


/******************* (C) COPYRIGHT 2016 Realtek Semiconductor Corporation *****END OF FILE****/

