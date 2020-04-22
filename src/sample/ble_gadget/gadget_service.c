/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     amale_ble_service.c
* @brief    amale BLE profile source file.
* @details  Demonstration of how to implement a self-definition profile.
* @author
* @date
* @version
*********************************************************************************************************
*/

#include <string.h>
#include <trace.h>
#include <gadget_service.h>
#include <gap.h>
#include <mcu_vendor\ble_ama.h>
#include "AMATransport.h"

static const uint8_t GATT_UUID128_AMA_SRV[16] = { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x03, 0xFE, 0x00, 0x00 };
#define  GATT_UUID128_AMA_TX  0x76, 0x30, 0xf8,0xdd,0x90, 0xa3,0x61,0xac,0xa7,0x43,0x05,0x30,0x77,0xb1,0x4e,0xf0
#define  GATT_UUID128_AMA_RX  0x0b, 0x42, 0x82,0x1f,0x64, 0x72,0x2f,0x8a,0xb4,0x4b,0x79,0x18,0x5b,0xa0,0xee,0x2b



extern const AMATransportInterfaces *AMATransport_GetInterfaces(void);

T_SERVER_ID ama_service_id;
/**<  Value of amale read characteristic. */
static uint8_t amale_char_read_value[GADGET_READ_MAX_LEN];
static uint16_t amale_char_read_len = 1;
char v1_user_descr[] = "V1 read characteristic";

/**<  Function pointer used to send event to application from amale profile. Initiated in ama_ble_service_add_service. */
static P_FUN_SERVER_GENERAL_CB pfn_ama_ble_service_cb = NULL;

/**< @brief  profile/service definition.  */
static T_ATTRIB_APPL ama_attr_tbl[] =
{
    /*--------------------------Alexa Service ---------------------------*/
    /* <<Primary Service>>, .. 0 */
    {
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),  /* wFlags     */
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),     /* bTypeValue */
        },
        UUID_128BIT_SIZE,                     /* bValueLen     */
        (void *)GATT_UUID128_AMA_SRV,        /* pValueContext */
        GATT_PERM_READ                              /* wPermissions  */
    },
//Alexa Characteristic Tx

    /* <<Characteristic>>, .. 1*/
    {
        ATTRIB_FLAG_VALUE_INCL,                   /* flags */
        {                                        /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE,            /* characteristic properties */
            //XXXXMJMJ GATT_CHAR_PROP_INDICATE,                  /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                        /* bValueLen */
        NULL,
        GATT_PERM_READ                            /* permissions */
    },

    /* characteristic value ,Alexa service input--- 2*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                   /* flags */
        {                                        /* type_value */
            GATT_UUID128_AMA_TX
        },
        255,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_WRITE_ENCRYPTED_REQ               /* permissions */
    },
