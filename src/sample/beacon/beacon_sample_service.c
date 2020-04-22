/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     simple_ble_service.c
* @brief    simple BLE profile source file.
* @details  Demonstration of how to implement a self-definition profile.
* @author
* @date
* @version
*********************************************************************************************************
*/

#include <string.h>
#include <trace.h>
#include <gap.h>
#include "beacon_sample_service.h"
#include <gap_adv.h>
#include <gap_conn_le.h>
#if defined(DEVICE_CONNECTABLE)
#define BEACON_SAMPLE_SERVICE_CHAR_V1_READ_INDEX              0x02
#define BEACON_SAMPLE_SERVICE_CHAR_UUID_WRITE_INDEX           0x05
#define BEACON_SAMPLE_SERVICE_CHAR_MAJOR_WRITE_INDEX          0x07
#define BEACON_SAMPLE_SERVICE_CHAR_MINOR_WRITE_INDEX          0x09
#define BEACON_SAMPLE_SERVICE_CHAR_POWER_WRITE_INDEX          0x0b

T_SERVER_ID beacon_sample_id;
/**<  Value of simple read characteristic. */
static uint8_t beacon_sample_char_read_value[BEACON_READ_V1_MAX_LEN];
static uint16_t beacon_sample_char_read_len = 1;
char v1_user_descr[] = "V1 read characteristic";

/**<  Function pointer used to send event to application from simple profile. Initiated in simp_ble_service_add_service. */
static P_FUN_SERVER_GENERAL_CB pfn_beacon_sample_service_cb = NULL;

/**< @brief  profile/service definition.  */
const T_ATTRIB_APPL beacon_sample_service_tbl[] =
{
    /* <<Primary Service>>, .. */
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_LE),  /* flags     */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),
            LO_WORD(GATT_UUID_BEACON_SAMPLE_PROFILE),      /* service UUID */
            HI_WORD(GATT_UUID_BEACON_SAMPLE_PROFILE)
        },
        UUID_16BIT_SIZE,                            /* bValueLen     */
        NULL,                                       /* p_value_context */
        GATT_PERM_READ                              /* permissions  */
    },
    /* <<Characteristic>> demo for read */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ                     /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHAR_BEACON_SAMPLE_V1_READ),
            HI_WORD(GATT_UUID_CHAR_BEACON_SAMPLE_V1_READ)
        },
        0,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    {
        ATTRIB_FLAG_VOID | ATTRIB_FLAG_ASCII_Z,     /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHAR_USER_DESCR),
            HI_WORD(GATT_UUID_CHAR_USER_DESCR),
        },
        (sizeof(v1_user_descr) - 1),                                           /* bValueLen */
        (void *)v1_user_descr,
        GATT_PERM_READ           /* permissions */
    },
    /* <<Characteristic>> demo for write */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_WRITE | GATT_CHAR_PROP_WRITE_NO_RSP) /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* flags */
        {                                          /* type_value */
            LO_WORD(GATT_UUID_CHAR_BEACON_UUID_WRITE),
            HI_WORD(GATT_UUID_CHAR_BEACON_UUID_WRITE)
        },
        0,                                          /* bValueLen */
        NULL,
        GATT_PERM_WRITE                             /* permissions */
    },
    /* <<Characteristic>> demo for write */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_WRITE | GATT_CHAR_PROP_WRITE_NO_RSP) /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* flags */
        {                                          /* type_value */
            LO_WORD(GATT_UUID_CHAR_BEACON_MAJOR_ID_WRITE),
            HI_WORD(GATT_UUID_CHAR_BEACON_MAJOR_ID_WRITE)
        },
        0,                                          /* bValueLen */
        NULL,
        GATT_PERM_WRITE                             /* permissions */
    },
    /* <<Characteristic>> demo for write */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_WRITE | GATT_CHAR_PROP_WRITE_NO_RSP) /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* flags */
        {                                          /* type_value */
            LO_WORD(GATT_UUID_CHAR_BEACON_MINOR_ID_WRITE),
            HI_WORD(GATT_UUID_CHAR_BEACON_MINOR_ID_WRITE)
        },
        0,                                          /* bValueLen */
        NULL,
        GATT_PERM_WRITE                             /* permissions */
    },
    /* <<Characteristic>> demo for write */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_WRITE | GATT_CHAR_PROP_WRITE_NO_RSP) /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* flags */
        {                                          /* type_value */
            LO_WORD(GATT_UUID_CHAR_BEACON_TX_POWER_WRITE),
            HI_WORD(GATT_UUID_CHAR_BEACON_TX_POWER_WRITE)
        },
        0,                                          /* bValueLen */
        NULL,
        GATT_PERM_WRITE                             /* permissions */
    },
};

