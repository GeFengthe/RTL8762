/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_lcd.h
* @brief     header file of lcd 8080 parallel interface driver.
* @details
* @author   elliot chen
* @date     2017-11-23
* @version   v1.0
* *********************************************************************************************************
*/


#ifndef _RTL876x_LCD_H_
#define _RTL876x_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"

/* Peripheral: LCD */
/* Description: LCD register defines */

/* Register: CTRL0 -------------------------------------------------------*/
/* Description: Control register 0. Offset: 0x00. Address: 0x40024800. */

/* CTRL0[31]: LCD_OUTPUT_START. 0x1: Start. 0x0: Stop. */
#define LCD_OUTPUT_START_POS            (31)
#define LCD_OUTPUT_START_MSK            ((uint32_t)0x1 << LCD_OUTPUT_START_POS)
#define LCD_OUTPUT_START_CLR            (~LCD_OUTPUT_START_MSK)
/* CTRL0[15:14]: LCD_GUARD_TIME. */
#define LCD_GUARD_TIME_POS              (14)
#define LCD_GUARD_TIME_MSK              (0x3 << LCD_GUARD_TIME_POS)
#define LCD_GUARD_TIME_CLR              (~LCD_GUARD_TIME_MSK)
/* CTRL0[11]: LCD_BYPASS_GUARD_TIME. 0x1: bypass. 0x0: not bypass. */
#define LCD_BYPASS_GUARD_TIME_POS       (11)
#define LCD_BYPASS_GUARD_TIME_MSK       (0x1 << LCD_BYPASS_GUARD_TIME_POS)
#define LCD_BYPASS_GUARD_TIME_CLR       (~LCD_BYPASS_GUARD_TIME_MSK)
/* CTRL0[9]: LCD_8_BIT_SWAP. */
#define LCD_8_BIT_SWAP_POS              (9)
#define LCD_8_BIT_SWAP_MSK              (0x1 << LCD_8_BIT_SWAP_POS)
#define LCD_8_BIT_SWAP_CLR              (~LCD_8_BIT_SWAP_MSK)
/* CTRL0[8]: LCD_16_BIT_SWAP. */
#define LCD_16_BIT_SWAP_POS             (8)
#define LCD_16_BIT_SWAP_MSK             (0x1 << LCD_16_BIT_SWAP_POS)
#define LCD_16_BIT_SWAP_CLR             (~LCD_16_BIT_SWAP_MSK)
/* CTRL0[6:4]: LCD_SPEED_SEL. */
#define LCD_SPEED_SEL_POS               (4)
#define LCD_SPEED_SEL_MSK               (0x7 << LCD_SPEED_SEL_POS)
#define LCD_SPEED_SEL_CLR               (~LCD_SPEED_SEL_MSK)

/* Register: CTRL1 -------------------------------------------------------*/
/* Description: Control registe 1. Offset: 0x04. Address: 0x40024804. */

/* CTRL1[7:3]: LCD_TX_DMA_FIFO_THR. Fixed to 0x08. */
#define LCD_TX_DMA_FIFO_THR_POS         (3)
#define LCD_TX_DMA_FIFO_THR_MSK         (0x1F << LCD_TX_DMA_FIFO_THR_POS)
#define LCD_TX_DMA_FIFO_THR             (0x08 << LCD_TX_DMA_FIFO_THR_POS)
#define LCD_TX_DMA_FIFO_THR_CLR         (~LCD_TX_DMA_FIFO_THR_MSK)
/* CTRL1[1]: LCD_TX_DMA_SINGLE_LOW. */
#define LCD_TX_DMA_SINGLE_LOW_POS       (1)
#define LCD_TX_DMA_SINGLE_LOW_MSK       (0x1 << LCD_TX_DMA_SINGLE_LOW_POS)
#define LCD_TX_DMA_SINGLE_LOW_CLR       (~LCD_TX_DMA_SINGLE_LOW_MSK)
/* CTRL1[0]: LCD_TX_DMA_EN. */
#define LCD_TX_DMA_EN_POS               (0)
#define LCD_TX_DMA_EN_MSK               (0x1 << LCD_TX_DMA_EN_POS)
#define LCD_TX_DMA_EN_CLR               (~LCD_TX_DMA_EN_MSK)

