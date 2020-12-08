/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      data_uart_test.c
* @brief    This file provides data uart test.
* @details
* @author  chenjie
* @date     2018-05-29
* @version  v1.0
*********************************************************************************************************
*/

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "board.h"
#include <data_uart_test.h>
#include <version.h>
#include <patch_header_check.h>
#include <gap.h>
#include <string.h>
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "rtl876x_uart.h"
#include "gap_bond_le.h"
#if MP_TEST_MODE_SUPPORT_AUTO_K_RF
#include "auto_k_rf_bonding_dut.h"
#include "auto_k_rf.h"
#endif
#include "rtl876x_wdg.h"
#include "rtl876x_lib_platform.h"
#include "test_mode.h"
#include "flash_adv_cfg.h"

#if MP_TEST_MODE_SUPPORT_DATA_UART_TEST

#define TEST_DEVICE_TYPE 0x21  /* Bee2 DUT device type */

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
static bool uart_test_is_dlps_allowed = true;

/*============================================================================*
 *                              Local Functions
 *============================================================================*/
static void uart_test_get_cmd_func(UART_PacketTypeDef *p_data);
static void uart_test_read_patch_version(void *p_data);
static void uart_test_read_app_version(void *p_data);
static void uart_test_read_mac_addr(void *p_data);
static void uart_test_write_mac_addr(void *p_data);
static void uart_test_enter_hci_test_mode(void *p_data);
static void uart_test_reboot_device(void *p_data);
static void uart_test_get_dut_info(void *p_data);
static void uart_test_auto_k_rf_freq(void *p_data);
static void uart_test_find_device_type(void *p_data);
static void uart_test_direct_k_rf_freq(void *p_data);
static void uart_test_manual_k_rf_freq(void *p_data);
static void uart_test_enter_hci_download_mode(void *p_data);

/*============================================================================*
 *                              Global Variables
 *============================================================================*/
/**<  Array of all used test function informations */
const T_UART_TEST_PROTOCOL uart_test_func_map[UART_TEST_SUPPORT_NUM] =
{
    /* Opcode, Parameter Length, Function */
    {READ_PATCH_VERSION_CMD, 0, uart_test_read_patch_version},
    {READ_APP_VERSION_CMD, 0, uart_test_read_app_version},
    {READ_MAC_ADDR_CMD, 0, uart_test_read_mac_addr},
    {ENTER_FAST_PAIR_MODE_CMD, 1, NULL},
    {GET_DEVICE_STATE_CMD, 0, NULL},
    {VOICE_TEST_START_CMD, 0, NULL},
    {VOICE_TEST_STOP_CMD, 0, NULL},
    {SET_VOICE_CONFIG_CMD, 0, NULL},
    {GET_VOICE_CONFIG_CMD, 0, NULL},
    {ENTER_DLPS_TEST_MODE_CMD, 0, NULL},
    {START_STOP_ADV_CMD, 1, NULL},
    {START_IR_TEST_MODE_CMD, 0, NULL},
    {ENTER_HCI_TEST_MODE_CMD, 0, uart_test_enter_hci_test_mode},
    {DISABLE_TEST_MODE_FLG_CMD, 0, NULL},
    {ENABLE_TEST_MODE_FLG_CMD, 0, NULL},
    {ERASE_PAIR_INFO_CMD, 0, NULL},
    {CHANGE_BAUDRATE_CMD, 1, NULL},
    {DIRECT_K_RF_FREQ_CMD, 2, uart_test_direct_k_rf_freq},
    {GET_GLODEN_INFO_CMD, 0, NULL},
    {GET_DUT_INFO_CMD, 32, uart_test_get_dut_info},
    {VERIFY_DUT_INFO_CMD, 0, NULL},
    {AUTO_K_RF_FREQ_CMD, 18, uart_test_auto_k_rf_freq},
    {FIND_DEVICE_TYPE_CMD, 0, uart_test_find_device_type},
    {REBOOT_DEVICE_CMD, 0, uart_test_reboot_device},
    {UPDATE_MAC_ADDR_CMD, 6, uart_test_write_mac_addr},
    {ENTER_SINGLE_TONE_MODE_CMD, 0, NULL},
    {READ_HARDWARE_VERSION_CMD, 0, NULL},
    {TERMINATE_CONNECT_CMD, 0, NULL},
    {MANUAL_K_RF_FREQ_CMD, 1, uart_test_manual_k_rf_freq},
    {ENTER_HCI_DOWNLOAD_MODE_CMD, 0, uart_test_enter_hci_download_mode},
    //Add more command here,Please store in order according to opcode!
};