/**
  * @brief  Set service related data from application.
  *
  * @param[in] param_type            parameter type to set.
  * @param[in] len                   value length to be set.
  * @param[in] p_value             value to set.
  * @return parameter set result.
  * @retval 0 false
  * @retval 1 true
  */
bool beacon_sample_service_set_parameter(T_BEACON_PARAM_TYPE param_type, uint16_t len,
                                         void *p_value)
{
    bool ret = true;

    switch (param_type)
    {
    default:
        ret = false;
        break;
    case BEACON_SAMPLE_SERVICE_PARAM_V1_READ_CHAR_VAL:
        if (len <= BEACON_READ_V1_MAX_LEN)
        {
            memcpy(beacon_sample_char_read_value, p_value, len);
            beacon_sample_char_read_len = len;
        }
        else
        {
            ret = false;
        }
        break;
    }

    if (!ret)
    {
        APP_PRINT_ERROR0("simp_ble_service_set_parameter failed");
    }

    return ret;
}

/**
 * @brief read characteristic data from service.
 *
 * @param service_id          ServiceID of characteristic data.
 * @param attrib_index        Attribute index of getting characteristic data.
 * @param offset              Used for Blob Read.
 * @param p_length            length of getting characteristic data.
 * @param pp_value            data got from service.
 * @return Profile procedure result
*/
T_APP_RESULT  beacon_sample_service_attr_read_cb(uint8_t conn_id, T_SERVER_ID service_id,
                                                 uint16_t attrib_index, uint16_t offset, uint16_t *p_length, uint8_t **pp_value)
{
    T_APP_RESULT  cause  = APP_RESULT_SUCCESS;

    switch (attrib_index)
    {
    default:
        APP_PRINT_ERROR1("simp_ble_service_attr_read_cb, Attr not found, index %d", attrib_index);
        cause = APP_RESULT_ATTR_NOT_FOUND;
        break;
    case BEACON_SAMPLE_SERVICE_CHAR_V1_READ_INDEX:
        {
            T_BEACON_CALLBACK_DATA callback_data;
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE;
            callback_data.msg_data.read_value_index = BEACON_READ_V1;
            callback_data.conn_id = conn_id;
            if (pfn_beacon_sample_service_cb)
            {
                pfn_beacon_sample_service_cb(service_id, (void *)&callback_data);
            }
            *pp_value = beacon_sample_char_read_value;
            *p_length = beacon_sample_char_read_len;
        }
        break;

    }

    return (cause);
}


void beacon_sample_write_uuid_callback(uint8_t conn_id, T_SERVER_ID service_id,
                                       uint16_t attrib_index,
                                       uint16_t length, uint8_t *p_value)
{
    APP_PRINT_INFO4("beacon_write_post_callback: conn_id %d, service_id %d, attrib_index 0x%x, length %d",
                    conn_id, service_id, attrib_index, length);
    if (length != 16)
    {
        APP_PRINT_INFO0("UUID format error.");
        return;
    }
    for (int i = 0; i < UUID_DATA_LENGTH; i++)
    {
        adv_data[UUID_OFFSET + i] = p_value[i];
    }
    le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data), (void *)adv_data);
    le_disconnect(conn_id);
}

