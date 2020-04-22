/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     key_handle.h
* @brief    This is the entry of user code which the key handle module resides in.
* @details
* @author   elliot chen
* @date     2018-11-06
* @version  v1.0
*********************************************************************************************************
*/

#ifndef _KEY_HANDLE_H_
#define _KEY_HANDLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <keyscan_driver.h>

/* Defines ------------------------------------------------------------------*/
/* define the bit mask of combine keys */
#define INVALID_COMBINE_KEYS_BIT_MASK               0x0000
#define ADV_COMBINE_KEYS_BIT_MASK                   0x0001
#define IR_LEARNING_COMBINE_KEYS_BIT_MASK           0x0002
#define HCI_UART_TEST_COMBINE_KEYS_BIT_MASK         0x0004
#define DATA_UART_TEST_COMBINE_KEYS_BIT_MASK        0x0008
#define SINGLE_TONE_TEST_COMBINE_KEYS_BIT_MASK      0x0010
#define FAST_PAIR_1_COMBINE_KEYS_BIT_MASK           0x0020
#define FAST_PAIR_2_COMBINE_KEYS_BIT_MASK           0x0040
#define FAST_PAIR_3_COMBINE_KEYS_BIT_MASK           0x0080
#define FAST_PAIR_4_COMBINE_KEYS_BIT_MASK           0x0100
#define FAST_PAIR_5_COMBINE_KEYS_BIT_MASK           0x0200
/* Configure combinatorial key timeout: 2 sec */
#define COMBINE_KEYS_DETECT_TIMEOUT                 2000
#define COMBINE_KEYS_REPEAT_SEND_TIMEOUT            200

#define GROUP_1         0x01
#define GROUP_2         0x02
#define GROUP_3         0x03
#define GROUP_4         0x04
typedef enum
{
    VK_NC            = 0x00,
    VK_POWER_ON      = 0x01,
    VK_POWER_OFF     = 0x02,
    VK_LIGHT_UP      = 0x03,
    VK_LIGHT_DOWN    = 0x04,
    VK_COLOR_TP_UP   = 0x05,
    VK_COLOR_TP_DOWN = 0x06,
    VK_GROUP_1_ON    = 0x07,
    VK_GROUP_1_OFF   = 0x08,
    VK_GROUP_2_ON    = 0x09,
    VK_GROUP_2_OFF   = 0x0A,
    VK_GROUP_3_ON    = 0x0B,
    VK_GROUP_3_OFF   = 0x0C,
    VK_GROUP_4_ON    = 0x0D,
    VK_GROUP_4_OFF   = 0x0E,
    KEY_INDEX_ENUM_GUAID
} T_KEY_INDEX_DEF;

#if 0
typedef enum
{
    VK_NC            = 0x00,
    VK_GROUP_1_ON    = 0x01,
    VK_GROUP_1_OFF   = 0x02,
    VK_GROUP_2_ON    = 0x03,
    VK_GROUP_2_OFF   = 0x04,
    VK_GROUP_3_ON    = 0x05,
    VK_GROUP_3_OFF   = 0x06,
    VK_GROUP_4_ON    = 0x07,
    VK_GROUP_4_OFF   = 0x08,
    VK_HOME          = 0x09,
    VK_MENU          = 0x0A,
    VK_POWER_ON      = 0x0B,
    VK_POWER_OFF     = 0x0C,
    VK_LIGHT_UP      = 0x0D,
    VK_LIGHT_DOWN    = 0x0E,
    VK_COLOR_TP_UP   = 0x0F,
    VK_COLOR_TP_DOWN = 0x10,
    KEY_INDEX_ENUM_GUAID
} T_KEY_INDEX_DEF;
#endif

/* define the key code table size, the value should modify according to BLE_KEY_CODE_TABLE */
#define BLE_KEY_CODE_TABLE_SIZE     KEY_INDEX_ENUM_GUAID
#define BLE_KEY_CODE_GROUP_START    VK_GROUP_1_ON

/* Key global parameters' struct */
typedef struct
{
    T_KEY_INDEX_DEF cur_press_key_index;       /* to indicate the current pressing key index */
    T_KEY_INDEX_DEF last_pressed_key_index;    /* to indicate the last pressed key index */
    uint32_t combine_keys_status;              /* to indicate the status of combined keys */
    uint8_t group_key;
    bool long_press_status;
} T_KEY_HANDLE_GLOBAL_DATA;

typedef bool (*p_group_transmitter_ctl_func)(uint8_t group, void *);
typedef struct
{
    uint8_t group;
    int16_t type;
    p_group_transmitter_ctl_func fncb;
} T_GROUP_CTL;

/*============================================================================*
*                        Export Global Variables
*============================================================================*/
extern const uint16_t BLE_KEY_CODE_TABLE[BLE_KEY_CODE_TABLE_SIZE];
extern T_KEY_HANDLE_GLOBAL_DATA key_handle_global_data;
extern plt_timer_t combine_keys_detection_timer;

/*============================================================================*
 *                         Functions
 *============================================================================*/
void key_handle_init_data(void);
void key_handle_pressed_event(T_KEYSCAN_FIFIO_DATA *pKey_Data);
void key_handle_release_event(T_KEYSCAN_FIFIO_DATA *p_keyscan_fifo_data);
bool key_handle_notfiy_key_data(T_KEY_INDEX_DEF key_index);
void key_handle_comb_keys_init_timer(void);
void key_handle_long_pressed_event(T_KEYSCAN_FIFIO_DATA *p_keyscan_fifo_data);

#ifdef __cplusplus
}
#endif

#endif