/**
  * @brief Read patch command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_read_patch_version(void *p_data)
{
    uint16_t patch_version = 0;
    uint8_t patch_version_bytes[2] = {0};

    T_IMG_HEADER_FORMAT *p_header;
    uint32_t addr = get_header_addr_by_img_id(RomPatch);
    p_header = (T_IMG_HEADER_FORMAT *)addr;

    if (p_header)
    {
        patch_version = p_header->git_ver.ver_info.version;
    }
    else
    {
        patch_version = 0x00;
    }

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[uart_test_read_patch_version] patch version: %d", 1,
                    patch_version);

    patch_version_bytes[0] = (uint8_t)(patch_version & 0x00FF);
    patch_version_bytes[1] = (uint8_t)(patch_version >> 8);

    /* Response data */
    UARTCmd_Response(READ_PATCH_VERSION_CMD, UART_TEST_SUCCESS, patch_version_bytes, 2);
}

/**
  * @brief Read App command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_read_app_version(void *p_data)
{
    uint8_t app_version_bytes[4] = {0};
    app_version_bytes[0] = (uint8_t)(VERSION_MAJOR);
    app_version_bytes[1] = (uint8_t)(VERSION_MINOR);
    app_version_bytes[2] = (uint8_t)(VERSION_REVISION);
    app_version_bytes[3] = (uint8_t)(VERSION_BUILDNUM);

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[uart_test_read_app_version] app version: %d.%d.%d.%d", 4,
                    app_version_bytes[0], app_version_bytes[1], app_version_bytes[2], app_version_bytes[3]);

    /* Response data */
    UARTCmd_Response(READ_APP_VERSION_CMD, UART_TEST_SUCCESS, app_version_bytes, 4);
}

/**
  * @brief Read Mac address command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_read_mac_addr(void *p_data)
{
    uint8_t mac_addr[6];
    gap_get_param(GAP_PARAM_BD_ADDR, mac_addr);

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                    "[uart_test_read_mac_addr] MAC Addr: %02X %02X %02X %02X %02X %02X",
                    6, mac_addr[5], mac_addr[4], mac_addr[3], mac_addr[2], mac_addr[1], mac_addr[0]);

    /* Response data */
    UARTCmd_Response(READ_MAC_ADDR_CMD, UART_TEST_SUCCESS, mac_addr, 6);
}

/**
  * @brief Write MAC address command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_write_mac_addr(void *p_data)
{
    uint8_t mac_addr[6];

    memcpy(mac_addr, (uint8_t *)p_data + 3, 6);

    DBG_DIRECT("[uart_test_write_mac_addr] MAC Addr: %02X %02X %02X %02X %02X %02X",
               mac_addr[5], mac_addr[4], mac_addr[3], mac_addr[2], mac_addr[1], mac_addr[0]);

    if (UpdateMAC(mac_addr))
    {
        UARTCmd_Response(UPDATE_MAC_ADDR_CMD, UART_TEST_SUCCESS, NULL, 0);
        DBG_DIRECT("[uart_test_write_mac_addr] UpdateMAC reboot!");
        WDG_SystemReset(RESET_ALL, UART_CMD_RESET);
    }
    else
    {
        UARTCmd_Response(UPDATE_MAC_ADDR_CMD, UART_TEST_ERROR, NULL, 0);
    }
}

/**
  * @brief Enter HCI test mode command.
  * @param None.
  * @retval None.
  */
