/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rcu_uart_test.h
* @brief
* @details
* @author    chenjie jin
* @date      2018-04-08
* @version   v1.0
* *********************************************************************************************************
*/

#ifndef __RCU_UART_TEST_H
#define __RCU_UART_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "uart_transport.h"
#include "app_msg.h"

/* Defines ------------------------------------------------------------------ */

/** @brief  Number of function used in application.*/
/* You must modify this value according the function number used in your application. */

#define UART_FN_MASK                    ((uint16_t)0x00FF)

/* Protocol command defines */
#define UART_FN_BEGIN                   ((uint16_t)0x1100)

#define READ_PATCH_VERSION_CMD              ((uint16_t)0x1100)
#define READ_APP_VERSION_CMD                ((uint16_t)0x1101) //
#define READ_MAC_ADDR_CMD                   ((uint16_t)0x1102) //
#define ENTER_FAST_PAIR_MODE_CMD            ((uint16_t)0x1103)
#define GET_DEVICE_STATE_CMD                ((uint16_t)0x1104)
#define VOICE_TEST_START_CMD                ((uint16_t)0x1105)
#define VOICE_TEST_STOP_CMD                 ((uint16_t)0x1106)
#define SET_VOICE_CONFIG_CMD                ((uint16_t)0x1107)
#define GET_VOICE_CONFIG_CMD                ((uint16_t)0x1108)
#define ENTER_DLPS_TEST_MODE_CMD            ((uint16_t)0x1109)
#define START_STOP_ADV_CMD                  ((uint16_t)0x110A)
#define START_IR_TEST_MODE_CMD              ((uint16_t)0x110B)
#define ENTER_HCI_TEST_MODE_CMD             ((uint16_t)0x110C) //
#define DISABLE_TEST_MODE_FLG_CMD           ((uint16_t)0x110D)
#define ENABLE_TEST_MODE_FLG_CMD            ((uint16_t)0x110E) //
#define ERASE_PAIR_INFO_CMD                 ((uint16_t)0x110F) //
#define CHANGE_BAUDRATE_CMD                 ((uint16_t)0x1110) //
#define DIRECT_K_RF_FREQ_CMD                ((uint16_t)0x1111) //
#define GET_GLODEN_INFO_CMD                 ((uint16_t)0x1112) //
#define GET_DUT_INFO_CMD                    ((uint16_t)0x1113) //
#define VERIFY_DUT_INFO_CMD                 ((uint16_t)0x1114) //
#define AUTO_K_RF_FREQ_CMD                  ((uint16_t)0x1115) //
#define FIND_DEVICE_TYPE_CMD                ((uint16_t)0x1116) //
#define REBOOT_DEVICE_CMD                   ((uint16_t)0x1117) //
#define UPDATE_MAC_ADDR_CMD                 ((uint16_t)0x1118) //
#define ENTER_SINGLE_TONE_MODE_CMD          ((uint16_t)0x1119) //
#define READ_HARDWARE_VERSION_CMD           ((uint16_t)0x111A)
#define TERMINATE_CONNECT_CMD               ((uint16_t)0x111B)
#define MANUAL_K_RF_FREQ_CMD                ((uint16_t)0x111C)
#define ENTER_HCI_DOWNLOAD_MODE_CMD         ((uint16_t)0x111D)

#define ENTER_AMPERE_TEST_CMD               ((uint16_t)0x111E) //
#define GET_CHECKSUM_CMD                    ((uint16_t)0x111F) //
#define IO_TEST_CMD                         ((uint16_t)0x1120) //

#define UART_FN_END                     ((uint16_t)0x1121)

#define UART_TEST_SUPPORT_NUM         (UART_FN_END - UART_FN_BEGIN)

typedef void (*p_uart_test_func)(void *);

typedef struct
{
    uint16_t opcode;
    uint16_t param_len;
    p_uart_test_func fncb;
} T_UART_TEST_PROTOCOL;

typedef enum
{
    UART_TEST_SUCCESS = 0,
    UART_TEST_ERROR = 1
} T_UART_TEST_STATUS;

typedef enum
{
    CHANGE_BAUDRATE_OPTION_115200 = 0,
    CHANGE_BAUDRATE_OPTION_2M = 1,
} CHANGE_BAUDRATE_OPTION;

extern const T_UART_TEST_PROTOCOL uart_test_func_map[UART_TEST_SUPPORT_NUM];

void uart_test_init(void);
bool uart_test_check_dlps_allowed(void);
void uart_test_handle_uart_msg(T_IO_MSG io_driver_msg_recv);

#ifdef __cplusplus
}
#endif

#endif /*__RCU_UART_TEST_H*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