/* Register: IMR ---------------------------------------------------------*/
/* Description: IMR. Offset: 0x08. Address: 0x40024808. */

/* IMR[10]: LCD_MASK_TX_FIFO_EMPTY_INT. */
#define LCD_MASK_TX_FIFO_EMPTY_INT_POS      (10)
#define LCD_MASK_TX_FIFO_EMPTY_INT_MSK      (0x1 << LCD_MASK_TX_FIFO_EMPTY_INT_POS)
#define LCD_MASK_TX_FIFO_EMPTY_INT_CLR      (~LCD_MASK_TX_FIFO_EMPTY_INT_MSK)
/* IMR[9]: LCD_MASK_TX_FIFO_OVERFLOW_INT. */
#define LCD_MASK_TX_FIFO_OVERFLOW_INT_POS   (9)
#define LCD_MASK_TX_FIFO_OVERFLOW_INT_MSK   (0x1 << LCD_MASK_TX_FIFO_OVERFLOW_INT_POS)
#define LCD_MASK_TX_FIFO_OVERFLOW_INT_CLR   (~LCD_MASK_TX_FIFO_OVERFLOW_INT_MSK)
/* IMR[8]: LCD_MASK_TX_FIFO_THR_INT. */
#define LCD_MASK_TX_FIFO_THR_INT_POS        (8)
#define LCD_MASK_TX_FIFO_THR_INT_MSK        (0x1 << LCD_MASK_TX_FIFO_THR_INT_POS)
#define LCD_MASKTX_FIFO_THR_INT_CLR         (~LCD_MASK_TX_FIFO_THR_INT_MSK)
/* IMR[4:0]: LCD_TX_FIFO_THRESHOLD. */
#define LCD_TX_FIFO_THRESHOLD_POS           (0)
#define LCD_TX_FIFO_THRESHOLD_MSK           (0x1F << LCD_TX_FIFO_THRESHOLD_POS)
#define LCD_TX_FIFO_THRESHOLD_CLR           (~LCD_TX_FIFO_THRESHOLD_MSK)

/* Register: SR ---------------------------------------------------------*/
/* Description: SR. Offset: 0x0C. Address: 0x4002480C. */
/* SR[26]: LCD_AUTO_DONE_INT. */
#define LCD_AUTO_DONE_INT_POS           (26)
#define LCD_AUTO_DONE_INT_MSK           (0x1 << LCD_AUTO_DONE_INT_POS)
#define LCD_AUTO_DONE_INT_CLR           (~LCD_AUTO_DONE_INT_MSK)
/* SR[20]: LCD_TX_FIFO_EMPTY_FLAG. */
#define LCD_TX_FIFO_EMPTY_FLAG_POS      (20)
#define LCD_TX_FIFO_EMPTY_FLAG_MSK      (0x1 << LCD_TX_FIFO_EMPTY_FLAG_POS)
#define LCD_TX_FIFO_EMPTY_FLAG_CLR      (~LCD_TX_FIFO_EMPTY_FLAG_MSK)
/* SR[19]: LCD_TX_FIFO_FULL_FLAG. */
#define LCD_TX_FIFO_FULL_FLAG_POS       (19)
#define LCD_TX_FIFO_FULL_FLAG_MSK       (0x1 << LCD_TX_FIFO_FULL_FLAG_POS)
#define LCD_TX_FIFO_FULL_FLAG_CLR       (~LCD_TX_FIFO_FULL_FLAG_MSK)

/* SR[18]: LCD_TX_FIFO_EMPTY_INT. */
#define LCD_TX_FIFO_EMPTY_INT_POS       (18)
#define LCD_TX_FIFO_EMPTY_INT_MSK       (0x1 << LCD_TX_FIFO_EMPTY_INT_POS)
#define LCD_TX_FIFO_EMPTY_INT_CLR       (~LCD_TX_FIFO_EMPTY_INT_MSK)
/* SR[17]: LCD_TX_FIFO_OVERFLOW_INT. */
#define LCD_TX_FIFO_OVERFLOW_INT_POS    (17)
#define LCD_TX_FIFO_OVERFLOW_INT_MSK    (0x1 << LCD_TX_FIFO_OVERFLOW_INT_POS)
#define LCD_TX_FIFO_OVERFLOW_INT_CLR    (~LCD_TX_FIFO_OVERFLOW_INT_MSK)
/* SR[16]: LCD_TX_FIFO_THR_INT. */
#define LCD_TX_FIFO_THR_INT_POS         (16)
#define LCD_TX_FIFO_THR_INT_MSK         (0x1 << LCD_TX_FIFO_THR_INT_POS)
#define LCD_TX_FIFO_THR_INT_CLR         (~LCD_TX_FIFO_THR_INT_MSK)

