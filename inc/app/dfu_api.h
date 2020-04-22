/**
************************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
************************************************************************************************************
* @file     dfu_api.h
* @brief    APIs to implement device firmware update.
* @details  OTA is used to update device firmware via bluetooth.
* @author   ranhui
* @date     2015-10-29
* @version  v1.0
*************************************************************************************************************
*/

#ifndef _DFU_API_H_
#define  _DFU_API_H_

#include <stdint.h>
#include <stdbool.h>

#define DFU_HEADER_SIZE                   12 /*only first 12 byte ctrl header*/

/** @defgroup DFU_API DFU API Sets
  * @brief API sets for device firmware update implementation
  * @{
  */

/*============================================================================*
  *                                Functions
  *============================================================================*/
/** @defgroup DFU_API_Exported_Functions DFU API Sets Exported Functions
    * @{
    */

typedef enum
{
    DFU_START = 0,
    DFU_DOING,
    DFU_END,
} DFU_PROGRESS_STATUS;

#ifdef SDK_8772
typedef void (*APP_CB_DFU_STATUS_TYPE)(DFU_PROGRESS_STATUS status, uint32_t length);
extern APP_CB_DFU_STATUS_TYPE app_cb_dfu_status;
#endif

/**
 * @brief  Set OTA mode flag of stack.
 *
 * After call this API, App can call API
 * "WDG_SystemReset(RESET_ALL_EXCEPT_AON);"
 * to restart the platform to enter OTA mode.
 * @param enable  true to Enable OTA mode, false to disable OTA mode.
 * @return  void
*/
void dfu_set_ota_mode_flag(bool enable);

void dfu_switch_to_ota_mode(void);
bool dfu_check_checksum(uint16_t image_id);
uint32_t dfu_checkbufcrc(uint8_t *buf, uint32_t length, uint16_t mCrcVal);
uint32_t dfu_flash_check_blank(uint16_t signature, uint32_t offset, uint16_t nSize);
uint32_t dfu_report_target_fw_info(uint16_t image_id, uint32_t *p_origin_fw_version,
                                   uint32_t *p_offset);
uint32_t dfu_report_target_ic_type(uint16_t signature, uint8_t *p_ic_type);
void dfu_init(void);
void dfu_fw_active_reset(void);
bool dfu_reset(uint16_t image_id);
uint32_t dfu_update(uint16_t signature, uint32_t offset, uint32_t length, void *p_void);
uint32_t dfu_flash_erase(uint16_t signature, uint32_t offset);

/** @} */ /* End of group DFU_API_Exported_Functions */
/** @} */ /* End of group DFU_API */

#endif //_DFU_API_H_

