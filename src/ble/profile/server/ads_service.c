/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     ads_service.c
* @brief    Audio down streaming service source file.
* @details  Interfaces to access ADS service.
* @author   Chenjie Jin
* @date     2018-4-6
* @version  v0.1
*********************************************************************************************************
*/
#include "stdint.h"
#include "stddef.h"
#include "string.h"
#include "trace.h"
#include "profile_server.h"
#include "ads_service.h"
#include "board.h"

/********************************************************************************************************
* local static variables defined here, only used in this source file.
********************************************************************************************************/
static P_FUN_SERVER_GENERAL_CB pfn_ads_cb = NULL;

T_ADS_GLOBAL_DATA ads_global_data;

const uint8_t GATT_UUID_ADS_SERVICE[16] = {0x12, 0xA2, 0x4D, 0x2E, 0xFE, 0x14, 0x48, 0x8e, 0x93, 0xD2, 0x17, 0x3C, 0xFD, 0x04, 0x00, 0x00};

/**< @brief  profile/service definition.  */
const T_ATTRIB_APPL ads_attr_tbl[] =
{
    /* <<Primary Service>>, .. 0*/
    {
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),  /* wFlags     */
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),     /* bTypeValue */
        },
        UUID_128BIT_SIZE,                     /* bValueLen     */
        (void *)GATT_UUID_ADS_SERVICE,        /* pValueContext */
        GATT_PERM_READ                              /* wPermissions  */
    },

    /* ADS_CHAR_RX */
    /* Characteristic, 1 */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {                                           /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ | GATT_CHAR_PROP_NOTIFY,                    /* characteristic properties */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },

    /* Characteristic Value, 2 */
    {
        ATTRIB_FLAG_VALUE_APPL,                              /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_ADS_CHAR_RX),
            HI_WORD(GATT_UUID_ADS_CHAR_RX),
        },
        0,                                                 /* bValueLen */
        NULL,
        GATT_PERM_READ                 /* wPermissions */
    },

    /* client characteristic configuration, 3 */
    {
        (ATTRIB_FLAG_VALUE_INCL |                   /* wFlags */
         ATTRIB_FLAG_CCCD_APPL),
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            /* NOTE: this value has an instantiation for each client, a write to */
            /* this attribute does not modify this default value:                */
            LO_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT), /* client char. config. bit field */
            HI_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT)
        },
        2,                                          /* bValueLen */
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)          /* wPermissions */
    },

    /* ADS_CHAR_TX */
    /* Characteristic Definition, 4 */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {                                           /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE_NO_RSP,                   /* characteristic properties */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },

    /* Characteristic Value, 5 */
    {
        ATTRIB_FLAG_VALUE_APPL,                              /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_ADS_CHAR_TX),
            HI_WORD(GATT_UUID_ADS_CHAR_TX),
        },
        0,                                                 /* bValueLen */
        NULL,
        GATT_PERM_WRITE                 /* wPermissions */
    },

    /* ADS_CHAR_CTL */
    /* Characteristic Definition, 6 */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {                                           /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE,                   /* characteristic properties */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },

    /* Characteristic Value, 7 */
    {
        ATTRIB_FLAG_VALUE_APPL,                              /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_ADS_CHAR_CTL),
            HI_WORD(GATT_UUID_ADS_CHAR_CTL),
        },
        0,                                                 /* bValueLen */
        NULL,
        GATT_PERM_WRITE                 /* wPermissions */
    }
};

/**< @brief  ADS service size definition.  */
const static uint16_t ads_attr_tbl_size = sizeof(ads_attr_tbl);

/**
 * @brief read characteristic data from service.
 *
 * @param conn_id           Connection ID.
 * @param service_id        ServiceID to be read.
 * @param attrib_index      Attribute index of getting characteristic data.
 * @param offset            Offset of characteritic to be read.
 * @param p_length          Length of getting characteristic data.
 * @param pp_value          Pointer to pointer of characteristic value to be read.
 * @return T_APP_RESULT
*/
T_APP_RESULT ads_attr_read_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t attrib_index,
                              uint16_t offset, uint16_t *p_length, uint8_t **pp_value)
{
    T_APP_RESULT cause = APP_RESULT_SUCCESS;
    *p_length = 0;

    PROFILE_PRINT_INFO2("ads_attr_read_cb attrib_index = %d offset %x", attrib_index, offset);

    switch (attrib_index)
    {
    default:
        {
            PROFILE_PRINT_ERROR1("ads_attr_read_cb attrib_index = %d not found", attrib_index);
            cause  = APP_RESULT_ATTR_NOT_FOUND;
        }
        break;

    case GATT_SVC_ADS_CHAR_RX_VALUE_INDEX:
        {
            T_ADS_CALLBACK_DATA callback_data;
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE;
            callback_data.conn_id = conn_id;
            callback_data.msg_data.read_value_index = ADS_READ_CHAR_RX_INDEX;
            cause = pfn_ads_cb(service_id, (void *)&callback_data);
            if (cause == APP_RESULT_PENDING)
            {
                memset(ads_global_data.char_rx_data_buff, 0, sizeof(ads_global_data.char_rx_data_buff));
            }

            *pp_value = ads_global_data.char_rx_data_buff;
            *p_length = sizeof(ads_global_data.char_rx_data_buff);
        }
        break;
    }
    return (cause);
}