/* Register: ICR ---------------------------------------------------------*/
/* Description: Interrupt Clear register. Offset: 0x10. Address: 0x40024810. */

/* CFG[5]: LCD_CLR_AUTO_DONE_INT. */
#define LCD_CLR_AUTO_DONE_INT_POS       (5)
#define LCD_CLR_AUTO_DONE_INT_MSK       (0x1 << LCD_CLR_AUTO_DONE_INT_POS)
#define LCD_CLR_AUTO_DONE_INT_CLR       (~LCD_CLR_AUTO_DONE_INT_MSK)
/* CFG[4]: LCD_CLR_TF_EMPTY_INT. */
#define LCD_CLR_TF_EMPTY_INT_POS        (4)
#define LCD_CLR_TF_EMPTY_INT_MSK        (0x1 << LCD_CLR_TF_EMPTY_INT_POS)
#define LCD_CLR_TF_EMPTY_INT_CLR        (~LCD_CLR_TF_EMPTY_INT_MSK)
/* CFG[3]: LCD_CLR_TF_OF_INT. */
#define LCD_CLR_TF_OF_INT_POS           (3)
#define LCD_CLR_TF_OF_INT_MSK           (0x1 << LCD_CLR_TF_OF_INT_POS)
#define LCD_CLR_TF_OF_INT_CLR           (~LCD_CLR_TF_OF_INT_MSK)
/* CFG[2]: LCD_CLR_TF_THR_INT. */
#define LCD_CLR_TF_THR_INT_POS          (2)
#define LCD_CLR_TF_THR_INT_MSK          (0x1 << LCD_CLR_TF_THR_INT_POS)
#define LCD_CLR_TF_THR_INT_CLR          (~LCD_CLR_TF_THR_INT_MSK)
/* CFG[1]: LCD_CLR_OUTPUT. */
#define LCD_CLR_OUTPUT_CNT_POS          (1)
#define LCD_CLR_OUTPUT_CNT_MSK          (0x1 << LCD_CLR_OUTPUT_CNT_POS)
#define LCD_CLR_OUTPUT_CNT_CLR          (~LCD_CLR_OUTPUT_CNT_MSK)
/* CFG[0]: LCD_CLR_FIFO. */
#define LCD_CLR_FIFO_POS                (0)
#define LCD_CLR_FIFO_MSK                (0x1 << LCD_CLR_FIFO_POS)
#define LCD_CLR_FIFO_CLR                (~LCD_CLR_FIFO_MSK)

/* Register: CFG ---------------------------------------------------------*/
/* Description: Configure register. Offset: 0x14. Address: 0x40024814. */