//Alexa Characteristic Rx
    /* <<Characteristic>>,  .. 3*/
    {
        ATTRIB_FLAG_VALUE_INCL,                   /* flags */
        {                                        /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_NOTIFY | GATT_CHAR_PROP_READ,           /* characteristic properties */
        },
        1,                                        /* bValueLen */
        NULL,
        GATT_PERM_READ                            /* permissions */
    },

    /*  characteristic value ,Alexa service output--- 4 */
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                   /* flags */
        {                                        /* type_value */
            GATT_UUID128_AMA_RX
        },
        255,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ_ENCRYPTED_REQ             /* permissions */
    },

    /* client characteristic configuration ----5*/
    {
        (ATTRIB_FLAG_VALUE_INCL |
         ATTRIB_FLAG_CCCD_APPL | ATTRIB_FLAG_CCCD_NO_FILTER),
        {
            LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            LO_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT),
            HI_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT)
        },
        2,
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)
    }
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
bool ama_ble_service_set_parameter(T_GADGET_PARAM_TYPE param_type, uint16_t len, void *p_value)
{
    bool ret = true;

    switch (param_type)
    {
    default:
        ret = false;
        break;
    case GADGET_BLE_SERVICE_PARAM_READ_CHAR_VAL:
        if (len <= GADGET_READ_MAX_LEN)
        {
            memcpy(amale_char_read_value, p_value, len);
            amale_char_read_len = len;
        }
        else
        {
            ret = false;
        }
        break;
    }

    if (!ret)
    {
        APP_PRINT_ERROR0("ama_ble_service_set_parameter failed");
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
T_APP_RESULT  ama_ble_service_attr_read_cb(uint8_t conn_id, T_SERVER_ID service_id,
                                           uint16_t attrib_index, uint16_t offset, uint16_t *p_length, uint8_t **pp_value)
{
    T_APP_RESULT  cause  = APP_RESULT_SUCCESS;

    switch (attrib_index)
    {
    default:
        APP_PRINT_ERROR1("ama_ble_service_attr_read_cb, Attr not found, index %d", attrib_index);
        cause = APP_RESULT_ATTR_NOT_FOUND;
        break;
    case GADGET_SERVICE_CHAR_READ_INDEX:
        {
            T_GADGET_CALLBACK_DATA callback_data;
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE;
            callback_data.msg_data.read_value_index = GADGET_READ;
            callback_data.conn_id = conn_id;
            if (pfn_ama_ble_service_cb)
            {
                pfn_ama_ble_service_cb(service_id, (void *)&callback_data);
            }
            *pp_value = amale_char_read_value;
            *p_length = amale_char_read_len;
        }
        break;

    }

    return (cause);
}


void ama_write_post_callback(uint8_t conn_id, T_SERVER_ID service_id, uint16_t attrib_index,
                             uint16_t length, uint8_t *p_value)
{
    APP_PRINT_INFO4("ama_write_post_callback: conn_id %d, service_id %d, attrib_index 0x%x, length %d",
                    conn_id, service_id, attrib_index, length);
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
T_APP_RESULT ama_ble_service_attr_write_cb(uint8_t conn_id, T_SERVER_ID service_id,
                                           uint16_t attrib_index, T_WRITE_TYPE write_type, uint16_t length, uint8_t *p_value,
                                           P_FUN_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    T_GADGET_CALLBACK_DATA callback_data;
    T_APP_RESULT  cause = APP_RESULT_SUCCESS;
    APP_PRINT_INFO1("ama_ble_service_attr_write_cb write_type = 0x%x", write_type);
    *p_write_ind_post_proc = ama_write_post_callback;
    if (GADGET_SERVICE_CHAR_WRITE_INDEX == attrib_index)
    {
        /* Make sure written value size is valid. */
        if (p_value == NULL)
        {
            cause  = APP_RESULT_INVALID_VALUE_SIZE;
        }
        else
        {
            /* Notify Application. */
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
            callback_data.conn_id  = conn_id;
            callback_data.msg_data.write.opcode = GADGET_WRITE;
            callback_data.msg_data.write.write_type = write_type;
            callback_data.msg_data.write.len = length;
            callback_data.msg_data.write.p_value = p_value;

            if (pfn_ama_ble_service_cb)
            {
                pfn_ama_ble_service_cb(service_id, (void *)&callback_data);
            }
        }
    }
    else
    {
        APP_PRINT_ERROR2("ama_ble_service_attr_write_cb Error: attrib_index 0x%x, length %d",
                         attrib_index,
                         length);
        cause = APP_RESULT_ATTR_NOT_FOUND;
    }
    return cause;
}

/**
  * @brief send notification of amale notify characteristic value.
  *
  * @param[in] conn_id           connection id
  * @param[in] service_id        service ID of service.
  * @param[in] p_value           characteristic value to notify
  * @param[in] length            characteristic value length to notify
  * @return notification action result
  * @retval 1 true
  * @retval 0 false
  */
bool ama_ble_service_send_v3_notify(uint8_t conn_id, T_SERVER_ID service_id, void *p_value,
                                    uint16_t length)
{
    APP_PRINT_INFO0("ama_ble_service_send_v3_notify");
    // send notification to client
    return server_send_data(conn_id, service_id, GADGET_SERVICE_CHAR_NOTIFY_INDEX, p_value,
                            length,
                            GATT_PDU_TYPE_ANY);
}

/**
 * @brief update CCCD bits from stack.
 *
 * @param conn_id           connection id.
 * @param service_id          Service ID.
 * @param index          Attribute index of characteristic data.
 * @param cccbits         CCCD bits from stack.
 * @return None
*/
void ama_ble_service_cccd_update_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t index,
                                    uint16_t cccbits)
{
    T_GADGET_CALLBACK_DATA callback_data;
    bool is_handled = false;
    callback_data.conn_id = conn_id;
    callback_data.msg_type = SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION;
    APP_PRINT_INFO2("ama_ble_service_cccd_update_cb: index = %d, cccbits 0x%x", index, cccbits);
    switch (index)
    {
    case GADGET_SERVICE_CHAR_NOTIFY_CCCD_INDEX:
        {
            if (cccbits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
            {
                // Enable Notification
                callback_data.msg_data.notification_indification_index = GADGET_NOTIFY_ENABLE;
            }
            else
            {
                // Disable Notification
                callback_data.msg_data.notification_indification_index = GADGET_NOTIFY_DISABLE;
            }
            is_handled =  true;
        }
        break;

    default:
        break;
    }
    /* Notify Application. */
    if (pfn_ama_ble_service_cb && (is_handled == true))
    {
        pfn_ama_ble_service_cb(service_id, (void *)&callback_data);
    }
}

/**
 * @brief Simple ble Service Callbacks.
*/
const T_FUN_GATT_SERVICE_CBS ama_ble_service_cbs =
{
    ama_ble_service_attr_read_cb,  // Read callback function pointer
    ama_ble_service_attr_write_cb, // Write callback function pointer
    ama_ble_service_cccd_update_cb // CCCD update callback function pointer
};

/**
  * @brief Add amale BLE service to the BLE stack database.
  *
  * @param[in]   p_func  Callback when service attribute was read, write or cccd update.
  * @return Service id generated by the BLE stack: @ref T_SERVER_ID.
  * @retval 0xFF Operation failure.
  * @retval others Service id assigned by stack.
  *
  */
T_SERVER_ID ama_ble_service_add_service(void *p_func)
{
    if (false == server_add_service(&ama_service_id,
                                    (uint8_t *)ama_attr_tbl,
                                    sizeof(ama_attr_tbl),
                                    ama_ble_service_cbs))
    {
        APP_PRINT_ERROR0("ama_ble_service_add_service: fail");
        ama_service_id = 0xff;
        return ama_service_id;
    }

    pfn_ama_ble_service_cb = (P_FUN_SERVER_GENERAL_CB)p_func;
    return ama_service_id;
}

static void ble_ama_send_data_cb(AMAAddress *address, uint8_t *data, uint16_t len)
{
    ble_ama_t *p_ama = NULL;
    DBG_DIRECT("data sent entire length = %d", len);
//    if(len == 44)
//        APP_PRINT_ERROR1("Send result %b", TRACE_BINARY(len, data));
    memcpy((char *)&p_ama, address->raw, sizeof(intptr_t));
    //APP_PRINT_ERROR1("%b", TRACE_BINARY(len, data));
    uint32_t err_code = server_send_data(p_ama->conn_handle, p_ama->service_handle,
                                         GADGET_SERVICE_CHAR_NOTIFY_INDEX,
                                         data, len, GATT_PDU_TYPE_ANY);
    //return err_code;
}

void bleAMATransport(void)
{
    static AMATransportCallbacks callbacks =
    {
        NULL,
        NULL,
        NULL,
        NULL,
        ble_ama_send_data_cb
    };

    AMATransport_GetInterfaces()->Register(AMA_TRANSPORT_BTLE, &callbacks);
    AMATransport_GetInterfaces()->AdapterStateChanged(AMA_TRANSPORT_BTLE, true);
}
