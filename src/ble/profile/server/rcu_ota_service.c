/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     ota_service.c
* @brief
* @details
* @author   Ken_mei
* @date     14-May-2018
* @version  v1.0.0
******************************************************************************
* @attention
* <h2><center>&copy; COPYRIGHT 2015 Realtek Semiconductor Corporation</center></h2>
******************************************************************************
*/

#include <string.h>
#include <gatt.h>
#include <bt_types.h>
#include "board.h"
#include "trace.h"
#include "gap_le.h"
#include "gap_conn_le.h"
#include "gap_msg.h"
#include "app_msg.h"
#include "flash_device.h"
#include "patch_header_check.h"
#include "mem_config.h"
#include "rcu_ota_service.h"
#include "otp.h"
#include "flash_adv_cfg.h"

/********************************************************************************************************
* local static variables defined here, only used in this source file.
********************************************************************************************************/
/*============================================================================*
 *                              Micro
 *============================================================================*/
#define SOCV_CFG    0
#define SYS_CFG     1
#define OTA_HEADER  2
#define SECURE_BOOT 3
#define ROM_PATCH   4
#define APP_IMG     5
#define APP_DATA1   6
#define APP_DATA2   7

#define IMAGE_NOEXIST         0
#define IMAGE_LOCATION_BANK0  1
#define IMAGE_LOCATION_BANK1  2
#define IMAGE_FIX_BANK_EXIST  3


/*============================================================================*
 *                              Local Variables
 *============================================================================*/
/**<  Function pointer used to send event to application from BWPS extended profile. */
/**<  Initiated in BWPSExtended_AddService. */
P_FUN_SERVER_GENERAL_CB pfnOTAExtendedCB = NULL;
uint8_t mac_addr[6];
uint32_t patch_version = 0;
uint32_t patch_ext_version = 0;
uint32_t app_version = 0;

const uint8_t GATT_UUID_OTA_SERVICE[16] = { 0x12, 0xA2, 0x4D, 0x2E, 0xFE, 0x14, 0x48, 0x8e, 0x93, 0xD2, 0x17, 0x3C, 0xFF, 0xD0, 0x00, 0x00};

