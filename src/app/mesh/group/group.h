/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
* @file     group.h
* @brief    Head file for group related.
* @details  Data types and external functions declaration.
* @author   bill
* @date     2018-11-06
* @version  v1.0
* *************************************************************************************
*/

/* Define to prevent recursive inclusion */
#ifndef _GROUP_H
#define _GROUP_H

/* Add Includes here */
#include "platform_misc.h"
#include "gap_manufacture_adv.h"

BEGIN_DECLS

/**
 * @addtogroup Group
 * @{
 */

/**
 * @defgroup Group_Exported_Macros Exported Macros
 * @brief
 * @{
 */
#define GROUP_RECEIVER_PREEMPTIVE_MODE      1
#define GROUP_RECEIVER_RX_EVEN_NOT_CFG      1
#define GROUP_RECEIVER_SUPPORT_SYNC         1

#define GROUP_ALL                           0xff
#define GROUP_INVALID                       0x00

#define GROUP_CTL_ON                        0x01
#define GROUP_CTL_OFF                       0x00

#define GROUP_TRANSMITTER_TX_TIMES          3 //!< bigger than or equal to 1
#define GROUP_RECEIVER_RELAY_TIMES          GROUP_TRANSMITTER_TX_TIMES //!< bigger than or equal to 1
#define GROUP_RECEIVER_TX_TIMES             GROUP_TRANSMITTER_TX_TIMES //!< bigger than or equal to 1

#define GROUP_SYNC_TIMEOUT_MSG              0x0718
#define GROUP_SYNC_DELAY_FACTOR             21845
#define GROUP_SYNC_PERIOD_MS                (1000*0xffff/GROUP_SYNC_DELAY_FACTOR)

/** @} */

/**
 * @defgroup Group_Exported_Types Exported Types
 * @brief
 * @{
 */
typedef enum
{
    GROUP_MSG_TYPE_CFG,
    GROUP_MSG_TYPE_CTL,
    GROUP_MSG_TYPE_SYNC
} group_msg_type_t;

enum
{
    GROUP_CFG_OPCODE_GROUP,
    GROUP_CFG_OPCODE_MAX = 0xff
} _SHORT_ENUM_;
typedef uint8_t group_cfg_opcode_t;

typedef struct
{
    group_cfg_opcode_t opcode;
    union
    {
        /* GROUP_INVALID means delete all group,
           GROUP_ALL is reserved,
           otherwise add one group */
        uint8_t group;
    };
} _PACKED_ group_cfg_t;

enum
{
    GROUP_CTL_OPCODE_ON_OFF,
    /* relative value */
    GROUP_CTL_OPCODE_LIGHTNESS,
    /* relative value */
    GROUP_CTL_OPCODE_TEMPERATURE,
    GROUP_CTL_OPCODE_NIGHT_LIGHT,
    GROUP_CTL_OPCODE_GOOD_NIGHT,
    GROUP_CTL_OPCODE_MAX = 0xffff
} _SHORT_ENUM_;
typedef uint16_t group_ctl_opcode_t;

typedef struct
{
    uint8_t group;
    group_ctl_opcode_t opcode;
    union
    {
        uint8_t on_off;
        int16_t lightness;
        int16_t temperature;
    };
} _PACKED_ group_ctl_t;

/**
  * @brief The weight of the sync msg is provided by the app.
  * The value maybe a random value, or maybe a fixed value calculated
  * from the bt addr etc.
  */
typedef struct
{
    //uint8_t bt_addr[6];
    //uint8_t group;
    /** the smaller the weight value, the higher priority */
    uint16_t weight;
    /** the sync data can be any values with variable length */
    uint8_t sync_data[22]; //!< variable length
} _PACKED_ group_sync_t;

typedef struct
{
    uint8_t type: 4; //!< @ref group_msg_type_t
    uint8_t relay: 1;
    uint8_t rfu: 3; //!< shall be 0
    uint8_t tid;
    union
    {
        uint8_t data[24]; //!< relay msg len minus 6 bytes
        group_cfg_t cfg;
        group_ctl_t ctl;
        group_sync_t sync;
    };
} _PACKED_ group_msg_t;

typedef enum
{
    GROUP_RECEIVER_STATE_NORMAL,
    GROUP_RECEIVER_STATE_CFG
} group_receiver_state_t;

