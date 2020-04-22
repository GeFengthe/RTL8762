/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_wdg.h
* @brief     header file of watch dog driver.
* @details
* @author    Lory_xu
* @date      2016-06-12
* @version   v0.1
* *********************************************************************************************************
*/

#ifndef _RTL876X_WDG_H_
#define _RTL876X_WDG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"
#include "rtl876x_bitfields.h"
#include "flash_device.h"

/** @addtogroup WATCH_DOG WATCH_DOG
  * @brief Watch Dog driver module
  * @{
  */

/** @defgroup WATCH_DOG_Exported_Macros Watch Dog Exported Macros
  * @brief
  * @{
  */
#define BTAON_FAST_RESET_REASON       0x15
/**
  * @}
  */


/** @defgroup WATCH_DOG_Exported_Types Watch Dog Exported Types
  * @{
  */
typedef enum
{
    INTERRUPT_CPU = 0,
    RESET_ALL_EXCEPT_AON = 1,
    RESET_CORE_DOMAIN = 2,
    RESET_ALL = 3
} T_WDG_MODE;

typedef enum
{
    RESET_REASON_HW                 = 0x0,  /* HW reset */
    RESET_REASON_WDG_TIMEOUT        = 0x1,
    SW_RESET_APP_START              = 0xD0,
    UPPER_CMD_RESET,
    DFU_SWITCH_TO_OTA,
    DFU_ACTIVE_RESET,
    DFU_SYSTEM_RESET,
    SWITCH_TEST_MODE,
    SWITCH_HCI_MODE,
    SINGLE_TONE_TIMEOUT_RESET,
    UART_CMD_RESET,
    RESET_REASON_FACTORY_RESET,
    SW_RESET_APP_END                = 0xFF,
} T_SW_RESET_REASON;

#ifdef SDK_8772
typedef void (*APP_CB_WDG_RESET_TYPE)(T_WDG_MODE wdg_mode, T_SW_RESET_REASON reset_reason);
extern APP_CB_WDG_RESET_TYPE app_cb_wdg_reset;
#endif

typedef bool (*BOOL_WDG_CB)();

/**
  * @}
  */

/** @defgroup WATCH_DOG_Exported_Variables Watch Dog Exported Variables
  * @{
  */
extern BOOL_WDG_CB user_wdg_cb;

/**
  * @}
  */

/** @defgroup WATCH_DOG_Exported_Functions Watch Dog Exported Functions
  * @{
  */

/**
   * @brief  Watch Dog Clock Enable.
   */
extern void WDG_ClockEnable(void);

/**
   * @brief  Watch Dog Timer Config.
   * @param  div_factor: 16Bit: 32.768k/(1+divfactor).
   * @param  cnt_limit: 2^(cnt_limit+1) - 1 ; max 11~15 = 0xFFF.
   * @param  wdg_mode: 0: interrupt CPU
   *                   1: reset all except aon
   *                   2: reset core domain
   *                   3: reset all
   * @retval none.
   */
extern void WDG_Config(uint16_t div_factor, uint8_t cnt_limit, T_WDG_MODE wdg_mode);

/**
   * @brief  Watch Dog Timer Enable.
   */
extern void WDG_Enable(void);

/**
   * @brief  Watch Dog Timer Disable.
   */
extern void WDG_Disable(void);

/**
   * @brief  Watch Dog Timer Restart.
   */
extern void WDG_Restart(void);

/**
   * @brief  Watch Dog System Reset.
   * @param  wdg_mode: 0: interrupt CPU
   *                   1: reset all except aon
   *                   2: reset core domain
   *                   3: reset all
   */
extern void WDG_SystemReset(T_WDG_MODE wdg_mode, T_SW_RESET_REASON reset_reason);

/**
 ** @brief  get aon register recorded reset reason.
 ** @return  Three types:
 *   a) HW reset: return 0
 *   b) SW RESET_ALL: return 0.
 *   c) SW RESET_ALL_EXCEPT_AON: return 0x2~0xFF @ref T_SW_RESET_REASON.
 *So, at the moment SDK can't distinguish HW reset and SW RESET_ALL,  can only distinguish type c) and not type c).
 */
static inline T_SW_RESET_REASON get_aon_record_reset_reason(void)
{
    return (T_SW_RESET_REASON)flash_ioctl(flash_ioctl_get_aon_record_reset_reason, 0, 0);
}

T_SW_RESET_REASON  reset_reason_get(void);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif //_RTL876X_WDG_H_