/* CFG[31]: LCD_MODE. */
#define LCD_MODE_POS                    (31)
#define LCD_MODE_MSK                    ((uint32_t)0x1 << LCD_MODE_POS)
#define LCD_MODE_CLR                    ((uint32_t)(~LCD_MODE_MSK))
/* CFG[11]: LCD_RD_TRIGGER_CTRL. */
#define LCD_RD_TRIGGER_CTRL_POS         (11)
#define LCD_RD_TRIGGER_CTRL_MSK         (0x1 << LCD_RD_TRIGGER_CTRL_POS)
#define LCD_RD_TRIGGER_CTRL_CLR         (~LCD_RD_TRIGGER_CTRL_MSK)
/* CFG[10]: LCD_WR_TRIGGER_CTRL. */
#define LCD_WR_TRIGGER_CTRL_POS         (10)
#define LCD_WR_TRIGGER_CTRL_MSK         (0x1 << LCD_WR_TRIGGER_CTRL_POS)
#define LCD_WR_TRIGGER_CTRL_CLR         (~LCD_WR_TRIGGER_CTRL_MSK)
/* CFG[9]: LCD_CMD_DATA_CTRL. */
#define LCD_CMD_DATA_CTRL_POS           (9)
#define LCD_CMD_DATA_CTRL_MSK           (0x1 << LCD_CMD_DATA_CTRL_POS)
#define LCD_CMD_DATA_CTRL_CLR           (~LCD_CMD_DATA_CTRL_MSK)
/* CFG[8]: LCD_MANUAL_CS_CTRL. */
#define LCD_MANUAL_CS_CTRL_POS          (8)
#define LCD_MANUAL_CS_CTRL_MSK          (0x1 << LCD_MANUAL_CS_CTRL_POS)
#define LCD_MANUAL_CS_CTRL_CLR          (~LCD_MANUAL_CS_CTRL_MSK)
/* CFG[1:0]: LCD_AUTO_MODE_CMD_NUM. */
#define LCD_AUTO_MODE_CMD_NUM_POS       (0)
#define LCD_AUTO_MODE_CMD_NUM_MSK       (0x3 << LCD_AUTO_MODE_CMD_NUM_POS)
#define LCD_AUTO_MODE_CMD_NUM_CLR       (~LCD_AUTO_MODE_CMD_NUM_MSK)

/* Register: RXDATA -----------------------------------------------------*/
/* Description: RXDATA. Offset: 0x20. Address: 0x40024820. */

/* RXDATA[8]: LCD_RX_DATA_VALID. */
#define LCD_RX_DATA_VALID_POS           (8)
#define LCD_RX_DATA_VALID_MSK           (0x01 << LCD_RX_DATA_VALID_POS)
#define LCD_RX_DATA_VALID_CLR           (~LCD_RX_DATA_VALID_MSK)
/* RXDATA[7:0]: LCD_RX_DATA_. */
#define LCD_RX_DATA_POS                 (0)
#define LCD_RX_DATA_MSK                 (0xFF << LCD_RX_DATA_POS)
#define LCD_RX_DATA_CLR                 (~LCD_RX_DATA_MSK)

/* Register: RGB_LEN -----------------------------------------------------*/
/* Description: RGB_LEN. Offset: 0x24. Address: 0x40024824. */

/* RGB_LEN[16:0]: LCD_OUTPUT_BYTE_NUM. */
#define LCD_OUTPUT_BYTE_NUM_POS         (0)
#define LCD_OUTPUT_BYTE_NUM_MSK         (0x1FFFF << LCD_OUTPUT_BYTE_NUM_POS)
#define LCD_OUTPUT_BYTE_NUM_CLR         (~LCD_OUTPUT_BYTE_NUM_MSK)

/* Register: DATA_CNT ----------------------------------------------------*/
/* Description: DATA_CNT. Offset: 0x28. Address: 0x40024828. */

/* DATA_CNT[16:0]: LCD_OUTPUT_CNT. */
#define LCD_OUTPUT_CNT_POS              (0)
#define LCD_OUTPUT_CNT_MSK              (0x1FFFF << LCD_OUTPUT_CNT_POS)
#define LCD_OUTPUT_CNT_CLR              (~LCD_OUTPUT_CNT_MSK)



/** @addtogroup LCD LCD
  * @brief LCD driver module
  * @{
  */

/** @defgroup LCD_Exported_Types LCD Exported Types
  * @{
  */

/**
 * @brief LCD initialize parameters
 *
 * LCD initialize parameters
 */
typedef struct
{
    uint32_t LCD_ClockDiv;                      /*!< Specifies the LCD clock speed. */
    uint32_t LCD_Mode;                          /*!< Specifies the LCD operation mode. */
    uint32_t LCD_GuardTimeCmd;                  /*!< Specifies the guard time function. */
    uint32_t LCD_GuardTime;                     /*!< Specifies the guard time. This parameter is 0~3T of divider clock. */
    uint32_t LCD_8BitSwap;                      /*!< Specifies the FIFO data format. */
    uint32_t LCD_16BitSwap;                     /*!<Specifies the FIFO data format.  */
    uint32_t LCD_TxThr;                         /*!<Specifies the TX FIFO threshold value.  */
    uint32_t LCD_TxDMACmd;                      /*!<Specifies the TX DMA status in auto mode.  */
} LCD_InitTypeDef;

