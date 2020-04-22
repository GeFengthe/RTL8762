/**
****************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
****************************************************************************************************
* @file      ftl_app_cb.h
* @brief
* @note      support for puran flash
* @author    Grace
* @date      2018-04-19
* @version   v0.1
* **************************************************************************************************
*/

#ifndef _FTL_APP_CB_H_
#define _FTL_APP_CB_H_

#include <stdint.h>

#ifdef  __cplusplus
extern  "C" {
#endif  // __cplusplus

#define FTL_CHECK_CRC_SUCCESS                   (0x0)
#define FTL_CHECK_CRC_FAIL                      (0x1)


#define FTL_READ_ERROR_CRC_FAIL                 (0x06)

#define FTL_BUFFER_DATA_WRITE_BACK_NONE         (0x00)
#define FTL_BUFFER_DATA_WRITE_BACK_SUCCESS      (0x01)
#define FTL_BUFFER_DATA_WRITE_BACK_DO_GC        (0x02)

uint32_t ftl_init_app_cb(uint32_t u32PageStartAddr, uint8_t pagenum);
uint32_t ftl_read_app_cb(uint16_t logical_addr, uint32_t *value);
uint32_t ftl_write_app_cb(uint16_t logical_addr, uint32_t w_data);
uint16_t ftl_gc_imp_app_cb(void);
uint32_t ftl_buffer_write_back(void);

#ifdef  __cplusplus
}
#endif // __cplusplus

#endif // _FTL_APP_CB_H_
