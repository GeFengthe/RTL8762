/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     pos.c
* @brief    Service source file.
* @details  Interfaces to access service.
* @author
* @date
* @version  v1.0
*********************************************************************************************************
*/
#include <os_timer.h>
#include "stdint.h"
#include "stddef.h"
#include "string.h"
#include "trace.h"
#include "profile_server.h"
#include "pos.h"
#include "pos_config.h"
#include "icm20618_driver.h"

/********************************************************************************************************
* local static variables defined here, only used in this source file.
********************************************************************************************************/

#define GATT_UUID_POSTURE                       0x1825
#define GATT_UUID_CHAR_POS_LEVEL                0x2A5D

#define GATT_SVC_POSTURE_LEVEL_INDEX        2 /**< @brief Index for posture level chars's value */
#define GATT_SVC_POS_CHAR_CCCD_INDEX            3 /**< @brief CCCD Index for posture level chars's value */

uint8_t notify_service_id;
uint8_t send_data[6];
/**<  Function pointer used to send event to application from pos profile. */
/**<  Initiated in pos_AddService. */
static P_FUN_SERVER_GENERAL_CB pfn_pos_cb = NULL;

/**< @brief  profile/service definition.  */
static const T_ATTRIB_APPL pos_attr_tbl[] =
{
    /*----------------- Service -------------------*/
    /* <<Primary Service>>, .. */
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_LE),   /* flags     */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),
            LO_WORD(GATT_UUID_POSTURE),              /* service UUID */
            HI_WORD(GATT_UUID_POSTURE)
        },
        UUID_16BIT_SIZE,                            /* bValueLen     */
        NULL,                                       /* p_value_context */
        GATT_PERM_READ                              /* permissions  */
    },

    /* <<Characteristic>>, .. */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
#if POS_NOTIFY_SUPPORT
            (GATT_CHAR_PROP_READ |                    /* characteristic properties */
             GATT_CHAR_PROP_NOTIFY)
#else
            GATT_CHAR_PROP_READ
#endif
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    /* Posture Level value */
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHAR_POS_LEVEL),
            HI_WORD(GATT_UUID_CHAR_POS_LEVEL)
        },
        0,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    }
#if POSTURE_NOTIFY_SUPPORT
    ,
    /* client characteristic configuration */
    {
        ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL,                   /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            /* NOTE: this value has an instantiation for each client, a write to */
            /* this attribute does not modify this default value:                */
            LO_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT), /* client char. config. bit field */
            HI_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT)
        },
        2,                                          /* bValueLen */
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)          /* permissions */
    }
#endif
};
/**< @brief  Service size definition.  */
const static uint16_t pos_attr_tbl_size = sizeof(pos_attr_tbl);

/**
 * @brief       Set a service parameter.
 *
 *              NOTE: You can call this function with a service parameter type and it will set the
 *                      battery service parameter.  Service parameters are defined in @ref T_POS_PARAM_TYPE.
 *                      If the "len" field sets to the size of a "uint16_t" ,the
 *                      "p_value" field must point to a data with type of "uint16_t".
 *
 * @param[in]   param_type   POS service parameter type: @ref T_POS_PARAM_TYPE
 * @param[in]   length       Length of data to write
 * @param[in]   p_value Pointer to data to write.  This is dependent on
 *                      the parameter type and WILL be cast to the appropriate
 *                      data type (For example: if data type of param is uint16_t, p_value will be cast to
 *                      pointer of uint16_t).
 *
 * @return Operation result.
 * @retval true Operation success.
 * @retval false Operation failure.
 *
 * <b>Example usage</b>
 * \code{.c}
 * \endcode
 */
bool pos_set_parameter(T_POS_PARAM_TYPE param_type, uint8_t length, uint8_t *p_value)
{
    bool ret = true;

    switch (param_type)
    {
    default:
        {
            ret = false;
            PROFILE_PRINT_ERROR0("pos_set_parameter failed");
        }
        break;

    case POS_PARAM_LEVEL:
        {
            if (length != sizeof(uint8_t))
            {
                ret = false;
            }
            else
            {
            }
        }
        break;
    }

    return ret;
}

/**
 * @brief       Send notify posture level notification data .
 *
 * @param[in]   conn_id  Connection id.
 * @param[in]   service_id  Service id.
 * @param[in]   posture_level  posture level value.
 * @return Operation result.
 * @retval true Operation success.
 * @retval false Operation failure.
 *
 * <b>Example usage</b>
 * \code{.c}
    void test(void)
    {
        uint8_t posture_level = 90;
        pos_posture_level_value_notify(conn_id, pos_id, posture_level);
    }
 * \endcode
 */
bool pos_value_notify(uint8_t conn_id, uint8_t service_id, uint8_t posture_level)
{
    return server_send_data(conn_id, service_id, GATT_SVC_POSTURE_LEVEL_INDEX, &posture_level,
                            sizeof(posture_level), GATT_PDU_TYPE_ANY);
}

/**
 * @brief       Confirm for read value request.
 *
 * @param[in]   conn_id  Callback when service attribute was read/write.
 * @param[in]   service_id  Callback when service attribute was read/write.
 * @param[in]   posture_level  Callback when service attribute was read/write.
 * @return Operation result.
 * @retval true Operation success.
 * @retval false Operation failure.
 * \endcode
 */