void uart_test_enter_hci_test_mode(void *pPacket)
{
    UARTCmd_Response(ENTER_HCI_TEST_MODE_CMD, UART_TEST_SUCCESS, NULL, 0);

    DBG_DIRECT("[uart_test_enter_hci_test_mode] Enter HCI Mode!!!");
    switch_to_hci_mode();
}

/**
  * @brief Reboot device command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_reboot_device(void *p_data)
{
    DBG_DIRECT("[uart_test_reboot_device] reboot device!");

    UARTCmd_Response(REBOOT_DEVICE_CMD, UART_TEST_SUCCESS, NULL, 0);

    WDG_SystemReset(RESET_ALL, UART_CMD_RESET);
}

/**
  * @brief Get DUT information command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_get_dut_info(void *p_data)
{
#if MP_TEST_MODE_SUPPORT_AUTO_K_RF
    bool result = false;
    uint8_t golden_info[32];
    uint8_t dut_info[32];

    memset(dut_info, 0, 32);
    memcpy(golden_info, (uint8_t *)p_data + 3, 32);
    result = dut_info_get(golden_info, dut_info);

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                    "[uart_test_get_dut_info] result is %d", 1, result);

    if (result == true)
    {
        UARTCmd_Response(GET_DUT_INFO_CMD, UART_TEST_SUCCESS, dut_info, 32);
    }
    else
    {
        UARTCmd_Response(GET_DUT_INFO_CMD, UART_TEST_ERROR, NULL, 0);
    }
#else
    UARTCmd_Response(GET_DUT_INFO_CMD, UART_TEST_ERROR, NULL, 0);
#endif
}

/**
  * @brief Direct K RF frequency command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_direct_k_rf_freq(void *p_data)
{
#if MP_TEST_MODE_SUPPORT_AUTO_K_RF
    uint8_t cal_xtal_result;
    uint8_t rx_channel = *((uint8_t *)p_data + 3);
    uint8_t drift_threshold = *((uint8_t *)p_data + 4);

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                    "[uart_test_direct_k_rf_freq] rx_channel is %d, drift_threshold is %d",
                    2, rx_channel, drift_threshold);

    if (Auto_K_RF(rx_channel, drift_threshold, &cal_xtal_result))
    {
        UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_direct_k_rf_freq] Cal RF freqency failed", 0);
        UARTCmd_Response(DIRECT_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
    }
    else
    {
        bool write_xtal_ret;

#ifdef WRITE_XTAL_TO_EFUSE
        write_xtal_ret = WriteXtalToEfuse(cal_xtal_result);
#else
        write_xtal_ret = WriteXtalToConfig(cal_xtal_result);
#endif
        if (write_xtal_ret == true)
        {
            UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_direct_k_rf_freq] Cal RF freqency successed",
                            0);
            UARTCmd_Response(DIRECT_K_RF_FREQ_CMD, UART_TEST_SUCCESS, &cal_xtal_result, 1);
        }
        else
        {
            UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_direct_k_rf_freq] Cal RF freqency failed", 0);
            UARTCmd_Response(DIRECT_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
        }
    }
#endif
}

/**
  * @brief Get DUT information command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_auto_k_rf_freq(void *p_data)
{
#if MP_TEST_MODE_SUPPORT_AUTO_K_RF
    bool result = false;
    uint8_t result_info[16];
    uint8_t cal_xtal_result;

    uint8_t rx_channel = *((uint8_t *)p_data + 19);
    uint8_t drift_threshold = *((uint8_t *)p_data + 20);

    memcpy(result_info, (uint8_t *)p_data + 3, 16);
    result = check_verify_result(result_info);

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                    "[uart_test_auto_k_rf_freq] rx_channel is %d, drift_threshold is %d, check_result is %d",
                    3, rx_channel, drift_threshold, result);

    if (result == true)
    {
        if (Auto_K_RF(rx_channel, drift_threshold, &cal_xtal_result))
        {
            UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_auto_k_rf_freq] Cal RF freqency failed", 0);
            UARTCmd_Response(AUTO_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
        }
        else
        {
            bool write_xtal_ret;

#ifdef WRITE_XTAL_TO_EFUSE
            write_xtal_ret = WriteXtalToEfuse(cal_xtal_result);
#else
            write_xtal_ret = WriteXtalToConfig(cal_xtal_result);
#endif
            if (write_xtal_ret == true)
            {
                UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_auto_k_rf_freq] Cal RF freqency successed", 0);
                UARTCmd_Response(AUTO_K_RF_FREQ_CMD, UART_TEST_SUCCESS, &cal_xtal_result, 1);
            }
            else
            {
                UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_auto_k_rf_freq] Cal RF freqency failed", 0);
                UARTCmd_Response(AUTO_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
            }
        }
    }
    else
    {
        UARTCmd_Response(AUTO_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
    }
#else
    UARTCmd_Response(AUTO_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
#endif
}

/**
  * @brief  Find device type command.
  * @param  p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_find_device_type(void *p_data)
{
    uint8_t dev_type = TEST_DEVICE_TYPE;

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_find_device_type] dev_type is %d", 1,
                    dev_type);
    UARTCmd_Response(FIND_DEVICE_TYPE_CMD, UART_TEST_SUCCESS, &dev_type, 1);
}

/**
  * @brief  Manual K RF frequency command.
  * @param  p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_manual_k_rf_freq(void *p_data)
{
    uint8_t xtal = *((uint8_t *)p_data + 3);

    if (WriteXtalToConfig(xtal) == true)
    {
        UARTCmd_Response(MANUAL_K_RF_FREQ_CMD, UART_TEST_SUCCESS, NULL, 0);
    }
    else
    {
        UARTCmd_Response(MANUAL_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
    }
}

/**
  * @brief Enter HCI uart test mode command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_enter_hci_download_mode(void *p_data)
{
    DBG_DIRECT("[uart_test_enter_hci_download_mode] Switch to HCI UART download mode!");

    flash_set_block_protect_locked(0);
    flash_erase_locked(FLASH_ERASE_SECTOR, get_header_addr_by_img_id(OTA));

    UARTCmd_Response(ENTER_HCI_DOWNLOAD_MODE_CMD, UART_TEST_SUCCESS, NULL, 0);

    WDG_SystemReset(RESET_ALL, UART_CMD_RESET);
}

/**
  * @brief  Get the spefied uart command.
  * @param  p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_get_cmd_func(UART_PacketTypeDef *p_data)
{
    uint16_t opcode = (p_data->Buf[2] << 8) + p_data->Buf[1];

    UART_DBG_BUFFER(MODULE_UART, LEVEL_INFO, "[uart_test_get_cmd_func] opcode is 0x%04X", 1, opcode);

    /* uart test command */
    if ((opcode >= UART_FN_BEGIN) && (opcode < UART_FN_END))
    {
        if (uart_test_func_map[opcode & UART_FN_MASK].fncb != NULL)
        {
            uart_test_func_map[opcode & UART_FN_MASK].fncb(p_data);
        }
        else
        {
            UART_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "No uart test cmd function!", 0);
        }
    }
    else
    {
        //Other command
    }
}

/*============================================================================*
 *                              Global Functions
 *============================================================================*/
/**
  * @brief initialize uart test function.
  * @param None.
  * @retval None.
  */
void uart_test_init(void)
{
    //UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[uart_test_init] initialize uart test mode", 0);
	  DBG_DIRECT("uart_test_init!");
    UartTransport_Init();
    uart_test_is_dlps_allowed = false;
}

/**
  * @brief Uart Test Check DLPS allowed or not.
  * @param voide.
  * @retval true or false.
  */
bool uart_test_check_dlps_allowed(void)
{
    return uart_test_is_dlps_allowed;
}

/**
 * @brief handle UART message
 * @param io_driver_msg_recv - recieved io message
 * @return none
 * @retval void
 */
void uart_test_handle_uart_msg(T_IO_MSG io_driver_msg_recv)
{
    UART_PacketTypeDef *pUartTestPacket = (UART_PacketTypeDef *)(io_driver_msg_recv.u.buf);

    if (Packet_Decode(pUartTestPacket))
    {
        uart_test_get_cmd_func(pUartTestPacket);
    }
}

#endif

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