/**End of group LCD_Exported_Types
  * @}
  */

/** @defgroup LCD_Exported_Constants LCD Exported Constants
  * @{
  */

/** @defgroup LCD_Pin_Group LCD Pin Group
  * @{
  */
#define LCD_PinGroup_DISABLE                        ((uint32_t)0x00)
#define LCD_PinGroup_1                              ((uint32_t)0x01 << 28)
#define LCD_PinGroup_2                              ((uint32_t)0x02 << 28)

#define IS_LCD_PINGROUP_IDX(IDX) (((IDX) == LCD_PinGroup_DISABLE) || ((IDX) == LCD_PinGroup_1) || \
                                  ((IDX) == LCD_PinGroup_2))

/**
  * @}
  */

/** @defgroup LCD_ClockDiv LCD clock div
  * @{
  */

#define LCD_CLOCK_DIV_2                             (((uint32_t)0x0 << LCD_SPEED_SEL_POS))
#define LCD_CLOCK_DIV_3                             (((uint32_t)0x1 << LCD_SPEED_SEL_POS))
#define LCD_CLOCK_DIV_4                             (((uint32_t)0x2 << LCD_SPEED_SEL_POS))
#define LCD_CLOCK_DIV_5                             (((uint32_t)0x3 << LCD_SPEED_SEL_POS))
#define LCD_CLOCK_DIV_6                             (((uint32_t)0x4 << LCD_SPEED_SEL_POS))
#define LCD_CLOCK_DIV_7                             (((uint32_t)0x5 << LCD_SPEED_SEL_POS))
#define LCD_CLOCK_DIV_8                             (((uint32_t)0x6 << LCD_SPEED_SEL_POS))
#define LCD_CLOCK_DIV_16                            (((uint32_t)0x7 << LCD_SPEED_SEL_POS))

#define IS_LCD_CLOCK_DIV(DIV) (((DIV) == LCD_CLOCK_DIV_2) || ((DIV) == LCD_CLOCK_DIV_3)    || \
                               ((DIV) == LCD_CLOCK_DIV_4) || ((DIV) == LCD_CLOCK_DIV_5) || \
                               ((DIV) == LCD_CLOCK_DIV_6) || ((DIV) == LCD_CLOCK_DIV_7) || \
                               ((DIV) == LCD_CLOCK_DIV_8) || ((DIV) == LCD_CLOCK_DIV_16))

/**
  * @}
  */

/** @defgroup LCD_Mode LCD Mode
  * @{
  */

#define LCD_MODE_AUTO                               (0)
#define LCD_MODE_MANUAL                             (LCD_MODE_MSK)

#define IS_LCD_MODE(MODE) (((MODE) == LCD_MODE_AUTO) || ((MODE) == LCD_MODE_MANUAL))

/**
  * @}
  */

/** @defgroup LCD_GuardTime_Cmd LCD Guard Time Cmd
  * @{
  */

#define LCD_GUARD_TIME_ENABLE                       (0)
#define LCD_GUARD_TIME_DISABLE                      (LCD_BYPASS_GUARD_TIME_MSK)


#define IS_LCD_GUARD_TIME_CMD(CMD) (((CMD) == LCD_GUARD_TIME_ENABLE) || ((CMD) == LCD_GUARD_TIME_DISABLE))

/**
  * @}
  */

/** @defgroup LCD_GuardTime LCD Guard Time
  * @{
  */

#define LCD_GUARD_TIME_1T                           (0)
#define LCD_GUARD_TIME_2T                           (((uint32_t)0x1 << LCD_GUARD_TIME_POS))
#define LCD_GUARD_TIME_3T                           (((uint32_t)0x2 << LCD_GUARD_TIME_POS))
#define LCD_GUARD_TIME_4T                           (LCD_GUARD_TIME_MSK)

#define IS_LCD_GUARD_TIME(TIME) (((TIME) == LCD_GUARD_TIME_1T) || ((TIME) == LCD_GUARD_TIME_2T) || \
                                 ((TIME) == LCD_GUARD_TIME_3T) || ((TIME) == LCD_GUARD_TIME_4T))