/**< @brief  profile/service definition.
*   here is an example of OTA service table
*   including Write
*/
const T_ATTRIB_APPL gattOtaServiceTable[] =
{
    /*--------------------------OTA Service ---------------------------*/
    /* <<Primary Service>>, .. 0*/
    {
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),  /* wFlags     */
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),     /* bTypeValue */
        },
        UUID_128BIT_SIZE,                     /* bValueLen     */
        (void *)GATT_UUID_OTA_SERVICE,        /* pValueContext */
        GATT_PERM_READ                              /* wPermissions  */
    },

    /* <<Characteristic1>>, .. 1*/                         //----------------------OTA CMD        1
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE_NO_RSP,                    /* characteristic properties */
            //XXXXMJMJ GATT_CHAR_PROP_INDICATE,                  /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  OTA characteristic value 2*/
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_OTA),
            HI_WORD(GATT_UUID_CHAR_OTA),
        },
        2,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ | GATT_PERM_WRITE            /* wPermissions */
    },

    /* <<Characteristic2>>, .. 3, MAC Address*/              //------------------------MAC Address  2
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            //XXXXMJMJ GATT_CHAR_PROP_INDICATE,                  /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  OTA characteristic value 4*/
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_MAC),
            HI_WORD(GATT_UUID_CHAR_MAC),
        },
        1,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic3>>, .. 5, Patch version*/            //-------------------------Patch Version  3
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            //XXXXMJMJ GATT_CHAR_PROP_INDICATE,                  /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  OTA characteristic value 6*/
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_PATCH),
            HI_WORD(GATT_UUID_CHAR_PATCH),
        },
        1,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },
    /* <<Characteristic4>>, .. 7 App version*/            //-----------------------------APP Version   4
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            //XXXXMJMJ GATT_CHAR_PROP_INDICATE,                  /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  OTA characteristic value 8*/
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_APP_VERSION),
            HI_WORD(GATT_UUID_CHAR_APP_VERSION),
        },
        1,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },
    /* <<Characteristic5>>, .. 9 Patch extension version*/  //--------------------Patch Ext Version    5
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            //XXXXMJMJ GATT_CHAR_PROP_INDICATE,                  /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  OTA characteristic value 0x0A*/
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_PATCH_EXTENSION),
            HI_WORD(GATT_UUID_CHAR_PATCH_EXTENSION),
        },
        1,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic5>>, .. B TEST MODE*/                 //------------------------MP TEST MODE     6
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE_NO_RSP,                    /* characteristic properties */
            //XXXXMJMJ GATT_CHAR_PROP_INDICATE,                  /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  OTA characteristic value 0x0C*/
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_TEST_MODE),
            HI_WORD(GATT_UUID_CHAR_TEST_MODE),
        },
        0,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_WRITE            /* wPermissions */
    },

    /* <<Characteristic5>>, .. 0x0D OTA UPDATE APP BANK NUM*/    //---------------------------Device info    7
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,
        },
        1,
        NULL,
        GATT_PERM_READ
    },
    /*  OTA UPDATE BANK NUM characteristic value 0x0E*/
    {
        ATTRIB_FLAG_VALUE_APPL,
        {
            LO_WORD(GATT_UUID_CHAR_DEVICE_INFO),
            HI_WORD(GATT_UUID_CHAR_DEVICE_INFO),
        },
        1,
        (void *)NULL,
        GATT_PERM_READ
    },
    /* <<Characteristic5>>, .. 0x0F OTA IMAGE COUNT TO UPDATE*/ //----------------------NUM OF IMG TO UPDATA  8
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE,
        },
        1,
        NULL,
        GATT_PERM_READ
    },
    /*  OTA IMAGE COUNT TO UPDATE characteristic value 0x10*/
    {
        ATTRIB_FLAG_VALUE_APPL,
        {
            LO_WORD(GATT_UUID_CHAR_IMAGE_COUNT_TO_UPDATE),
            HI_WORD(GATT_UUID_CHAR_IMAGE_COUNT_TO_UPDATE),
        },
        5,
        (void *)NULL,
        GATT_PERM_WRITE
    },

    /* <<Characteristic3>>, .. 11, Patch version*/            //-------------------------image version  9
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            //XXXXMJMJ GATT_CHAR_PROP_INDICATE,                  /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  OTA characteristic value 12*/
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_IMAGE_VERSION),
            HI_WORD(GATT_UUID_CHAR_IMAGE_VERSION),
        },
        1,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },
};




/**
 * @brief write characteristic data from service.
 *
 * @param ServiceID          ServiceID to be written.
 * @param iAttribIndex       Attribute index of characteristic.
 * @param wLength            length of value to be written.
 * @param pValue             value to be written.
 * @return Profile procedure result
*/

T_APP_RESULT ota_attr_write_cb(uint8_t conn_id, uint8_t service_id, uint16_t attrib_index,
                               T_WRITE_TYPE write_type,
                               uint16_t length, uint8_t *p_value, P_FUN_WRITE_IND_POST_PROC *p_write_ind_post_proc)
