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
 * @brief test triplet for power strip product
 *  --product id(dec)--product secret(ascii)--mac address(hex)--
2880    5ecc3d9350bc895292ff602a298927bb    38d2ca021495
2880    a36d9bd40213c6d9cfc076e6a958ec66    38d2ca021496
2880    7ff102459a99b540ffd8f2bc883d6b2a    38d2ca021497
2880    14d48f3fa46ab4f84f075bc26876fe14    38d2ca021498
2880    1244cab8bf92f0601ddfa8ddfb0d53ea    38d2ca021499
2880    f8bab2826fe8b1935f27db9b3c73590c    38d2ca02149a
2880    004c9161ecab6fde6a31c54d96ba77a7    38d2ca02149b
2880    e512a0555f5eea4e5ebb6dc8da64ab24    38d2ca02149c
2880    c8527f2fcc2975f5c057c1b93a30aa8d    38d2ca02149d
2880    f04ea300de25b53d0459bba52877a97f    38d2ca02149e
*/

/*test
7845,1221de1846b0cb57fdc4de1ccfef5c8f,f8a763523aa9
*/

#define ALI_PRODUCT_ID                          2880
#define ALI_VERSION_ID                          0x00000001
#define ALI_SECRET_KEY                          "1244cab8bf92f0601ddfa8ddfb0d53ea"
#define ALI_SECRET_LEN                          32
#define ALI_AIS_SUPPORT                         1
#define ALI_AIS_AUTH                            1

/* realtek defined ids */
#define DFU_AUTO_BETWEEN_DEVICES                0
#define DFU_PRODUCT_ID                          DFU_PRODUCT_ID_MESH_UNKNOWN
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
