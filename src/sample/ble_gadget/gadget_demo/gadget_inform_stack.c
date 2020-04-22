#include <gadget_inform_stack.h>
#include <mcu_vendor/ble_ama.h>
#include <AMATransport.h>
#include <gadget_service.h>
#include <trace.h>
#include <gadget_app.h>

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_ama       AMA Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
void on_connect(ble_ama_t *p_ama, uint8_t conn_id)
{
    p_ama->conn_handle = conn_id;
    p_ama->service_handle = ams_srv_id;
    p_ama->rx_char_handles.cccd_handle = GADGET_SERVICE_CHAR_NOTIFY_CCCD_INDEX;
    p_ama->tx_char_handles.value_handle = GADGET_SERVICE_CHAR_WRITE_INDEX;
    p_ama->rx_char_handles.value_handle = GADGET_SERVICE_CHAR_READ_INDEX;
    p_ama->mtu = 244;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_ama       AMA Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
void on_disconnect(ble_ama_t *p_ama)
{
    p_ama->conn_handle = 0xFF;
    AMATransport_GetInterfaces()->ConnectionStateChanged(AMA_TRANSPORT_BTLE, &p_ama->address, false);
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_ama       AMA Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
void on_write(ble_ama_t *p_ama)
{
}

uint32_t on_mtu_change(ble_ama_t *p_ama, uint16_t conn_id)
{
    if (p_ama->conn_handle != conn_id) { return 1; }

//    uint16_t client_mtu = p_ble_evt->evt.gatts_evt.params.exchange_mtu_request.client_rx_mtu;
//    NRF_LOG_DEBUG("MTU updated server=%d, client=%d", p_ama->mtu, client_mtu);
//    if (client_mtu < p_ama->mtu) {
//        return 1;
//    }
    return 0;
}

/**
 * @brief callback invoked when AMA gateway subscribe/unsubscribe
 *
 * @param[in] pEventParam Write/Read request param to the attribute
 */
uint32_t on_subscription(ble_ama_t *p_ama, T_GADGET_SUB_TYPE t_gadget_sub_state)
{
    if (t_gadget_sub_state == GADGET_NOTIFY_ENABLE)
    {
        AMATransport_GetInterfaces()->ConnectionStateChanged(AMA_TRANSPORT_BTLE, &p_ama->address, true);
        AMATransport_GetInterfaces()->MTUChanged(AMA_TRANSPORT_BTLE, &p_ama->address, p_ama->mtu - 3);
    }
    else if (t_gadget_sub_state == GADGET_NOTIFY_DISABLE)
    {
        AMATransport_GetInterfaces()->ConnectionStateChanged(AMA_TRANSPORT_BTLE, &p_ama->address, false);
    }
    return 0;
}

uint32_t on_data_received(ble_ama_t *p_ama, T_GADGET_CALLBACK_DATA   *p_cb_data)
{
    AMATransport_GetInterfaces()->ReceivedData(AMA_TRANSPORT_BTLE, &p_ama->address, \
                                               p_cb_data->msg_data.write.p_value, p_cb_data->msg_data.write.len);
    return 0;
}

uint32_t on_data_sent(ble_ama_t *p_ama)
{
    AMATransport_GetInterfaces()->DataSent(AMA_TRANSPORT_BTLE, &p_ama->address, true);
    return 0;
}
