/**
*****************************************************************************************
*     Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file    System_rtl876x.h
  * @brief   CMSIS Device System Header file
  * @details CMSIS Device System Header File for RTL876x Device Series
  * @author
  * @date    3. March 2016
  * @version v1.0
  * *************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2016 Realtek Semiconductor Corporation</center></h2>
   * ************************************************************************************
   */

/*============================================================================*
 *                      Define to prevent recursive inclusion
 *============================================================================*/
#ifndef SYSTEM_RTL876X_H
#define SYSTEM_RTL876X_H


/*============================================================================*
 *                      Headers
 *============================================================================*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup SYSTEM_RTL876X  System RTL876X
  * @brief CMSIS API sets for RTL876x Device Series
  * @{
  */

/*============================================================================*
 *                      Types
 *============================================================================*/
/** @defgroup SYSTEM_RTL876X_Exported_Types System RTL876X Exported Types
  * @{
  */
typedef void (*IRQ_Fun)();       /**< ISR Handler Prototype */

typedef enum
{
    LOG_TIMESTAMP_OS        = 0,
    LOG_TIMESTAMP_HW_TIMER7 = 1,
    LOG_TIMESTAMP_RSVD      = 2,
    LOG_TIMESTAMP_MAX       = 3
} T_LOG_TIMESTAMP_TYPE;

typedef enum
{
    InitialSP_VECTORn = 0,
    Reset_VECTORn,
    NMI_VECTORn,
    HardFault_VECTORn,
    MemMang_VECTORn,
    BusFault_VECTORn,
    UsageFault_VECTORn,
    RSVD0_VECTORn,
    RSVD1_VECTORn,
    RSVD2_VECTORn,
    RSVD3_VECTORn,
    SVC_VECTORn,
    DebugMonitor_VECTORn,
    RSVD4_VECTORn,
    PendSV_VECTORn,
    SysTick_VECTORn,
    System_VECTORn = 16,
    WDG_VECTORn,
    BTMAC_VECTORn,
    Timer3_VECTORn,
    Timer2_VECTORn,
    Platform_VECTORn,
    I2S0_TX_VECTORn,
    I2S0_RX_VECTORn,
    Timer4_7_VECTORn,
    GPIO4_VECTORn,
    GPIO5_VECTORn,
    Uart1_VECTORn,
    Uart0_VECTORn,
    RTC_VECTORn,
    SPI0_VECTORn,
    SPI1_VECTORn,
    I2C0_VECTORn,
    I2C1_VECTORn,
    ADC_VECTORn,
    Peripheral_VECTORn,
    GDMA0_Channel0_VECTORn,
    GDMA0_Channel1_VECTORn,
    GDMA0_Channel2_VECTORn,
    GDMA0_Channel3_VECTORn,
    GDMA0_Channel4_VECTORn,
    GDMA0_Channel5_VECTORn,
    GPIO_Group3_VECTORn,
    GPIO_Group2_VECTORn,
    IR_VECTORn,
    GPIO_Group1_VECTORn,
    GPIO_Group0_VECTORn,
    Uart2_VECTORn,
    Timer4_VECTORn = 48,
    Timer5_VECTORn,
    Timer6_VECTORn,
    Timer7_VECTORn,
    SPIFlash_VECTORn,
    Qdecode_VECTORn,
    Keyscan_VECTORn,
    SPI2W_VECTORn,
    LPCOMP_VECTORn,
    PTA_Mailbox_VECTORn,
    SPORT1_TX_VECTORn,
    SPORT1_RX_VECTORn,
    LCD_VECTORn,
} VECTORn_Type;

typedef bool (*APP_MAIN_FUNC)();
typedef void (*USER_CALL_BACK)();
typedef bool (*BOOL_DFU_STATUS_CB)();

/** @} */ /* End of group SYSTEM_RTL876X_Exported_types */

/*============================================================================*
 *                      Variables
 *============================================================================*/
/** @defgroup SYSTEM_RTL876X_Exported_Variables System RTL876X Exported Variables
    * @brief
    * @{
    */
extern APP_MAIN_FUNC app_pre_main;
extern APP_MAIN_FUNC app_main;
extern USER_CALL_BACK app_pre_main_cb;
extern BOOL_DFU_STATUS_CB user_dfu_status_cb;
/** @} */ /* End of group SYSTEM_RTL876X_Exported_Variables */

/*============================================================================*
 *                      Functions
 *============================================================================*/
/** @defgroup SYSTEM_RTL876X_Exported_Functions System RTL876X Exported Functions
    * @brief
    * @{
    */

/**
 * @brief  Initialize RAM vector table to a given RAM address.
 * @param  ram_vector_addr: RAM Vector Address.
 * @retval TRUE Success
 * @retval FALSE Fail
 * @note   When using vector table relocation, the base address of the new vector
 *         table must be aligned to the size of the vector table extended to the
 *         next larger power of 2. In RTL8762C, the base address is aligned at 0x100.
 */
extern bool RamVectorTableInit(uint32_t ram_vector_addr);

/**
 * @brief  Update ISR Handler in RAM Vector Table.
 * @param  v_num: Vector number(index)
 * @param  isr_handler: User defined ISR Handler.
 * @retval TRUE Success
 * @retval FALSE Fail
 */
extern bool RamVectorTableUpdate(VECTORn_Type v_num, IRQ_Fun isr_handler);

/*
 * @brief  Call the system service.
 * @param  opcode: operation code.
 * @param  parm: parameter.
 * @return none
 */
extern void SystemCall(uint32_t opcode, uint32_t parm);

/** @} */ /* End of group SYSTEM_RTL876X_Exported_Functions */

/** @} */ /* End of group SYSTEM_RTL876X */

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_RTL876X_H */

