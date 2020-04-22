/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     ads_service.h
  * @brief    Head file for using Audio Down Streaming service.
  * @details  ADS data structs and external functions declaration.
  * @author   Chenjie Jin
  * @date     2018-4-5
  * @version  v0.1
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _ADS_H_
#define _ADS_H_

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

/* Add Includes here */
#include "profile_server.h"

/*============================================================================*
 *                         Macros
 *============================================================================*/

/** @addtogroup ADS ADS
  * @brief ADS Audio down streaming service
  * @{
  */

/** @defgroup ADS_Exported_Constants ADS Exported Constants
  * @brief macros that other .c files may use all defined here
  * @{
  */

#define ADS_CHAR_RX_LEN 20
#define ADS_CHAR_TX_LEN 255
#define ADS_CHAR_CTL_LEN 20

#define ADS_CTL_OPCODE_GET_CAPS_LEN 5
#define ADS_RX_OPCODE_GET_CAPS_RESP_LEN 7
#define ADS_CTL_OPCODE_AUDIO_START_LEN 3
#define ADS_RX_OPCODE_AUDIO_START_ERROR_LEN 3
#define ADS_RX_OPCODE_AUDIO_END_LEN 1
#define ADS_RX_OPCODE_AUDIO_PAUSE_LEN 1
#define ADS_RX_OPCODE_AUDIO_PLAY_LEN 1
#define ADS_CTL_OPCODE_AUDIO_STOP_LEN 1
#define ADS_RX_OPCODE_BUFFER_READY_LEN 9

#define ADS_VERSION_MAJOR 0x00
#define ADS_VERSION_MINOR 0x01

#define ADS_CODEC_MASK_AAC_ADTS    0x0001
#define ADS_CODEC_MASK_AAC_ADIF    0x0002
#define ADS_CODEC_MASK_MP3         0x0004
#define ADS_CODEC_MASK_SBC         0x0008

#define ADS_CODECS_SUPPORT (ADS_CODEC_MASK_SBC)

/** @brief ATVV related services UUIDs */
#define GATT_UUID_ADS_CHAR_RX  0xFDC1
#define GATT_UUID_ADS_CHAR_TX  0xFDC2
#define GATT_UUID_ADS_CHAR_CTL 0xFDC3

/** @brief Index defines for Characteristic's value */
#define GATT_SVC_ADS_CHAR_RX_VALUE_INDEX   2
#define GATT_SVC_ADS_CHAR_RX_CCCD_INDEX    3
#define GATT_SVC_ADS_CHAR_TX_VALUE_INDEX   5
#define GATT_SVC_ADS_CHAR_CTL_VALUE_INDEX  7
///@endcond

/** @} End of ADS_Exported_Constants */

/** @defgroup ADS_Exported_Types ADS Exported Types
  * @brief  types that other .c files may use all defined here
  * @{
  */
/* Add all public types here */
/** @defgroup ADS_Callback_Data ATTB Callback Data
  * @brief ADS data struct for notification data to application.
  * @{
  */

/** @defgroup ADS ATVV Read Info
  * @brief  Parameter for read characteristic value.
  * @{
  */
#define ADS_READ_CHAR_RX_INDEX   1

/** @} */

/** @defgroup ADS_Write_Info ADS Write Info
  * @brief  Parameter for write characteristic value.
  * @{
  */
#define ADS_WRITE_CHAR_TX_INDEX   1
#define ADS_WRITE_CHAR_CTL_INDEX  2

/** @} */

/** @defgroup ADS_Notify_Indicate_Info ADS Notify Indicate Info
  * @brief  Parameter for enable or disable notification or indication.
  * @{
  */
#define ADS_CHAR_RX_NOTIFY_INDICATE_ENABLE   1
#define ADS_CHAR_RX_NOTIFY_INDICATE_DISABLE  2

/** Message content */
typedef struct _T_ATTB_WRITE_PARAMETER
{
    uint8_t len;
    uint8_t *p_value;
} T_ADS_WRITE_PARAMETER;

/** @struct _T_ADS_WRITE_MSG
  * @brief write message
  */
typedef struct _TADS_WRITE_MSG
{
    uint8_t write_type; /**< ref: @ref ADS_RmC_Write_Info */
    T_ADS_WRITE_PARAMETER write_parameter;
} T_ADS_WRITE_MSG;

typedef union _T_ADS_UPSTREAM_MSG_DATA
{
    uint8_t notification_indification_index; /**< ref: @ref ADS_RmC_Notify_Indicate_Info */
    uint8_t read_value_index; /**< ref: @ref ADS_RmC_Read_Info */
    T_ADS_WRITE_MSG write;
} T_ADS_UPSTREAM_MSG_DATA;

/** ADS service data to inform application */
typedef struct _T_ADS_CALLBACK_DATA
{
    uint8_t                 conn_id;
    T_SERVICE_CALLBACK_TYPE     msg_type;
    T_ADS_UPSTREAM_MSG_DATA    msg_data;
} T_ADS_CALLBACK_DATA;
/** @} */

typedef enum
{
    ADS_RX_OPCODE_GET_CAPS_RESP = 0x01,
    ADS_RX_OPCODE_AUDIO_START_ERROR = 0x02,
    ADS_RX_OPCODE_AUDIO_END = 0x03,
    ADS_RX_OPCODE_BUFFER_READY = 0x04,
    ADS_RX_OPCODE_PLAY_PREV_ITEM = 0x05,
    ADS_RX_OPCODE_PLAY_NEXT_ITEM = 0x06,
    ADS_RX_OPCODE_AUDIO_PAUSE = 0x07,
    ADS_RX_OPCODE_AUDIO_PLAY = 0x08,
} ADS_CHAR_RX_OPCODE;

typedef enum
{
    ADS_CTL_OPCODE_GET_CAPS = 0x0A,
    ADS_CTL_OPCODE_AUDIO_START = 0x0B,
    ADS_CTL_OPCODE_AUDIO_STOP = 0x0C,
    ADS_CTL_OPCODE_AUDIO_VOL_UP = 0x0D,
    ADS_CTL_OPCODE_AUDIO_VOL_DOWN = 0x0E,
    ADS_CTL_OPCODE_AUDIO_MUTE = 0x0F,
    ADS_CTL_OPCODE_AUDIO_UNMUTE = 0x10,
} ADS_CHAR_CTL_CMD_OPCODE;

typedef enum
{
    ADS_INVALID_CODEC = 0x0F01,
} ADS_ERROR_CODE;

/** @} End of ADS_Exported_Types */

typedef struct _T_ADS_GLOBAL_DATA
{
    uint8_t master_version_major;
    uint8_t master_version_minor;
    uint16_t codec_used;
    uint8_t char_rx_data_buff[ADS_CHAR_RX_LEN];
    uint8_t char_ctl_data_buff[ADS_CHAR_CTL_LEN];
    uint8_t char_tx_data_buff[ADS_CHAR_TX_LEN];
} T_ADS_GLOBAL_DATA;

extern T_ADS_GLOBAL_DATA ads_global_data;

/** @defgroup ADS_Exported_Functions ADS Exported Functions
  * @brief functions that other .c files may use all defined here.
  * @{
  */
uint8_t ads_add_service(void *pFunc);
/** @} End of ADS_Exported_Functions */


/** @} */ /* End of group ADS */

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif //_ADS_H

