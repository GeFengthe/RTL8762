/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     group_transmitter.c
  * @brief    Source file for group transmitter.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2018-11-06
  * @version  v1.0
  * *************************************************************************************
  */

/* Add Includes here */
#include <string.h>
#include "group.h"
#include "gap_scheduler.h"
#include "platform_diagnose.h"

typedef struct
{
    uint8_t tid;
} group_transmitter_ctx_t;

static group_transmitter_ctx_t gtc;

static bool group_transmitter_transmit(group_msg_type_t type, uint8_t *pdata, uint8_t len)
{
    group_msg_t *pmsg;
    uint8_t *pbuffer = gap_sched_task_get();
    if (pbuffer == NULL)
    {
        return false;
    }
    pbuffer[0] = len + MEMBER_OFFSET(group_msg_t, cfg) + 4;
    pbuffer[1] = GAP_ADTYPE_MANUFACTURER_SPECIFIC;
    LE_WORD2EXTRN(pbuffer + 2, MANUFACTURE_ADV_DATA_COMPANY_ID);
    pbuffer[4] = MANUFACTURE_ADV_DATA_TYPE_GROUP;
    pmsg = (group_msg_t *)(pbuffer + 5);
    pmsg->type = type;
    pmsg->relay = 0;
    pmsg->rfu = 0;
    pmsg->tid = gtc.tid++;
    memcpy(&pmsg->cfg, pdata, len);
    gap_sched_task_p ptask = CONTAINER_OF(pbuffer, gap_sched_task_t, adv_data);
    ptask->adv_type = GAP_SCHED_ADV_TYPE_IND;
    ptask->adv_len = pbuffer[0] + 1;
    ptask->retrans_count = GROUP_TRANSMITTER_TX_TIMES - 1;
    ptask->retrans_interval = 10;
    printi("group_transmitter_transmit: tid %d, type %d, len %d", pmsg->tid,
           pmsg->type, len + MEMBER_OFFSET(group_msg_t, cfg));
    dprinti((uint8_t *)pmsg, len + MEMBER_OFFSET(group_msg_t, cfg));
    gap_sched_try(ptask);
    return true;
}

bool group_transmitter_cfg_group(uint8_t group)
{
    group_cfg_t msg;
    msg.opcode = GROUP_CFG_OPCODE_GROUP;
    msg.group = group;
    return group_transmitter_transmit(GROUP_MSG_TYPE_CFG, (uint8_t *)&msg,
                                      sizeof(group_cfg_opcode_t) + sizeof(group));
}

bool group_transmitter_ctl_on_off(uint8_t group, uint8_t on_off)
{
    group_ctl_t msg;
    msg.opcode = GROUP_CTL_OPCODE_ON_OFF;
    msg.group = group;
    msg.on_off = on_off;
    return group_transmitter_transmit(GROUP_MSG_TYPE_CTL, (uint8_t *)&msg, MEMBER_OFFSET(group_ctl_t,
                                      on_off) + sizeof(on_off));
}

bool group_transmitter_ctl_lightness(uint8_t group, int16_t lightness)
{
    group_ctl_t msg;
    msg.opcode = GROUP_CTL_OPCODE_LIGHTNESS;
    msg.group = group;
    msg.lightness = lightness;
    return group_transmitter_transmit(GROUP_MSG_TYPE_CTL, (uint8_t *)&msg, MEMBER_OFFSET(group_ctl_t,
                                      on_off) + sizeof(lightness));
}

bool group_transmitter_ctl_temperature(uint8_t group, int16_t temperature)
{
    group_ctl_t msg;
    msg.opcode = GROUP_CTL_OPCODE_TEMPERATURE;
    msg.group = group;
    msg.temperature = temperature;
    return group_transmitter_transmit(GROUP_MSG_TYPE_CTL, (uint8_t *)&msg, MEMBER_OFFSET(group_ctl_t,
                                      on_off) + sizeof(temperature));
}

bool group_transmitter_ctl_night_light(uint8_t group)
{
    group_ctl_t msg;
    msg.opcode = GROUP_CTL_OPCODE_NIGHT_LIGHT;
    msg.group = group;
    return group_transmitter_transmit(GROUP_MSG_TYPE_CTL, (uint8_t *)&msg, MEMBER_OFFSET(group_ctl_t,
                                      on_off));
}

bool group_transmitter_ctl_good_night(uint8_t group)
{
    group_ctl_t msg;
    msg.opcode = GROUP_CTL_OPCODE_GOOD_NIGHT;
    msg.group = group;
    return group_transmitter_transmit(GROUP_MSG_TYPE_CTL, (uint8_t *)&msg, MEMBER_OFFSET(group_ctl_t,
                                      on_off));
}

void group_transmitter_init(void)
{
    plt_rand(&gtc.tid, 1);
}