typedef void (*pf_group_receiver_receive_cb_t)(T_LE_SCAN_INFO *ple_scan_info, uint8_t *pdata,
                                               uint8_t len);
/** @} */

/**
 * @defgroup Group_Receiver_Exported_Functions Receiver Exported Functions
 * @brief
 * @{
 */

/**
  * @brief initialize the group receiver
  * @return none
  */
void group_receiver_init(void);

/**
  * @brief set the state of the group receiver
  *
  * The group receiver receives the configuration messages from the transmitter
  * only when the receiver is in GROUP_RECEIVER_STATE_CFG state.
  * @param[in] state: receiver state
  * @return none
  */
void group_receiver_state_set(group_receiver_state_t state);

/**
  * @brief register the callback of the transmitter
  * @param[in] type: msg type
  * @param[in] pf: the callback function
  * @return none
  */
void group_receiver_reg_cb(group_msg_type_t type, pf_group_receiver_receive_cb_t pf);

/**
  * @brief check whether the receiver has been configured by the transmitter
  * @return check result
  * @retval true: yes
  * @retval false: no
  */
bool group_receiver_check(void);

/**
  * @brief clear all the group receiver flash parameters to reset the receiver
  * @return operation result
  * @retval true: operation success
  * @retval false: operation failure
  */
bool group_receiver_nvm_clear(void);

/**
  * @brief receive the advertising
  * @param[in] ple_scan_info: the adv pkt
  * @return none
  * <b>Example usage</b>
  * \code{.c}
    T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data)
    {
        ...
        case GAP_MSG_LE_SCAN_INFO:
            group_receiver_receive(p_data->p_le_scan_info);
            break;
        ...
    }
  * \endcode
  */
void group_receiver_receive(T_LE_SCAN_INFO *ple_scan_info);

/**
  * @brief sync the node status to other nodes
  * @param[in] psync: the sync msg pointer, the weight is provided by the caller
  * @param[in] len: the sync msg length
  * @return none
  * <b>Example usage</b>
  * \code{.c}
    {
        group_sync_t sync;
        plt_rand((uint8_t *)&sync.weight, 2);
        light_get_cwrgb(sync.sync_data);
        group_receiver_sync(&sync, 7);
    }
  * \endcode
  */
void group_receiver_sync(group_sync_t *psync, uint8_t len);

/**
  * @brief handle the timeout event of sync
  * @return none
  */
void group_receiver_handle_sync_timeout(void);
/** @} */

/**
 * @defgroup Group_Transmitter_Exported_Functions Tranmitter Exported Functions
 * @brief
 * @{
 */

/**
  * @brief initialize the group transmitter
  * @return none
  */
void group_transmitter_init(void);

/**
  * @brief configure the receiver's group
  * @param[in] group: the group to configure
  * @return operation result
  * @retval true: operation success
  * @retval false: operation failure
  */
bool group_transmitter_cfg_group(uint8_t group);

/**
  * @brief control the receiver's on off state
  * @param[in] group: the group to control
  * @param[in] on_off: on off state
  * @return operation result
  * @retval true: operation success
  * @retval false: operation failure
  */
bool group_transmitter_ctl_on_off(uint8_t group, uint8_t on_off);

/**
  * @brief control the receiver's lightness state
  * @param[in] group: the group to control
  * @param[in] lightness: lightness delta value
  * @return operation result
  * @retval true: operation success
  * @retval false: operation failure
  */
bool group_transmitter_ctl_lightness(uint8_t group, int16_t lightness);

/**
  * @brief control the receiver's temperature state
  * @param[in] group: the group to control
  * @param[in] temperature: temperature delta value
  * @return operation result
  * @retval true: operation success
  * @retval false: operation failure
  */
bool group_transmitter_ctl_temperature(uint8_t group, int16_t temperature);

/**
  * @brief control the receiver's temperature state
  * @param[in] group: the group to control
  * @return operation result
  * @retval true: operation success
  * @retval false: operation failure
  */
bool group_transmitter_ctl_night_light(uint8_t group);

/**
  * @brief control the receiver's temperature state
  * @param[in] group: the group to control
  * @return operation result
  * @retval true: operation success
  * @retval false: operation failure
  */
bool group_transmitter_ctl_good_night(uint8_t group);

/** @} */
/** @} */

END_DECLS

#endif /* _GROUP_H */