/**
  * @}
  */

/** @defgroup LCD_8BitSwap LCD 8 Bit Swap
  * @{
  */

#define LCD_8BitSwap_DISABLE                        (0)
#define LCD_8BitSwap_ENABLE                         (LCD_8_BIT_SWAP_MSK)


#define IS_LCD_8BitSwap_CMD(CMD) (((CMD) == LCD_8BitSwap_DISABLE) || ((CMD) == LCD_8BitSwap_ENABLE))

/**
  * @}
  */

/** @defgroup LCD_16BitSwap LCD 16 Bit Swap
  * @{
  */

#define LCD_16BitSwap_DISABLE                       (0)
#define LCD_16BitSwap_ENABLE                        (LCD_16_BIT_SWAP_MSK)


#define IS_LCD_16BitSwap_CMD(CMD) (((CMD) == LCD_16BitSwap_DISABLE) || ((CMD) == LCD_16BitSwap_ENABLE))

/**
  * @}
  */

/** @defgroup LCD_TxDMACmd LCD Tx DMA Cmd
  * @{
  */

#define LCD_TX_DMA_ENABLE                           (LCD_TX_DMA_EN_MSK)
#define LCD_TX_DMA_DISABLE                          (0)


#define IS_LCD_TX_DMA_CMD(CMD) (((CMD) == LCD_TX_DMA_ENABLE) || ((CMD) == LCD_TX_DMA_DISABLE))

/**
  * @}
  */

/** @defgroup LCD_Flag_Definition  LCD Flag Definition
  * @{
  */

#define LCD_FLAG_TF_EMPTY                           (LCD_TX_FIFO_EMPTY_FLAG_MSK)
#define LCD_FLAG_TF_FULL                            (LCD_TX_FIFO_FULL_FLAG_MSK)

#define IS_LCD_FLAG_CONFIG(CONFIG)   (((CONFIG) == LCD_FLAG_TF_EMPTY) || \
                                      ((CONFIG) == LCD_FLAG_TF_FULL))

/**
  * @}
  */

/** @defgroup LCD_Interrupts_Mask_Definition  LCD Interrupts Mask Definition
  * @{
  */

#define LCD_INT_TF_EMPTY_MSK                        (LCD_MASK_TX_FIFO_EMPTY_INT_MSK)
#define LCD_INT_TF_OF_MSK                           (LCD_MASK_TX_FIFO_OVERFLOW_INT_MSK)
#define LCD_INT_TF_LEVEL_MSK                        (LCD_MASK_TX_FIFO_THR_INT_MSK)

#define IS_LCD_INT_MSK_CONFIG(CONFIG)   (((CONFIG) == LCD_INT_TF_EMPTY_MSK)  || \
                                         ((CONFIG) == LCD_INT_TF_OF_MSK)   || \
                                         ((CONFIG) == LCD_INT_TF_LEVEL_MSK))

/**
  * @}
  */

/** @defgroup LCD_Interrupts_Definition  LCD Interrupts Definition
  * @{
  */
#define LCD_INT_SR_AUTO_DONE                        (LCD_AUTO_DONE_INT_MSK)
#define LCD_INT_SR_TF_EMPTY                         (LCD_TX_FIFO_EMPTY_INT_MSK)
#define LCD_INT_SR_TF_OF                            (LCD_TX_FIFO_OVERFLOW_INT_MSK)
#define LCD_INT_SR_TF_LEVEL                         (LCD_TX_FIFO_THR_INT_MSK)

#define IS_LCD_INT_CONFIG(CONFIG)       (((CONFIG) == LCD_INT_SR_AUTO_DONE)  || \
                                         ((CONFIG) == LCD_INT_SR_TF_EMPTY) || \
                                         ((CONFIG) == LCD_INT_SR_TF_OF)   || \
                                         ((CONFIG) == LCD_INT_SR_TF_LEVEL))

/**
  * @}
  */


/** @defgroup LCD_Interrupts_Clear_Status LCD Interrupts Clear Status
  * @{
  */

