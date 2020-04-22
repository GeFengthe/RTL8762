/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_rcc.h
* @brief     header file of reset and clock control driver.
* @details
* @author    tifnan_ge
* @date      2015-05-16
* @version   v1.0
* *********************************************************************************************************
*/


#ifndef _RTL876X_RCC_H_
#define _RTL876X_RCC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"
#include "rtl876x_bitfields.h"



/** @addtogroup RCC RCC
  * @brief RCC driver module
  * @{
  */



/** @defgroup RCC_Exported_Constants RCC Exported Constants
  * @{
  */


/** @defgroup RCC_Peripheral_Clock  RCC Peripheral Clock
  * @{
  */

#define APBPeriph_I2S0_CLOCK              ((uint32_t)((1 << 5) | (1 << 8) | (0x00 << 29)))
#define APBPeriph_I2S1_CLOCK              ((uint32_t)((1 << 6) | (1 << 8) | (0x00 << 29)))
#define APBPeriph_CODEC_CLOCK             ((uint32_t)((1 << 4) | (0x00 << 29)))

#define APBPeriph_GPIO_CLOCK              ((uint32_t)((0x01 << 24) | (0x01 << 29)))
#define APBPeriph_GDMA_CLOCK              ((uint32_t)((0x01 << 16) | (0x01 << 29)))
#define APBPeriph_TIMER_CLOCK             ((uint32_t)((0x01 << 14) | (0x01 << 29)))
#define APBPeriph_UART2_CLOCK             ((uint32_t)((0x01 << 10) | (0x01 << 29)))
#define APBPeriph_UART1_CLOCK             ((uint32_t)((0x01 << 12) | (0x01 << 29)))
#define APBPeriph_FLASH_CLOCK             ((uint32_t)((0x01 << 8)  | (0x01 << 29)))

#define APBPeriph_IR_CLOCK                ((uint32_t)((0x01 << 20) | (0x02 << 29)))
#define APBPeriph_SPI1_CLOCK              ((uint32_t)((0x01 << 18) | (0x02 << 29)))
#define APBPeriph_SPI0_CLOCK              ((uint32_t)((0x01 << 16) | (0x02 << 29)))
#define APBPeriph_UART0_CLOCK             ((uint32_t)((0x01 << 0) | (0x02 << 29)))

#define APBPeriph_LCD_CLOCK               ((uint32_t)((0x01 << 28) | (0x03 << 29)))
#define APBPeriph_ADC_CLOCK               ((uint32_t)((0x01 << 24) | (0x03 << 29)))
#define APBPeriph_SPI2W_CLOCK             ((uint32_t)((0x01 << 16) | (0x03 << 29)))
#define APBPeriph_KEYSCAN_CLOCK           ((uint32_t)((0x01 << 6) | (0x03 << 29)))
#define APBPeriph_QDEC_CLOCK              ((uint32_t)((0x01 << 4) | (0x03 << 29)))
#define APBPeriph_I2C1_CLOCK              ((uint32_t)((0x01 << 2) | (0x03 << 29)))
#define APBPeriph_I2C0_CLOCK              ((uint32_t)((0x01 << 0) | (0x03 << 29)))

#define IS_APB_PERIPH_CLOCK(CLOCK) (((CLOCK) == APBPeriph_GPIO_CLOCK) || ((CLOCK) == APBPeriph_GDMA_CLOCK)\
                                    || ((CLOCK) == APBPeriph_TIMER_CLOCK) || ((CLOCK) == APBPeriph_IR_CLOCK)\
                                    || ((CLOCK) == APBPeriph_SPI1_CLOCK) || ((CLOCK) == APBPeriph_SPI0_CLOCK)\
                                    || ((CLOCK) == APBPeriph_UART0_CLOCK) || ((CLOCK) == APBPeriph_ADC_CLOCK)\
                                    || ((CLOCK) == APBPeriph_SPI2W_CLOCK) || ((CLOCK) == APBPeriph_KEYSCAN_CLOCK)\
                                    || ((CLOCK) == APBPeriph_QDEC_CLOCK) || ((CLOCK) == APBPeriph_I2C1_CLOCK)\
                                    || ((CLOCK) == APBPeriph_I2C0_CLOCK) || ((CLOCK) == APBPeriph_CODEC_CLOCK)\
                                    || ((CLOCK) == APBPeriph_UART1_CLOCK) || ((CLOCK) == APBPeriph_UART2_CLOCK)\
                                    || ((CLOCK) == APBPeriph_I2S0_CLOCK) || ((CLOCK) == APBPeriph_I2S1_CLOCK)\
                                    || ((CLOCK) == APBPeriph_LCD_CLOCK))