//T_APP_RESULT  OTAServiceAttrWriteCb(uint8_t ServiceId, uint16_t iAttribIndex, uint16_t wLength, uint8_t *pValue, P_FUN_WRITE_IND_POST_PROC *pWriteIndPostProc)
{
    TOTA_CALLBACK_DATA callback_data;
    T_APP_RESULT  wCause = APP_RESULT_SUCCESS;

    if (BLE_SERVICE_CHAR_OTA_INDEX == attrib_index)
    {
        /* Make sure written value size is valid. */
        if ((length != sizeof(uint8_t)) || (p_value == NULL))
        {
            wCause  = APP_RESULT_INVALID_VALUE_SIZE;
        }
        else
        {
            /* Notify Application. */
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
            callback_data.msg_data.write.opcode = OTA_WRITE_CHAR_VAL;
            callback_data.msg_data.write.u.value = p_value[0];

            if (pfnOTAExtendedCB)
            {
                pfnOTAExtendedCB(service_id, (void *)&callback_data);
            }
        }
    }
    else if (BLE_SERVICE_CHAR_IMAGE_COUNT_INDEX == attrib_index)
    {
        /* Make sure written value size is valid. */
        if ((length != OTA_UPDATE_IMAGE_INFO_LEN) || (p_value == NULL))
        {
            wCause  = APP_RESULT_INVALID_VALUE_SIZE;
        }
        else
        {
            /* Notify Application. */
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
            callback_data.msg_data.write.opcode = OTA_WRITE_IMAGE_COUNT_VAL;
            callback_data.msg_data.write.u.update_image_info.image_count = p_value[0];
            callback_data.msg_data.write.u.update_image_info.update_patch_version = CHAR2SHORT(&p_value[1]);
            callback_data.msg_data.write.u.update_image_info.update_app_version = CHAR2SHORT(&p_value[3]);

            if (pfnOTAExtendedCB)
            {
                pfnOTAExtendedCB(service_id, (void *)&callback_data);
            }
        }
    }
    else if (BLE_SERVICE_CHAR_TEST_MODE_INDEX == attrib_index)
    {
        /* Make sure written value size is valid. */
        if ((length != sizeof(uint8_t)) || (p_value == NULL))
        {
            wCause  = APP_RESULT_INVALID_VALUE_SIZE;
        }
        else
        {
            /* Notify Application. */
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
            callback_data.msg_data.write.opcode = OTA_WRITE_TEST_MODE_CHAR_VAL;
            callback_data.msg_data.write.u.value = p_value[0];

            if (pfnOTAExtendedCB)
            {
                pfnOTAExtendedCB(service_id, (void *)&callback_data);
            }
        }
    }
    else
    {
        PROFILE_PRINT_INFO2("--> OTA_AttrWrite Error  iAttribIndex = 0x%x wLength=%d",
                            attrib_index,
                            length);
        wCause = APP_RESULT_ATTR_NOT_FOUND;
    }
    return wCause;

}