#define LCD_INT_AUTO_DONE_CLR                       (LCD_CLR_AUTO_DONE_INT_MSK)
#define LCD_INT_TF_EMPTY_CLR                        (LCD_CLR_TF_EMPTY_INT_MSK)
#define LCD_INT_TF_OF_CLR                           (LCD_CLR_TF_OF_INT_MSK)
#define LCD_INT_TF_LEVEL_CLR                        (LCD_CLR_TF_THR_INT_MSK)

#define IS_LCD_INT_CLEAR(INT)           (((INT) == LCD_INT_AUTO_DONE_CLR) || ((INT) == LCD_INT_TF_EMPTY_CLR) || \
                                         ((INT) == LCD_INT_TF_OF_CLR) || ((INT) == LCD_INT_TF_LEVEL_CLR))

/**
  * @}
  */


/**End of group LCD_Exported_Constants
  * @}
  */

/** @defgroup LCD_Exported_Functions LCD Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the LCD peripheral registers to their default values.
  * @param  None.
  * @retval None
  */
void LCD_DeInit(void);

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
void LCD_PinGroupConfig(uint32_t LCD_PinGroupType);

/**
  * @brief Initializes the LCD peripheral according to the specified
  *   parameters in the LCD_InitStruct
  * @param  LCD: selected LCD peripheral.
  * @param  LCD_InitStruct: pointer to a LCD_InitTypeDef structure that
  *   contains the configuration information for the specified LCD peripheral
  * @retval None
  */
void LCD_Init(LCD_InitTypeDef *LCD_InitStruct);

/**
  * @brief  Fills each LCD_InitStruct member with its default value.
  * @param  LCD_InitStruct: pointer to an LCD_InitTypeDef structure which will be initialized.
  * @retval None
  */
void LCD_StructInit(LCD_InitTypeDef *LCD_InitStruct);

/**
  * @brief   Enable or disable the selected LCD mode.
  * @param  NewState: new state of the operation mode.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void LCD_Cmd(FunctionalState NewState);

/**
  * @brief   Send command in manual mode.
  * @param  cmd: command which to be sent.
  * @retval None
  */
void LCD_SendCommand(uint8_t cmd);

/**
  * @brief   Send data in manual mode.
  * @param  pBuf: buffer address to be sent.
  * @param  len:  data length.
  * @retval None
  */
void LCD_SendData(uint8_t *pBuf, uint32_t len);

/**
  * @brief   Receive data in manual mode.
  * @param  pBuf: buffer address to be received.
  * @param  len: data length.
  * @retval None
  */
void LCD_ReceiveData(uint8_t *pBuf, uint32_t len);

/**
  * @brief   Send command and data buffer in manual mode.
  * @param cmd: command which to be sent.
  * @param  pBuf: buffer address to be sent.
  * @param  len:  data length.
  * @retval None
  */
void LCD_Write(uint8_t cmd, uint8_t *pBuf, uint32_t len);

/**
  * @brief   Send command and read data buffer in manual mode.
  * @param cmd: command which to be sent.
  * @param  pBuf: buffer address to be sent.
  * @param  len:  data length.
  * @retval None
  */
void LCD_Read(uint8_t cmd, uint8_t *pBuf, uint32_t len);

/**
  * @brief   Configure command sequences in auto mode.
 * @param  pCmdBuf: buffer address which store command sequence.
  * @param  len:  command length.
  * @retval None
  */
FlagStatus LCD_SetCmdSequence(uint8_t *pCmdBuf, uint8_t len);

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
void LCD_MaskINTConfig(uint32_t LCD_INT_MSK, FunctionalState NewState);

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
ITStatus LCD_GetINTStatus(uint32_t LCD_INT);

/**
  * @brief Get the specified LCD flag status.
  * @param  LCD_INT: the specified LCD flag.
  * This parameter can be one of the following values:
  *   This parameter can be the following values:
  *     @arg LCD_FLAG_TF_EMPTY:  FIFO empty flag.
  *     @arg LCD_FLAG_TF_FULL:  FIFO full flag.
  * @retval The new state of LCD_FLAG (SET or RESET).
  */
FlagStatus LCD_GetFlagStatus(uint32_t LCD_FLAG);