/**
  * @}
  */

/** @defgroup APB_Peripheral_Define APB Peripheral Define
  * @{
  */

#define APBPeriph_TIMER                 ((uint32_t)((1 << 16) | (0x00 << 26)))
#define APBPeriph_GDMA                  ((uint32_t)((1 << 13) | (0x00 << 26)))
#define APBPeriph_UART1                 ((uint32_t)((1 << 12) | (0x00 << 26)))
#define APBPeriph_FLASH                 ((uint32_t)((1 << 4)  | (0x00 << 26)))

#define APBPeriph_LCD                   ((uint32_t)((1 << 25) | (0x02 << 26)))
#define APBPeriph_SPI2W                 ((uint32_t)((1 << 24) | (0x02 << 26)))
#define APBPeriph_KEYSCAN               ((uint32_t)((1 << 19) | (0x02 << 26)))
#define APBPeriph_QDEC                  ((uint32_t)((1 << 18) | (0x02 << 26)))
#define APBPeriph_I2C1                  ((uint32_t)((1 << 17) | (0x02 << 26)))
#define APBPeriph_I2C0                  ((uint32_t)((1 << 16) | (0x02 << 26)))
#define APBPeriph_IR                    ((uint32_t)((1 << 10) | (0x02 << 26)))
#define APBPeriph_SPI1                  ((uint32_t)((1 << 9) | (0x02 << 26)))
#define APBPeriph_SPI0                  ((uint32_t)((1 << 8) | (0x02 << 26)))
#define APBPeriph_UART0                 ((uint32_t)((1 << 0) | (0x02 << 26)))
#define APBPeriph_UART2                 ((uint32_t)((1 << 1) | (0x02 << 26)))

#define APBPeriph_GPIO                  ((uint32_t)((1 << 8) | (0x03 << 26)))
#define APBPeriph_ADC                   ((uint32_t)((1 << 0) | (0x03 << 26)))

#define APBPeriph_I2S0                  ((uint32_t)((1 << 1) | (0x00 << 26)))
#define APBPeriph_I2S1                  ((uint32_t)((1 << 2) | (0x00 << 26)))
#define APBPeriph_CODEC                 ((uint32_t)((1 << 0) | (0x00 << 26)))


#define IS_APB_PERIPH(PERIPH) (((PERIPH) == APBPeriph_TIMER) || ((PERIPH) == APBPeriph_GDMA)\
                               || ((PERIPH) == APBPeriph_SPI2W) || ((PERIPH) == APBPeriph_KEYSCAN)\
                               || ((PERIPH) == APBPeriph_QDEC) || ((PERIPH) == APBPeriph_I2C1)\
                               || ((PERIPH) == APBPeriph_I2C0) || ((PERIPH) == APBPeriph_IR)\
                               || ((PERIPH) == APBPeriph_SPI1) || ((PERIPH) == APBPeriph_SPI0)\
                               || ((PERIPH) == APBPeriph_UART0) || ((PERIPH) == APBPeriph_GPIO)\
                               || ((PERIPH) == APBPeriph_ADC) || ((PERIPH) == APBPeriph_CODEC)\
                               || (PERIPH == APBPeriph_UART1) || (PERIPH == APBPeriph_UART2)\
                               || ((PERIPH) == APBPeriph_I2S0) || ((PERIPH) == APBPeriph_I2S1)\
                               || ((PERIPH) == APBPeriph_LCD))

/**
  * @}
  */

/**End of group RCC_Exported_Constants
  * @}
  */

/** @defgroup RCC_Exported_Macros RCC Exported Macros
  * @{
  */

