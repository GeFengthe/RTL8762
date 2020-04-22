/**
************************************************************************************************************
*               Copyright(c) 2014, Realtek Semiconductor Corporation. All rights reserved.
************************************************************************************************************
* @file     mp_cmd.c
* @brief    User defined mp commands.
* @details  MP command interfaces.
* @author   hector
* @date     2018-08-13
* @version  v1.0
*************************************************************************************************************
*/
#include "mp_cmd.h"
#include "rtl876x_lib_platform.h"
#include "user_data.h"
#include "platform_diagnose.h"


typedef struct
{
    uint32_t product_id;
    uint8_t key[32];
    uint8_t mac[6];
} _PACKED_ ali_data_t;

static mp_cmd_process_result_t mp_cmd_update_ali_data(uint16_t opcode, const uint8_t *data,
                                                      uint32_t len)
{
    ali_data_t *pdata = (ali_data_t *)data;
    if (!UpdateMAC(pdata->mac))
    {
        printe("mp_cmd_update_ali_data: update bd addr failed!");
        return MP_CMD_RESULT_ERROR;
    }

    if (!user_data_write_ali_data(pdata->product_id, pdata->key))
    {
        printe("mp_cmd_update_ali_data: update alibaba data failed!");
        return MP_CMD_RESULT_ERROR;
    }

    return MP_CMD_RESULT_OK;
}


/*----------------------------------------------------
 * command table
 * --------------------------------------------------*/
const mp_cmd_table_t mp_cmd_table[] =
{
    {MP_CMD_UPDATE_ALI_DATA, 42, mp_cmd_update_ali_data},

    /** must be at the end, do not modify */
    {0, 0, 0}
};

uint32_t mp_cmd_table_length(void)
{
    uint32_t i = 0;
    while ((0 != mp_cmd_table[i].opcode) ||
           (0 != mp_cmd_table[i].payload_len) ||
           (NULL != mp_cmd_table[i].cmd_process))
    {
        i++;
    }

    return i;
}
