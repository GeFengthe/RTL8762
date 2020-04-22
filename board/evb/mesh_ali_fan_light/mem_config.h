/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file    mem_config.h
  * @brief   Memory Configuration
  * @date    2017.6.6
  * @version v1.0
  * *************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   * *************************************************************************************
  */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef MEM_CONFIG_H
#define MEM_CONFIG_H

#include "flash_map.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Code configuration
 *============================================================================*/
/** @brief set app bank to support OTA: 1 is ota bank1, 0 is ota bank0 */
#define APP_BANK                                  0

/** @brief ram code configuration: 1 is ram code, 0 is flash code */
#define FEATURE_RAM_CODE                          0

/** @brief encrypt app or not */
#define FEATURE_ENCRYPTION                        0

/* ali defined ids */
/**
 * @brief test triplet for fan light(cw light + fan)
 *  --product id(dec)--product secret(ascii)--mac address(hex)--
 * 2384 2efdf43b02717af1527373d9a9b421da 109e3a1c5f54
 * 2384 ca81f2203b24a7a11f6d87c1bbd2ba29 109e3a1c5f55
 * 2384 94688de86b1142cb9b11ef9a31727b2b 109e3a1c5f56
 * 2384 fea11a8827c50135a3fc4d001da83213 109e3a1c5f57
 * 2384 09544465ce2413ef4b3bc8cebfd47c52 109e3a1c5f58
*/
#define ALI_PRODUCT_ID                          2384
#define ALI_VERSION_ID                          0x00000001
#define ALI_SECRET_KEY                          "2efdf43b02717af1527373d9a9b421da"
#define ALI_SECRET_LEN                          32
#define ALI_AIS_SUPPORT                         1
#define ALI_AIS_AUTH                            1

/* realtek defined ids */
#define DFU_AUTO_BETWEEN_DEVICES                0
#define DFU_PRODUCT_ID                          DFU_PRODUCT_ID_MESH_ALI_LIGHT
#define DFU_APP_VERSION                         0x00000000
//#define DFU_VERSION_1                      ({T_IMG_HEADER_FORMAT *p_header = (T_IMG_HEADER_FORMAT *)get_header_addr_by_img_id(RomPatch);\
//        (p_header->git_ver.ver_info.sub_version._version_major << 28) + (p_header->git_ver.ver_info.sub_version._version_minor << 20)\
//        + (p_header->git_ver.ver_info.sub_version._version_revision << 5) + p_header->git_ver.ver_info.sub_version._version_reserve;})
#define DFU_VERSION(a)                       ({T_IMG_HEADER_FORMAT *p_header = (T_IMG_HEADER_FORMAT *)get_header_addr_by_img_id(a);\
        (p_header->git_ver.ver_info.sub_version._version_major << 28) + (p_header->git_ver.ver_info.sub_version._version_minor << 20)\
        + (p_header->git_ver.ver_info.sub_version._version_revision << 5) + p_header->git_ver.ver_info.sub_version._version_reserve;})

/* sig mesh defined ids */
#define COMPANY_ID                              0x01A8
#define PRODUCT_ID                              0x0000
#define VERSION_ID                              0x0000

/* mesh ota defined ids */
#define DFU_UPDATER_FW_ID                       BYTES4(ALI_VERSION_ID)

/*============================================================================*
 *                        data ram layout configuration
 *============================================================================*/
/* Data RAM layout:                   112K
example:
   1) reserved for rom and patch:      31K (fixed)
   2) app global + ram code:           35K (adjustable, config APP_GLOBAL_SIZE)
   3) Heap ON:                         30K (adjustable, config APP_GLOBAL_SIZE)
   6) patch ram code:                  16K (fixed)
*/

/** @brief data ram size for app global variables and code, could be changed, but (APP_GLOBAL_SIZE + HEAP_DATA_ON_SIZE) must be 65k */
#define APP_GLOBAL_SIZE                (30 * 1024)

/** @brief data ram size for heap, could be changed, but (APP_GLOBAL_SIZE + HEAP_DATA_ON_SIZE) must be 65k */
#define HEAP_DATA_ON_SIZE              (65 * 1024 - APP_GLOBAL_SIZE)

/** @brief shared cache ram size (adjustable, config SHARE_CACHE_RAM_SIZE: 0/8KB/16KB) */
#define SHARE_CACHE_RAM_SIZE           (0 * 1024)


#ifdef __cplusplus
}
#endif


/** @} */ /* End of group MEM_CONFIG */
#endif