void beacon_sample_write_major_id_callback(uint8_t conn_id, T_SERVER_ID service_id,
                                           uint16_t attrib_index,
                                           uint16_t length, uint8_t *p_value)
{
    APP_PRINT_INFO4("beacon_write_post_callback: conn_id %d, service_id %d, attrib_index 0x%x, length %d",
                    conn_id, service_id, attrib_index, length);
    if (length != 2)
    {
        APP_PRINT_INFO0("ID format error.");
        return;
    }
    for (int i = 0; i < ID_DATA_LENGTH; i++)
    {
        adv_data[MAJOR_ID_OFFSET + i] = p_value[i];
    }
    le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data), (void *)adv_data);
    le_disconnect(conn_id);
}

void beacon_sample_write_minor_id_callback(uint8_t conn_id, T_SERVER_ID service_id,
                                           uint16_t attrib_index,
                                           uint16_t length, uint8_t *p_value)
{
    APP_PRINT_INFO4("beacon_write_post_callback: conn_id %d, service_id %d, attrib_index 0x%x, length %d",
                    conn_id, service_id, attrib_index, length);
    if (length != 2)
    {
        APP_PRINT_INFO0("ID format error.");
        return;
    }
    for (int i = 0; i < ID_DATA_LENGTH; i++)
    {
        adv_data[MINOR_ID_OFFSET + i] = p_value[i];
    }
    le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data), (void *)adv_data);
    le_disconnect(conn_id);
}

