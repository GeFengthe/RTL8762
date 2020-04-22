#include<mcu_vendor/ble_ama.h>
#include<gadget_service.h>

void on_connect(ble_ama_t *p_ama, uint8_t conn_id);

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_ama       AMA Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
void on_disconnect(ble_ama_t *p_ama);

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_ama       AMA Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
void on_write(ble_ama_t *p_ama);

uint32_t on_mtu_change(ble_ama_t *p_ama, uint16_t conn_id);

/**
 * @brief callback invoked when AMA gateway subscribe/unsubscribe
 *
 * @param[in] pEventParam Write/Read request param to the attribute
 */
uint32_t on_subscription(ble_ama_t *p_ama, T_GADGET_SUB_TYPE t_gadget_sub_state);

uint32_t on_data_received(ble_ama_t *p_ama, T_GADGET_CALLBACK_DATA   *p_cb_data);

uint32_t on_data_sent(ble_ama_t *p_ama);