/** @defgroup RCC_Peripheral_Clock  RCC Peripheral Clock
  * @{
  */

#define CLOCK_GATE_5M                          ((uint32_t)(0x01 << 29))/* 5M clock source for adc and keyscan */
#define CLOCK_GATE_20M                         ((uint32_t)(0x01 << 27))/* 20M clock source for 2wssi and qdec */
#define CLOCK_GATE_10M                         ((uint32_t)(0x01 << 28))/* 10M clock source for bluewiz */
#define IS_CLOCK_GATE(CLOCK) (((CLOCK) == CLOCK_GATE_5M) || ((CLOCK) == CLOCK_GATE_20M)\
                              || ((CLOCK) == CLOCK_GATE_10M))

/**
  * @}
  */

/** @defgroup SPI_Clock_Divider SPI Clock Divider
  * @{
  */

#define SPI_CLOCK_DIV_1                    ((uint16_t)0x0)
#define SPI_CLOCK_DIV_2                    ((uint16_t)0x1)
#define SPI_CLOCK_DIV_4                    ((uint16_t)0x2)
#define SPI_CLOCK_DIV_8                    ((uint16_t)0x3)
#define IS_SPI_DIV(DIV)              (((DIV) == SPI_CLOCK_DIV_1) || \
                                      ((DIV) == SPI_CLOCK_DIV_2) || \
                                      ((DIV) == SPI_CLOCK_DIV_4) || \
                                      ((DIV) == SPI_CLOCK_DIV_8))
/**
  * @}
  */

/** @defgroup I2C_Clock_Divider I2C Clock Divider
  * @{
  */

#define I2C_CLOCK_DIV_1                    ((uint16_t)0x0)
#define I2C_CLOCK_DIV_2                    ((uint16_t)0x1)
#define I2C_CLOCK_DIV_4                    ((uint16_t)0x2)
#define I2C_CLOCK_DIV_8                    ((uint16_t)0x3)
#define IS_I2C_DIV(DIV)              (((DIV) == I2C_CLOCK_DIV_1) || \
                                      ((DIV) == I2C_CLOCK_DIV_2) || \
                                      ((DIV) == I2C_CLOCK_DIV_4) || \
                                      ((DIV) == I2C_CLOCK_DIV_8))
/**
  * @}
  */

/** @defgroup UART_Clock_Divider UART Clock Divider
  * @{
  */

#define UART_CLOCK_DIV_1                    ((uint16_t)0x0)
#define UART_CLOCK_DIV_2                    ((uint16_t)0x1)
#define UART_CLOCK_DIV_4                    ((uint16_t)0x2)
#define UART_CLOCK_DIV_16                   ((uint16_t)0x3)
#define IS_UART_DIV(DIV)              (((DIV) == UART_CLOCK_DIV_1) || \
                                       ((DIV) == UART_CLOCK_DIV_2) || \
                                       ((DIV) == UART_CLOCK_DIV_4) || \
                                       ((DIV) == UART_CLOCK_DIV_16))
/**
  * @}
  */

/**End of group RCC_Exported_Macros
  * @}
  */

/** @defgroup RCC_Exported_Functions RCC Exported Functions
  * @{
  */

extern void RCC_PeriphClockCmd(uint32_t APBPeriph, uint32_t APBPeriph_Clock,
                               FunctionalState NewState);
extern void RCC_PeriClockConfig(uint32_t APBPeriph_Clock, FunctionalState NewState);
extern void RCC_PeriFunctionConfig(uint32_t APBPeriph, FunctionalState NewState);
extern void RCC_SPIClkDivConfig(SPI_TypeDef *SPIx, uint16_t ClockDiv);
extern void RCC_I2CClkDivConfig(I2C_TypeDef *I2Cx, uint16_t ClockDiv);
extern void RCC_UARTClkDivConfig(UART_TypeDef *UARTx, uint16_t ClockDiv);


#ifdef __cplusplus
}
#endif

#endif /* _RTL876X_RCC_H_ */

/**End of group RCC_Exported_Functions
  * @}
  */

/**
  * @}
  */



/******************* (C) COPYRIGHT 2015 Realtek Semiconductor *****END OF FILE****/



