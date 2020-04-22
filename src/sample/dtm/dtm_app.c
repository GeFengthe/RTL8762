/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      dtm_app.c
* @brief     direct test mode implementation.
* @details
* @author    jeff
* @date      2016-12-01
* @version   v1.0
*
*/

#include "rtl876x_nvic.h"
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_uart.h"
#include "gap_dtm.h"
#include "trace.h"
#include "board.h"

/** @defgroup  DTM_APP Direct Test Mode Application
  * @brief This file handles commands from 8852B and return with events.
  * @{
  */

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
  * @brief  Uart initialization
  * @return void
  */
void dtm_uart_init(void)
{
    Pinmux_Config(DATA_UART_TX_PIN, UART0_TX);
    Pinmux_Config(DATA_UART_RX_PIN, UART0_RX);

    Pad_Config(DATA_UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(DATA_UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);

    UART_InitTypeDef uartInitStruct;
    UART_StructInit(&uartInitStruct);

    uartInitStruct.rxTriggerLevel = UART_RX_FIFO_TRIGGER_LEVEL_4BYTE;
    UART_Init(UART, &uartInitStruct);

    UART_INTConfig(UART, UART_INT_RD_AVA, ENABLE);

    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = UART0_IRQn;
    nvic_init_struct.NVIC_IRQChannelCmd      = ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 5;
    NVIC_Init(&nvic_init_struct);

    return;
}

/**
  * @brief  Send event to 8852B
  * @param  event events to send
  * @return void
  */
void dtm_uart_send_bytes(uint16_t event)
{
    uint8_t uartdata[2] = {0};
    uartdata[0] = (event & 0xff00) >> 8;
    uartdata[1] = event & 0xff;
    uint8_t *p_ch = uartdata;

    uint8_t i = 0;
    for (i = 0; i < 2; i++)
    {
        while (UART_GetFlagState(UART, UART_FLAG_THR_EMPTY) != SET)
        {
            ;
        }
        UART_SendData(UART, p_ch++, 1);
    }
}

/**
  * @brief      handle test command from 8852B, then invoke gap api to start test procedure.
  * @param[in]  command commands from 8852B.
  * @return     void
  */
void dtm_test_req(uint16_t command)
{
    uint8_t contrl = 0;
    uint8_t param = 0;
    uint8_t tx_chann = 0;
    uint8_t rx_chann = 0;
    uint8_t data_len = 0;
    uint8_t pkt_pl = 0;
    uint8_t cmd = (command & 0xc000) >> 14;
    uint16_t event  = 0;

    //the upper 2 bits of the data length for any Transmitter or Receiver commands following
    static uint8_t up_2_bits = 0;
    //physical to use
    static uint8_t phy = 1;
    //modulation index to use
    static uint8_t mod_idx = 0;
    //local supported features
    static uint8_t lcl_feats[GAP_LE_SUPPORTED_FEATURES_LEN] = {0};

    switch (cmd)
    {
    case 0:
        contrl = (command & 0x3f00) >> 8;
        param = (command & 0xfc) >> 2;

        switch (contrl)
        {
        case 0:
            if (param == 0)
            {
                up_2_bits = 0;
                phy = 1;
                mod_idx = 0;
            }
            else
            {
                event |= 1;
            }
            dtm_uart_send_bytes(event);
            break;
        case 1:
            if (param <= 3)
            {
                up_2_bits = param;
            }
            else
            {
                event |= 1;
            }
            dtm_uart_send_bytes(event);
            break;
        case 2:
            if (param >= 1 && param <= 4)
            {
                phy = (param == 4) ? 3 : param;
            }
            else
            {
                event |= 1;
            }
            dtm_uart_send_bytes(event);
            break;
        case 3:
            if (param == 0 || param == 1)
            {
                mod_idx = param;
            }
            else
            {
                event |= 1;
            }
            dtm_uart_send_bytes(event);
            break;
        case 4:
            if (param == 0)
            {
                le_get_gap_param(GAP_PARAM_LOCAL_FEATURES, lcl_feats);
                if (lcl_feats[0] & 0x20)
                {
                    event |= 0x0002;
                }
                if (lcl_feats[1] & 0x01)
                {
                    event |= 0x0004;
                }
                if (lcl_feats[1] & 0x02)
                {
                    event |= 0x0008;
                }
                APP_PRINT_INFO1("dtm_test_req: lcl_feats 0x%x", event);
            }
            else
            {
                event |= 1;
            }
            dtm_uart_send_bytes(event);
            break;
        case 5:
            /*
            8852B do not send these commands
                0x00 Read supportedMaxTxOctets
                0x01 Read supportedMaxTxTime
                0x02 Read supportedMaxRxOctets
                0x03 Read supportedMaxRxTime
            */
            break;
        }
        APP_PRINT_INFO3("dtm_test_req: up_2_bits 0x%x, phy 0x%x, mod_idx 0x%x", up_2_bits, phy, mod_idx);
        break;

    case 1:
        rx_chann = (command & 0x3f00) >> 8;
        APP_PRINT_INFO3("dtm_test_req: rx_channel 0x%x, phy 0x%x, mod_idx 0x%x", rx_chann, phy, mod_idx);
#if F_BT_LE_5_0_SUPPORT
        if (le_dtm_enhanced_receiver_test(rx_chann, phy, mod_idx) != GAP_CAUSE_SUCCESS)
        {
            APP_PRINT_ERROR0("dtm_test_req: le_dtm_enhanced_receiver_test fail");
        }
#else
        if (le_dtm_receiver_test(rx_chann) != GAP_CAUSE_SUCCESS)
        {
            APP_PRINT_ERROR0("dtm_test_req: le_dtm_receiver_test fail");
        }
#endif
        break;

    case 2:
        tx_chann = (command & 0x3f00) >> 8;
        data_len = up_2_bits << 6 | (command & 0xfc) >> 2;
        pkt_pl = command & 0x03;
        if (pkt_pl == 3)
        {
            pkt_pl = 4;
        }
        APP_PRINT_INFO4("dtm_test_req: tx_channel 0x%x, data_len 0x%x, pkt_pl 0x%x, phy 0x%x", tx_chann,
                        data_len, pkt_pl, phy);
#if F_BT_LE_5_0_SUPPORT
        if (le_dtm_enhanced_transmitter_test(tx_chann, data_len, pkt_pl, phy) != GAP_CAUSE_SUCCESS)
        {
            APP_PRINT_ERROR0("dtm_test_req: le_dtm_enhanced_transmitter_test fail");
        }
#else
        if (le_dtm_transmitter_test(tx_chann, data_len, pkt_pl) != GAP_CAUSE_SUCCESS)
        {
            APP_PRINT_ERROR0("dtm_test_req: le_dtm_transmitter_test fail");
        }
#endif
        break;

    case 3:
        contrl = (command & 0x3f00) >> 8;
        param = (command & 0xfc) >> 2;
        if (contrl == 0 && param == 0)
        {
            APP_PRINT_INFO0("dtm_test_req: le test end");
            if (le_dtm_test_end() != GAP_CAUSE_SUCCESS)
            {
                APP_PRINT_ERROR0("dtm_test_req: le_test_end fail");
            }
        }
        break;

    default:
        return;
    }
}


/**
  * @brief  Receive command from 8852B
  * @return void
  */
void UART0_Handler(void)
{
    uint8_t uartdata[2] = {0, 0};
    uint8_t *p = uartdata;
    uint16_t command = 0;
    uint32_t int_status = 0;
    int_status = UART_GetIID(UART);

    UART_INTConfig(UART, UART_INT_RD_AVA | UART_INT_LINE_STS, DISABLE);

    switch (int_status)
    {
    case UART_INT_ID_TX_EMPTY:
        break;

    case UART_INT_ID_RX_LEVEL_REACH:

    case UART_INT_ID_RX_TMEOUT:
        while (UART_GetFlagState(UART, UART_FLAG_RX_DATA_RDY) == SET)
        {
            UART_ReceiveData(UART, p++, 1);
            if (p - uartdata > 2)
            {
                break;
            }
        }
        command = (uartdata[0] << 8) | uartdata[1];
        APP_PRINT_INFO1("FORM 8852B: 0x%x", command);
        dtm_test_req(command);
        break;

    case UART_INT_ID_LINE_STATUS:
        break;

    default:
        break;
    }

    UART_INTConfig(UART, UART_INT_RD_AVA, ENABLE);
    return;
}

/**
  * @brief      Callback for gap le to notify app
  * @param[in]  cb_type callback msy type
  * @param[in]  p_cb_data point to callback data
  * @retval     result
  */
T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data)
{

    T_APP_RESULT result = APP_RESULT_SUCCESS;
    T_LE_CB_DATA *p_data = (T_LE_CB_DATA *)p_cb_data;
    uint16_t status  = 0;
    uint16_t event = 0;
    APP_PRINT_INFO1("app_gap_callback: cb_type %d", cb_type);
    switch (cb_type)
    {
    case GAP_MSG_LE_DTM_RECEIVER_TEST:
#if F_BT_LE_5_0_SUPPORT
    case GAP_MSG_LE_DTM_ENHANCED_RECEIVER_TEST:
#endif
        status = p_data->le_cause.cause;
        if (status == 0)
        {
            APP_PRINT_INFO2("app_gap_callback: event 0x%x, status 0x%x", (event & 0x8000) >> 15, event & 0x1);
        }
        else
        {
            event |= 1;
            APP_PRINT_INFO2("app_gap_callback: event 0x%x, status 0x%x", (event & 0x8000) >> 15, event & 0x1);
        }
        dtm_uart_send_bytes(event);
        break;

    case GAP_MSG_LE_DTM_TRANSMITTER_TEST:
#if F_BT_LE_5_0_SUPPORT
    case GAP_MSG_LE_DTM_ENHANCED_TRANSMITTER_TEST:
#endif
        status = p_data->le_cause.cause;
        if (status == 0)
        {
            APP_PRINT_INFO2("app_gap_callback: event 0x%x, status 0x%x", (event & 0x8000) >> 15, event & 0x1);
        }
        else
        {
            event |= 1;
            APP_PRINT_INFO2("app_gap_callback: event 0x%x, status 0x%x", (event & 0x8000) >> 15, event & 0x1);
        }
        dtm_uart_send_bytes(event);
        break;

    case GAP_MSG_LE_DTM_TEST_END:
        status = p_data->p_le_dtm_test_end_rsp->cause;
        if (status == 0)
        {
            event |= 1 << 15;
            event |= p_data->p_le_dtm_test_end_rsp->num_pkts;
            APP_PRINT_INFO2("app_gap_callback: event 0x%x, packet count 0x%x", (event & 0x8000) >> 15,
                            event & 0x7fff);
        }
        else
        {
            event |= 1;
            APP_PRINT_INFO2("app_gap_callback: event 0x%x, status 0x%x", (event & 0x8000) >> 15, event & 0x1);
        }
        dtm_uart_send_bytes(event);
        break;
    }

    return result;
}

/** @} */ /* End of group PERIPH_APP */