bool pos_value_read_confirm(uint8_t conn_id, uint8_t service_id,
                            uint8_t posture_level)
{
    return server_attr_read_confirm(conn_id, service_id, GATT_SVC_POSTURE_LEVEL_INDEX,
                                    &posture_level, sizeof(posture_level), APP_RESULT_SUCCESS);
}

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
T_APP_RESULT pos_attr_read_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t attrib_index,
                              uint16_t offset, uint16_t *p_length, uint8_t **pp_value)
{
    T_APP_RESULT cause = APP_RESULT_SUCCESS;
    *p_length = 0;
    int16_t ACC_DATA[3];
    int16_t GYRO_DATA[3];

    PROFILE_PRINT_INFO2("pos_attr_read_cb attrib_index = %d offset %x", attrib_index, offset);

    switch (attrib_index)
    {
    default:
        {
            PROFILE_PRINT_ERROR1("pos_attr_read_cb attrib_index = %d not found", attrib_index);
            cause  = APP_RESULT_ATTR_NOT_FOUND;
        }
        break;

    case GATT_SVC_POSTURE_LEVEL_INDEX:
        {
            T_POS_CALLBACK_DATA callback_data;
            callback_data.msg_type = SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE;
            callback_data.conn_id = conn_id;
            callback_data.msg_data.read_value_index = POS_READ_LEVEL;
            cause = pfn_pos_cb(service_id, (void *)&callback_data);
            ICM20618_Acc_Read(ACC_DATA);
            ICM20618_GYRO_Read(GYRO_DATA);
            APP_PRINT_INFO3("icm20618 ACC_DATA: X:%d  Y:%d  Z:%d", ACC_DATA[0], ACC_DATA[1], ACC_DATA[2]);
            APP_PRINT_INFO3("icm20618 GYRO_DATA: X:%d  Y:%d  Z:%d", GYRO_DATA[0], GYRO_DATA[1], GYRO_DATA[2]);
            send_data[0] = (uint8_t)(ACC_DATA[0] >> 8);
            send_data[1] = (uint8_t)(ACC_DATA[0] & 0x00ff);
            send_data[2] = (uint8_t)(ACC_DATA[1] >> 8);
            send_data[3] = (uint8_t)(ACC_DATA[1] & 0x00ff);
            send_data[4] = (uint8_t)(ACC_DATA[2] >> 8);
            send_data[5] = (uint8_t)(ACC_DATA[2] & 0x00ff);
            *pp_value = send_data;
            *p_length = 6 * sizeof(uint8_t);
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
void pos_cccd_update_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t index, uint16_t ccc_bits)
{
    T_POS_CALLBACK_DATA callback_data;
    callback_data.msg_type = SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION;
    callback_data.conn_id = conn_id;
    bool handle = true;
    PROFILE_PRINT_INFO2("pos_cccd_update_cb index = %d ccc_bits %x", index, ccc_bits);

    switch (index)
    {
    case GATT_SVC_POS_CHAR_CCCD_INDEX:
        {
            if (ccc_bits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
            {
                callback_data.msg_data.notification_indification_index = POS_NOTIFY_LEVEL_ENABLE;
            }
            else
            {
                callback_data.msg_data.notification_indification_index = POS_NOTIFY_LEVEL_DISABLE;
            }
            break;
        }
    default:
        {
            handle = false;
            break;
        }

    }

    if (pfn_pos_cb && (handle == true))
    {
        pfn_pos_cb(service_id, (void *)&callback_data);
    }

    return;
}

/**
 * @brief POS Service Callbacks.
*/
const T_FUN_GATT_SERVICE_CBS pos_cbs =
{
    pos_attr_read_cb,  // Read callback function pointer
    NULL, // Write callback function pointer
    pos_cccd_update_cb  // CCCD update callback function pointer
};

/**
  * @brief       Add service to the BLE stack database.
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
         pos_id = pos_add_service(app_handle_profile_message);
     }
  * \endcode
  */
T_SERVER_ID pos_add_service(void *p_func)
{
    T_SERVER_ID service_id;
    if (false == server_add_service(&service_id,
                                    (uint8_t *)pos_attr_tbl,
                                    pos_attr_tbl_size,
                                    pos_cbs))
    {
        PROFILE_PRINT_ERROR1("pos_add_service: service_id %d", service_id);
        service_id = 0xff;
    }
    pfn_pos_cb = (P_FUN_SERVER_GENERAL_CB)p_func;
    notify_service_id = service_id;
    return service_id;
}
void vTimerPOSCallback(TimerHandle_t pxTimer)
{
    int16_t ACC_DATA[3];
    int16_t GYRO_DATA[3];
    uint8_t send_data[6];
    uint32_t xNewPeriod = 1000;
    ICM20618_Acc_Read(ACC_DATA);
    ICM20618_GYRO_Read(GYRO_DATA);
    APP_PRINT_INFO3("icm20618 ACC_DATA: X:%d  Y:%d  Z:%d", ACC_DATA[0], ACC_DATA[1], ACC_DATA[2]);
    APP_PRINT_INFO3("icm20618 GYRO_DATA: X:%d  Y:%d  Z:%d", GYRO_DATA[0], GYRO_DATA[1], GYRO_DATA[2]);
    send_data[0] = (uint8_t)(ACC_DATA[0] >> 8);
    send_data[1] = (uint8_t)(ACC_DATA[0] & 0x00ff);
    send_data[2] = (uint8_t)(ACC_DATA[1] >> 8);
    send_data[3] = (uint8_t)(ACC_DATA[1] & 0x00ff);
    send_data[4] = (uint8_t)(ACC_DATA[2] >> 8);
    send_data[5] = (uint8_t)(ACC_DATA[2] & 0x00ff);
    server_send_data(0, notify_service_id, GATT_SVC_POSTURE_LEVEL_INDEX, send_data,
                     6 * sizeof(uint8_t), GATT_PDU_TYPE_NOTIFICATION);
    os_timer_restart(&xTimerPOS, xNewPeriod);
}