//#define SUPPORT_BANK_SWITCH
/**
 * @brief read characteristic data from service.
 *
 * @param ServiceId          ServiceID of characteristic data.
 * @param iAttribIndex       Attribute index of getting characteristic data.
 * @param iOffset            Used for Blob Read.
 * @param piLength           length of getting characteristic data.
 * @param ppValue            data got from service.
 * @return Profile procedure result
*/
T_APP_RESULT ota_attr_read_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t attrib_index,
                              uint16_t offset, uint16_t *p_length, uint8_t **pp_value)
{
    T_APP_RESULT  wCause = APP_RESULT_SUCCESS;

    switch (attrib_index)
    {
    default:
        PROFILE_PRINT_INFO1("<-- OTA_AttrRead, Attr not found, index=%d", attrib_index);
        wCause  = APP_RESULT_ATTR_NOT_FOUND;
        break;
    case BLE_SERVICE_CHAR_MAC_ADDRESS_INDEX:
        {
            gap_get_param(GAP_PARAM_BD_ADDR, mac_addr);
            uint8_t addr[6];
            for (int i = 0; i < 6; i++)
            {
                addr[i] = mac_addr[5 - i];
            }
            memcpy(mac_addr, addr, 6);
            *pp_value  = (uint8_t *)mac_addr;
            *p_length = sizeof(mac_addr);
        }
        break;
    case BLE_SERVICE_CHAR_PATCH_INDEX:
        {
            T_IMG_HEADER_FORMAT *p_header;
            uint32_t addr = get_header_addr_by_img_id(RomPatch);
            PROFILE_PRINT_INFO1("patch_addr = %x", addr);
            p_header = (T_IMG_HEADER_FORMAT *)addr;

            patch_version = p_header->git_ver.ver_info.version;

            PROFILE_PRINT_INFO1("patch_version = %x", patch_version);
            *pp_value  = (uint8_t *)&patch_version;
            *p_length = sizeof(patch_version);
        }
        break;

    case BLE_SERVICE_CHAR_APP_VERSION_INDEX:
        {
            T_IMG_HEADER_FORMAT *p_header;
            uint32_t addr = get_header_addr_by_img_id(AppPatch);
            PROFILE_PRINT_INFO1("app_addr = %x", addr);
            p_header = (T_IMG_HEADER_FORMAT *)addr;

            app_version = p_header->git_ver.ver_info.version;

            PROFILE_PRINT_INFO1("app_version = %x", app_version);
            *pp_value  = (uint8_t *)&app_version;
            *p_length = sizeof(app_version);
        }
        break;

    case BLE_SERVICE_CHAR_PATCH_EXTENSION_INDEX:        //not used in bee2
        {

        }
        break;
    case BLE_SERVICE_CHAR_DEVICE_INFO_INDEX:
        {
            static T_DFU_DEVICE_INFO DeviceInfo;
            T_IMG_HEADER_FORMAT *p_header;
            uint32_t addr = get_header_addr_by_img_id(OTA);
            p_header = (T_IMG_HEADER_FORMAT *)addr;
            DeviceInfo.secure_version = p_header->ctrl_header.secure_version;

            DeviceInfo.ictype = 5;
            DeviceInfo.ota_version = 0x1;
            DeviceInfo.secure_version = p_header->ctrl_header.secure_version;
            DeviceInfo.mode.buffercheck = (uint8_t)(OTA_BUF_CHECK_EN & 0x01);//buffer_check_en & 0x1;
            DeviceInfo.mode.aesflg = OTP->ota_with_encryption_data;
            DeviceInfo.mode.aesmode = 1;
            // DeviceInfo.mode.verify_algo = 0;//crc    need to delete this feature;
            DeviceInfo.mode.copy_img = 0;
            DeviceInfo.mode.multi_img = is_ota_support_bank_switch() & 0x01;
            DeviceInfo.maxbuffersize = DFU_TEMP_BUFFER_SIZE;
            PROFILE_PRINT_INFO1("otaheader_addr = %x", addr);

            /* prepare img_indicator, if modify here need to sync with vaule
                of BLE_SERVICE_CHAR_IMAGE_VERSION_INDEX */
            if (is_ota_support_bank_switch())
            {
                if ((addr & 0xffffff) == OTA_BANK0_ADDR)
                {
                    DeviceInfo.img_indicator = ((IMAGE_LOCATION_BANK0 << (OTA_HEADER * 2))
                                                | (IMAGE_LOCATION_BANK0 << (SECURE_BOOT * 2))
                                                | (IMAGE_LOCATION_BANK0 << (ROM_PATCH * 2))
                                                | (IMAGE_LOCATION_BANK0 << (APP_IMG * 2))
                                               );
                }
                else
                {
                    DeviceInfo.img_indicator = ((IMAGE_LOCATION_BANK1 << (OTA_HEADER * 2))
                                                | (IMAGE_LOCATION_BANK1 << (SECURE_BOOT * 2))
                                                | (IMAGE_LOCATION_BANK1 << (ROM_PATCH * 2))
                                                | (IMAGE_LOCATION_BANK1 << (APP_IMG * 2))
                                               );
                }
            }
            else
            {
                DeviceInfo.img_indicator = ((IMAGE_FIX_BANK_EXIST << (OTA_HEADER * 2))
                                            | (IMAGE_FIX_BANK_EXIST << (SECURE_BOOT * 2))
                                            | (IMAGE_FIX_BANK_EXIST << (ROM_PATCH * 2))
                                            | (IMAGE_FIX_BANK_EXIST << (APP_IMG * 2))
                                           );
            }
            *pp_value  = (uint8_t *)&DeviceInfo;
            *p_length = sizeof(T_DFU_DEVICE_INFO);
        }
        break;
    case BLE_SERVICE_CHAR_IMAGE_VERSION_INDEX:
        {
            T_IMG_HEADER_FORMAT *p_imgheader;
            static uint32_t version[5];

            /* prepare version info according to img_indicator */
            if (is_ota_support_bank_switch())
            {
                T_OTA_HEADER_FORMAT *p_otaheader;
                uint32_t addr = get_header_addr_by_img_id(OTA);
                p_otaheader = (T_OTA_HEADER_FORMAT *)addr;
                version[0] = p_otaheader->ver_val;

                addr = get_header_addr_by_img_id(SecureBoot);
                p_imgheader = (T_IMG_HEADER_FORMAT *)addr;
                version[1] = p_imgheader->git_ver.ver_info.version;

                addr = get_header_addr_by_img_id(RomPatch);
                p_imgheader = (T_IMG_HEADER_FORMAT *)addr;
                version[2] = p_imgheader->git_ver.ver_info.version;

                addr = get_header_addr_by_img_id(AppPatch);
                p_imgheader = (T_IMG_HEADER_FORMAT *)addr;
                version[3] = p_imgheader->git_ver.ver_info.version;

                APP_PRINT_INFO4("Header V_0x%X, Secure Boot V_0x%X, Patch V_0x%X, APP V_0x%X",
                                version[0], version[1], version[2], version[3]);

                *pp_value  = (uint8_t *)version;
                *p_length = 16;
            }
            else
            {
                T_OTA_HEADER_FORMAT *p_otaheader;
                uint32_t addr = get_header_addr_by_img_id(OTA);
                p_otaheader = (T_OTA_HEADER_FORMAT *)addr;
                version[0] = p_otaheader->ver_val;

                addr = get_header_addr_by_img_id(SecureBoot);
                p_imgheader = (T_IMG_HEADER_FORMAT *)addr;
                version[1] = p_imgheader->git_ver.ver_info.version;

                addr = get_header_addr_by_img_id(RomPatch);
                p_imgheader = (T_IMG_HEADER_FORMAT *)addr;
                version[2] = p_imgheader->git_ver.ver_info.version;

                addr = get_header_addr_by_img_id(AppPatch);
                p_imgheader = (T_IMG_HEADER_FORMAT *)addr;
                version[3] = p_imgheader->git_ver.ver_info.version;

                APP_PRINT_INFO4("Header V_0x%X, Secure Boot V_0x%X, Patch V_0x%X, APP V_0x%X",
                                version[0], version[1], version[2], version[3]);

                *pp_value  = (uint8_t *)version;
                *p_length = 16;
            }
        }
        break;
    }
    return (wCause);
}


/**
 * @brief OTA ble Service Callbacks.
*/
const T_FUN_GATT_SERVICE_CBS OTAServiceCBs =
{
    ota_attr_read_cb,   // Read callback function pointer
    ota_attr_write_cb,  // Write callback function pointer
    NULL                    // CCCD update callback function pointer
};

/**
 * @brief  add OTA ble service to application.
 *
 * @param  pFunc          pointer of app callback function called by profile.
 * @return service ID auto generated by profile layer.
 * @retval ServiceId
*/
uint8_t ota_add_service(void *pFunc)
{
    uint8_t ServiceId;
    if (false == server_add_service(&ServiceId,
                                    (uint8_t *)gattOtaServiceTable,
                                    sizeof(gattOtaServiceTable),
                                    OTAServiceCBs))
    {
        PROFILE_PRINT_ERROR1("OTAService_AddService: ServiceId %d", ServiceId);
        ServiceId = 0xff;
        return ServiceId;
    }
    pfnOTAExtendedCB = (P_FUN_SERVER_GENERAL_CB)pFunc;
    return ServiceId;
}