void beacon_sample_write_tx_power_callback(uint8_t conn_id, T_SERVER_ID service_id,
                                           uint16_t attrib_index,
                                           uint16_t length, uint8_t *p_value)
{
    APP_PRINT_INFO4("beacon_write_post_callback: conn_id %d, service_id %d, attrib_index 0x%x, length %d",
                    conn_id, service_id, attrib_index, length);
    if (length != 1)
    {
        APP_PRINT_INFO0("TX power format error.");
        return;
    }
    adv_data[RSSI_OFFSET] = p_value[0];
    le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data), (void *)adv_data);
    le_disconnect(conn_id);
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
T_APP_RESULT beacon_sample_service_attr_write_cb(uint8_t conn_id, T_SERVER_ID service_id,
                                                 uint16_t attrib_index, T_WRITE_TYPE write_type, uint16_t length, uint8_t *p_value,
                                                 P_FUN_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    T_BEACON_CALLBACK_DATA callback_data;
    T_APP_RESULT  cause = APP_RESULT_SUCCESS;
    APP_PRINT_INFO1("beacon_sample_service_attr_write_cb write_type = 0x%x", write_type);
    switch (attrib_index)
    {
    case BEACON_SAMPLE_SERVICE_CHAR_UUID_WRITE_INDEX:
        *p_write_ind_post_proc = beacon_sample_write_uuid_callback;
        if (p_value == NULL)
        {
            cause  = APP_RESULT_INVALID_VALUE_SIZE;
        }
        else
        {
            /* Notify Application. */
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
            callback_data.conn_id  = conn_id;
            callback_data.msg_data.write.opcode = BEACON_WRITE_V2;
            callback_data.msg_data.write.write_type = write_type;
            callback_data.msg_data.write.len = length;
            callback_data.msg_data.write.p_value = p_value;

            if (pfn_beacon_sample_service_cb)
            {
                pfn_beacon_sample_service_cb(service_id, (void *)&callback_data);
            }
        }
        break;
    case BEACON_SAMPLE_SERVICE_CHAR_MAJOR_WRITE_INDEX:
        *p_write_ind_post_proc = beacon_sample_write_major_id_callback;
        if (p_value == NULL)
        {
            cause  = APP_RESULT_INVALID_VALUE_SIZE;
        }
        else
        {
            /* Notify Application. */
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
            callback_data.conn_id  = conn_id;
            callback_data.msg_data.write.opcode = BEACON_WRITE_V2;
            callback_data.msg_data.write.write_type = write_type;
            callback_data.msg_data.write.len = length;
            callback_data.msg_data.write.p_value = p_value;

            if (pfn_beacon_sample_service_cb)
            {
                pfn_beacon_sample_service_cb(service_id, (void *)&callback_data);
            }
        }
        break;
    case BEACON_SAMPLE_SERVICE_CHAR_MINOR_WRITE_INDEX:
        *p_write_ind_post_proc = beacon_sample_write_minor_id_callback;
        if (p_value == NULL)
        {
            cause  = APP_RESULT_INVALID_VALUE_SIZE;
        }
        else
        {
            /* Notify Application. */
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
            callback_data.conn_id  = conn_id;
            callback_data.msg_data.write.opcode = BEACON_WRITE_V2;
            callback_data.msg_data.write.write_type = write_type;
            callback_data.msg_data.write.len = length;
            callback_data.msg_data.write.p_value = p_value;

            if (pfn_beacon_sample_service_cb)
            {
                pfn_beacon_sample_service_cb(service_id, (void *)&callback_data);
            }
        }
        break;
    case BEACON_SAMPLE_SERVICE_CHAR_POWER_WRITE_INDEX:
        *p_write_ind_post_proc = beacon_sample_write_tx_power_callback;
        if (p_value == NULL)
        {
            cause  = APP_RESULT_INVALID_VALUE_SIZE;
        }
        else
        {
            /* Notify Application. */
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
            callback_data.conn_id  = conn_id;
            callback_data.msg_data.write.opcode = BEACON_WRITE_V2;
            callback_data.msg_data.write.write_type = write_type;
            callback_data.msg_data.write.len = length;
            callback_data.msg_data.write.p_value = p_value;

            if (pfn_beacon_sample_service_cb)
            {
                pfn_beacon_sample_service_cb(service_id, (void *)&callback_data);
            }
        }
        break;
    default:
        APP_PRINT_ERROR2("beacon_sample_service_attr_write_cb Error: attrib_index 0x%x, length %d",
                         attrib_index,
                         length);
        cause = APP_RESULT_ATTR_NOT_FOUND;
        break;
    }
    if (BEACON_SAMPLE_SERVICE_CHAR_UUID_WRITE_INDEX == attrib_index)
    {
        /* Make sure written value size is valid. */

    }
    else
    {

    }
    return cause;
}


/**
 * @brief Simple ble Service Callbacks.
*/
const T_FUN_GATT_SERVICE_CBS beacon_sample_service_cbs =
{
    beacon_sample_service_attr_read_cb,  // Read callback function pointer
    beacon_sample_service_attr_write_cb, // Write callback function pointer
    NULL, // CCCD update callback function pointer
};

/**
  * @brief Add simple BLE service to the BLE stack database.
  *
  * @param[in]   p_func  Callback when service attribute was read, write or cccd update.
  * @return Service id generated by the BLE stack: @ref T_SERVER_ID.
  * @retval 0xFF Operation failure.
  * @retval others Service id assigned by stack.
  *
  */
T_SERVER_ID beacon_sample_add_service(void *p_func)
{
    if (false == server_add_service(&beacon_sample_id,
                                    (uint8_t *)beacon_sample_service_tbl,
                                    sizeof(beacon_sample_service_tbl),
                                    beacon_sample_service_cbs))
    {
        APP_PRINT_ERROR0("beacon_sample_service_add_service: fail");
        beacon_sample_id = 0xff;
        return beacon_sample_id;
    }

    pfn_beacon_sample_service_cb = (P_FUN_SERVER_GENERAL_CB)p_func;
    return beacon_sample_id;
}
#endif
