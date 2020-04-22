/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      switch_app.h
* @brief     Smart mesh switch application header file
* @details
* @author    elliot chen
* @date      2018-9-13
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef _SWITCH_APP_
#define _SWITCH_APP_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <profile_client.h>
#include <profile_server.h>
#include "app_msg.h"
#include "mesh_api.h"

typedef struct
{
    uint16_t cid;
    struct
    {
        uint8_t adv_ver: 4;
        uint8_t sec: 1;
        uint8_t ota: 1;
        uint8_t bt_ver: 2; //!< 0 bt4.0, 1 bt4.2, 2 bt5.0, 3 higher
    } pid;
    uint32_t product_id;
    uint8_t mac_addr[6];
    struct
    {
        uint8_t silent_adv: 1;
        uint8_t uuid_ver: 7;
    } feature;
    uint8_t rfu[2];
} _PACKED_ ali_uuid_t;

/**
 * @brief test pids for ali light
 *  --product id--mac address--product secret--

    lightness pids
    00000002  78da071127ea  ed97034564d01aed3c91d1a1b03d0069
    00000002  78da071127eb  362bdeb7e820d40960ac45b5593e58c0
    00000002  78da071127ec  8b4eb878d14669f62553dc65befd6971
    00000002  78da071127ed  392796712cbce91f73eafda1d10b4aa1
    00000002  78da071127ee  fb698169762238dfaa114bd859f20579

    ctl pids
    465 78da07b89d7a 50e69b7d948d1bd7ba9ed7b008389010
    465 78da07b89d7b 089ef1b65c776ad518fcf5c3948b4a29
    465 78da07b89d7c 3afc7365ae9f9aa351285bc2151b60b4

    hsl pids
*/

#if 1
#define ALI_PRODUCT_ID          374
#define ALI_SECRET_KEY          "48d667fc61c29efcf7192e41cfdaae84"
#define ALI_SECRET_LEN          32
#else
#define ALI_PRODUCT_ID          2//0x293e2
#define ALI_SECRET_KEY          "362bdeb7e820d40960ac45b5593e58c0"
#define ALI_SECRET_LEN          32
#endif
/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg);
/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data);

/**
 * @brief  Callback will be called when data sent from profile client layer.
 * @param  client_id the ID distinguish which module sent the data.
 * @param  conn_id connection ID.
 * @param  p_data  pointer to data.
 * @retval   result @ref T_APP_RESULT
 */
T_APP_RESULT app_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data);
T_APP_RESULT app_profile_callback(T_SERVER_ID service_id, void *p_data);

bool prov_cb(prov_cb_type_t cb_type, prov_cb_data_t cb_data);
void fn_cb(uint8_t frnd_index, fn_cb_type_t type, uint16_t lpn_addr);
void lpn_cb(uint8_t frnd_index, lpn_cb_type_t type, uint16_t fn_addr);
#ifdef __cplusplus
}
#endif

#endif /* _SWITCH_APP_ */