/**
  * @brief   Dynamic switch LCD operation mode.
  * @param  mode: selected LCD operation mode.
  *   This parameter can be the following values:
  *     @arg LCD_MODE_AUTO: automation mode.
  *     @arg LCD_MODE_MANUAL: manual mode.
  * @retval None
  */
__STATIC_INLINE void LCD_SwitchMode(uint32_t mode)
{
    LCD->CFG &= LCD_MODE_CLR;
    LCD->CFG |= mode;
}

/**
  * @brief  Enable or disable GDMA for LCD transmission.
  * @param  NewState: new state of GDMA.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
__STATIC_INLINE void LCD_GDMACmd(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == ENABLE)
    {
        LCD->CTRL1 |= LCD_TX_DMA_EN_MSK;
    }
    else
    {
        LCD->IMR &= LCD_TX_DMA_EN_CLR;
    }
}

/**
  * @brief  Set CS singal in manual mode.
  * @param  None
  * @retval None
  */
__STATIC_INLINE void LCD_SetCS(void)
{
    LCD->CFG |= LCD_MANUAL_CS_CTRL_MSK;
}

/**
  * @brief  Reset CS singal in manual mode.
  * @param  None
  * @retval None
  */
__STATIC_INLINE void LCD_ResetCS(void)
{
    LCD->CFG &= LCD_MANUAL_CS_CTRL_CLR;
}

/**
  * @brief  Clears the LCD interrupt pending bits.
  * @param  LCD_INT: specifies the interrupt pending bit to clear.
  *   This parameter can be any combination of the following values:
  *     @arg LCD_INT_AUTO_DONE_CLR: Clear auto done interrupt.
  *     @arg LCD_INT_TF_EMPTY_CLR: Clear TX FIFO empty interrupt.
  *     @arg LCD_INT_TF_OF_CLR: Clear TX FIFO overflow interrupt.
  *     @arg LCD_INT_TF_LEVEL_CLR: Clear TX FIFO threshold interrupt.
  * @retval None
  */
__STATIC_INLINE void LCD_ClearINTPendingBit(uint32_t LCD_CLEAR_INT)
{
    /* Check the parameters */
    assert_param(IS_LCD_INT_CLEAR(LCD_CLEAR_INT));

    LCD->ICR &= LCD_CLEAR_INT;
}

/**
 * @brief  set LCD output data length whose unit is byte.
 * @param  len : length of read data which can be 0 to 0x1FFFF.
 * @retval None
 */
__STATIC_INLINE uint32_t LCD_SetTxDataLen(uint32_t len)
{
    return LCD->RGB_LEN = len & LCD_OUTPUT_BYTE_NUM_MSK;
}

/**
 * @brief  Get LCD output data length whose unit is byte.
 * @param  None.
 * @retval length of read data which can be 0 to 0x1FFFF
 */
__STATIC_INLINE uint32_t LCD_GetTxDataLen(void)
{
    return (LCD->RGB_LEN & LCD_OUTPUT_BYTE_NUM_MSK);
}

/**
 * @brief  Get LCD output data counter whose unit is byte.
 * @param  None.
 * @retval length of counter which can be 0 to 0x1FFFF
 */
__STATIC_INLINE uint32_t LCD_GetDataCounter(void)
{
    return (LCD->DATA_CNT & LCD_OUTPUT_CNT_MSK);
}

/**
 * @brief  Clear LCD output data counter value.
 * @param  None.
 * @retval None
 */
__STATIC_INLINE void LCD_ClearDataCounter(void)
{
    LCD->ICR &= LCD_CLR_OUTPUT_CNT_MSK;
}

/**
 * @brief  Write LCD FIFO in auto mode.
 * @param  data: FIFO data.
 * @retval None
 */
__STATIC_INLINE void LCD_WriteFIFO(uint32_t data)
{
    LCD->FIFO = data;
}

/**
 * @brief  Clear LCD output data counter value.
 * @param  None.
 * @retval None
 */
__STATIC_INLINE void LCD_ClearFIFO(void)
{
    LCD->ICR &= LCD_CLR_FIFO_MSK;
}

#ifdef __cplusplus
}
#endif

#endif /* _RTL876x_LCD_H_ */

/**End of group LCD_Exported_Functions
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

