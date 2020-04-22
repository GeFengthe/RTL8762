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
 * 5702 06bc5a3ff153f747b3675f9d8c4b77a4    f8a7631d71a9
 * 5702 76d81cad073535e602d7ed03e77425ec    f8a7631d71aa
 * 5702 d1437f99d878d7a2a536719c81e97ebc    f8a7631d71ab
 * 5702 e21f3b065688e9b44678ccdba9db0ed5    f8a7631d71ac
 * 5702 906566c9383f1f6b203a5b27f8acf26d    f8a7631d71ad
 * 5702 e82bcb2e4badd730b070c0f9ee3bd603    f8a7631d71ae
 * 5702 36734870172d833ecf7856532df98e5a    f8a7631d71af
 * 5702 acbe4b5902b87a4c02bde5ae8b248441    f8a7631d71b0
 * 5702 4d2727c8f4ebe39062371835770537b4    f8a7631d71b1
 * 5702 80b1a765285a4689782922a298897684    f8a7631d71b2
*/

#define ALI_PRODUCT_ID                          5702
#define ALI_VERSION_ID                          0x00000001
#define ALI_SECRET_KEY                          "06bc5a3ff153f747b3675f9d8c4b77a4"
#define ALI_SECRET_LEN                          32
#define ALI_AIS_SUPPORT                         1

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