/**
 * @brief update CCCD bits from stack.
 *
 * @param conn_id           Connection ID.
 * @param service_id        Service ID.
 * @param index             Attribute index of characteristic data.
 * @param ccc_bits          CCCD bits from stack.
 * @return None
*/
void ads_cccd_update_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t index, uint16_t ccc_bits)
{
    T_ADS_CALLBACK_DATA callback_data;
    callback_data.msg_type = SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION;
    callback_data.conn_id = conn_id;
    bool handle = true;
    PROFILE_PRINT_INFO2("ads_cccd_update_cb index = %d ccc_bits %x", index, ccc_bits);

    switch (index)
    {
    case GATT_SVC_ADS_CHAR_RX_CCCD_INDEX:
        {
            if (ccc_bits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
            {
                callback_data.msg_data.notification_indification_index = ADS_CHAR_RX_NOTIFY_INDICATE_ENABLE;
            }
            else
            {
                callback_data.msg_data.notification_indification_index = ADS_CHAR_RX_NOTIFY_INDICATE_DISABLE;
            }
            break;
        }
    default:
        {
            handle = false;
            break;
        }

    }

    if (pfn_ads_cb && (handle == true))
    {
        pfn_ads_cb(service_id, (void *)&callback_data);
    }

    return;
}

void ads_write_post_callback(uint8_t conn_id, T_SERVER_ID service_id, uint16_t attrib_index,
                             uint16_t length, uint8_t *p_value)
{
    T_ADS_CALLBACK_DATA callback_data;

    APP_PRINT_INFO4("ads_write_post_callback: conn_id %d, service_id %d, attrib_index 0x%x, length %d",
                    conn_id, service_id, attrib_index, length);

    if (GATT_SVC_ADS_CHAR_TX_VALUE_INDEX == attrib_index)
    {
        /* Notify Application. */
        callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
        callback_data.conn_id  = conn_id;
        callback_data.msg_data.write.write_type = ADS_WRITE_CHAR_TX_INDEX;
        callback_data.msg_data.write.write_parameter.len = length;
        callback_data.msg_data.write.write_parameter.p_value = p_value;

        if (pfn_ads_cb)
        {
            pfn_ads_cb(service_id, (void *)&callback_data);
        }
    }
    else if (GATT_SVC_ADS_CHAR_CTL_VALUE_INDEX == attrib_index)
    {
        /* Notify Application. */
        callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
        callback_data.conn_id  = conn_id;
        callback_data.msg_data.write.write_type = ADS_WRITE_CHAR_CTL_INDEX;
        callback_data.msg_data.write.write_parameter.len = length;
        callback_data.msg_data.write.write_parameter.p_value = p_value;

        if (pfn_ads_cb)
        {
            pfn_ads_cb(service_id, (void *)&callback_data);
        }
    }
}

/**
 * @brief write characteristic data from service.
 *
 * @param conn_id
 * @param service_id        ServiceID to be written.
 * @param attrib_index      Attribute index of characteristic.
 * @param length            length of value to be written.
 * @param p_value           value to be written.
 * @return Profile procedure result
*/
T_APP_RESULT ads_attr_write_cb(uint8_t conn_id, T_SERVER_ID service_id,
                               uint16_t attrib_index, T_WRITE_TYPE write_type, uint16_t length, uint8_t *p_value,
                               P_FUN_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    T_APP_RESULT  cause = APP_RESULT_SUCCESS;
    APP_PRINT_INFO1("ads_attr_write_cb write_type = 0x%x", write_type);
    *p_write_ind_post_proc = ads_write_post_callback;
    if (GATT_SVC_ADS_CHAR_TX_VALUE_INDEX == attrib_index)
    {
        /* Make sure written value size is valid. */
        if (p_value == NULL)
        {
            cause  = APP_RESULT_INVALID_VALUE_SIZE;
        }
        else
        {
            /* Wait for ads_write_post_callback. */
        }
    }
    else if (GATT_SVC_ADS_CHAR_CTL_VALUE_INDEX == attrib_index)
    {
        /* Make sure written value size is valid. */
        if (p_value == NULL)
        {
            cause  = APP_RESULT_INVALID_VALUE_SIZE;
        }
        else
        {
            /* Wait for ads_write_post_callback. */
        }
    }
    else
    {
        APP_PRINT_ERROR2("ads_attr_write_cb Error: attrib_index 0x%x, length %d",
                         attrib_index,
                         length);
        cause = APP_RESULT_ATTR_NOT_FOUND;
    }
    return cause;
}

/**
 * @brief ADS Service Callbacks.
*/
const T_FUN_GATT_SERVICE_CBS ads_cbs =
{
    ads_attr_read_cb,  // Read callback function pointer
    ads_attr_write_cb, // Write callback function pointer
    ads_cccd_update_cb  // CCCD update callback function pointer
};

/**
  * @brief       Add ADS service to the BLE stack database.
  *
  *
  * @param[in]   p_func  Callback when service attribute was read, write or cccd update.
  * @return Service id generated by the BLE stack: @ref T_SERVER_ID.
  * @retval 0xFF Operation failure.
  * @retval Others Service id assigned by stack.
  *
  * <b>Example usage</b>
  * \code{.c}
     void profile_init()
     {
         server_init(1);
         ads_id = ads_add_service(app_handle_profile_message);
     }
  * \endcode
  */
T_SERVER_ID ads_add_service(void *p_func)
{
    T_SERVER_ID service_id;
    if (false == server_add_service(&service_id,
                                    (uint8_t *)ads_attr_tbl,
                                    ads_attr_tbl_size,
                                    ads_cbs))
    {
        PROFILE_PRINT_ERROR1("ads_add_service: service_id %d", service_id);
        service_id = 0xff;
    }
    pfn_ads_cb = (P_FUN_SERVER_GENERAL_CB)p_func;
    return service_id;
}

