/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     group_reciever.c
  * @brief    Source file for group reciever.
  * @details  Data types and external functions declaration.
  * @author   bill
  * @date     2018-11-06
  * @version  v1.0
  * *************************************************************************************
  */

/* Add Includes here */
#include <string.h>
#include "group.h"
#include "platform_diagnose.h"
#include "ftl.h"
#include "gap_scheduler.h"
#include "app_msg.h"

#define GROUP_RECEIVER_MAX_TRANSMITTER_NUM                  3
#define GROUP_RECEIVER_MAX_GROUP_NUM_EACH_TRANSMITTER       4
#define GROUP_RECEIVER_MAX_CACHE_SIZE                       20
#define GROUP_FLASH_PARAMS_OFFSET                           1948

#define GROUP_FLASH_PARAMS_EACH_TRANSMITTER_SIZE            (((1 + 1 + 6 + GROUP_RECEIVER_MAX_GROUP_NUM_EACH_TRANSMITTER + 4 + 3)/4)*4)

typedef struct
{
    bool valid;
    uint8_t bt_addr[6];
    uint8_t group[GROUP_RECEIVER_MAX_GROUP_NUM_EACH_TRANSMITTER];
    uint32_t rank;
    uint8_t tid;
} group_transmitter_info_t;

typedef struct _group_cache_t
{
    struct _group_cache_t *pnext;
    uint8_t bt_addr[6];
    uint8_t tid;
} group_cache_t;

typedef struct
{
    plt_timer_t timer;
    bool phase;
    uint32_t sync_time;
    group_sync_t sync_msg;
    uint8_t sync_len;
    bool quit;
    uint16_t weight;
} group_sync_info_t;

typedef struct
{
    group_transmitter_info_t gti[GROUP_RECEIVER_MAX_TRANSMITTER_NUM];
    group_receiver_state_t state;
    group_sync_info_t si;
    uint8_t tid;
    group_cache_t cache[GROUP_RECEIVER_MAX_CACHE_SIZE];
    plt_list_t cache_free;
    plt_list_t cache_used;
    pf_group_receiver_receive_cb_t cfg_cb;
    pf_group_receiver_receive_cb_t ctl_cb;
    pf_group_receiver_receive_cb_t sync_cb;
} group_receiver_ctx_t;

static group_receiver_ctx_t grc;

static bool group_receiver_group_find(int index, uint8_t group)
{
    int loop;
    for (loop = 0; loop < GROUP_RECEIVER_MAX_GROUP_NUM_EACH_TRANSMITTER; loop++)
    {
        if (grc.gti[index].group[loop] == group)
        {
            return true;
        }
    }
    return false;
}

static bool group_receiver_group_delete(int index)
{
    bool ret = false;
    for (int loop = 0; loop < GROUP_RECEIVER_MAX_GROUP_NUM_EACH_TRANSMITTER; loop++)
    {
        if (grc.gti[index].group[loop] != GROUP_INVALID)
        {
            grc.gti[index].group[loop] = GROUP_INVALID;
            ret = true;
        }
    }
    return ret;
}

static bool group_receiver_group_add(int index, uint8_t group)
{
    if (group_receiver_group_find(index, group))
    {
        return false;
    }

    for (int loop = 0; loop < GROUP_RECEIVER_MAX_GROUP_NUM_EACH_TRANSMITTER; loop++)
    {
        if (grc.gti[index].group[loop] == GROUP_INVALID)
        {
            grc.gti[index].group[loop] = group;
            return true;
        }
    }
    printw("group_receiver_group_add: no space left for the new group");
    return false;
}

static int group_receiver_allocate(uint8_t bt_addr[], uint8_t tid, uint32_t rank)
{
    int loop;
    for (loop = 0; loop < GROUP_RECEIVER_MAX_TRANSMITTER_NUM; loop++)
    {
        if (false == grc.gti[loop].valid)
        {
            grc.gti[loop].valid = true;
            memcpy(grc.gti[loop].bt_addr, bt_addr, 6);
            memset(grc.gti[loop].group, GROUP_INVALID, GROUP_RECEIVER_MAX_GROUP_NUM_EACH_TRANSMITTER);
            grc.gti[loop].rank = rank;
            grc.gti[loop].tid = tid;
            return loop;
        }
    }
    return -1;
}

static bool group_receiver_free(int index)
{
    if (false == grc.gti[index].valid)
    {
        return false;
    }
    grc.gti[index].valid = false;
    return true;
}

static int group_receiver_find(uint8_t bt_addr[], uint8_t tid)
{
    int loop;
    for (loop = 0; loop < GROUP_RECEIVER_MAX_TRANSMITTER_NUM; loop++)
    {
        if (grc.gti[loop].valid)
        {
            if (0 == memcmp(bt_addr, grc.gti[loop].bt_addr, 6))
            {
                if (tid == grc.gti[loop].tid)
                {
                    return -2;
                }
                else
                {
                    grc.gti[loop].tid = tid;
                }
                return loop;
            }
        }
    }
    return -1;
}

bool group_receiver_check(void)
{
    for (int loop = 0; loop < GROUP_RECEIVER_MAX_TRANSMITTER_NUM; loop++)
    {
        if (grc.gti[loop].valid)
        {
            return true;
        }
    }
    return false;
}

static int group_receiver_rank(bool order)
{
    int ret = -1;
    uint32_t rank = order ? 0x00000000 : 0xffffffff;
    for (int loop = 0; loop < GROUP_RECEIVER_MAX_TRANSMITTER_NUM; loop++)
    {
        if (grc.gti[loop].valid)
        {
            if (order)
            {
                if (rank <= grc.gti[loop].rank)
                {
                    ret = loop;
                    rank = grc.gti[loop].rank;
                }
            }
            else
            {
                if (rank >= grc.gti[loop].rank)
                {
                    ret = loop;
                    rank = grc.gti[loop].rank;
                }
            }
        }
    }
    return ret;
}

static bool group_receiver_nvm_save(int index)
{
    uint8_t flash_data[GROUP_FLASH_PARAMS_EACH_TRANSMITTER_SIZE] = {0};
    flash_data[0] = grc.gti[index].valid;
    memcpy(flash_data + 1, grc.gti[index].bt_addr, 6);
    memcpy(flash_data + 7, grc.gti[index].group, GROUP_RECEIVER_MAX_GROUP_NUM_EACH_TRANSMITTER);
    memcpy(flash_data + 7 + GROUP_RECEIVER_MAX_GROUP_NUM_EACH_TRANSMITTER, &grc.gti[index].rank, 4);
    return 0 == ftl_save((void *)flash_data,
                         GROUP_FLASH_PARAMS_OFFSET + index * GROUP_FLASH_PARAMS_EACH_TRANSMITTER_SIZE,
                         grc.gti[index].valid ? GROUP_FLASH_PARAMS_EACH_TRANSMITTER_SIZE : 4);
}

static bool group_receiver_nvm_load(void)
{
    uint32_t ret;
    int loop;
    for (loop = 0; loop < GROUP_RECEIVER_MAX_TRANSMITTER_NUM; loop++)
    {
        uint8_t flash_data[GROUP_FLASH_PARAMS_EACH_TRANSMITTER_SIZE] = {0};
        ret = ftl_load((void *)flash_data,
                       GROUP_FLASH_PARAMS_OFFSET + loop * GROUP_FLASH_PARAMS_EACH_TRANSMITTER_SIZE,
                       GROUP_FLASH_PARAMS_EACH_TRANSMITTER_SIZE);
        if (ret == 0 && flash_data[0] == 1)
        {
            grc.gti[loop].valid = true;
            memcpy(grc.gti[loop].bt_addr, flash_data + 1, 6);
            memcpy(grc.gti[loop].group, flash_data + 7, GROUP_RECEIVER_MAX_GROUP_NUM_EACH_TRANSMITTER);
            memcpy(&grc.gti[loop].rank, flash_data + 7 + GROUP_RECEIVER_MAX_GROUP_NUM_EACH_TRANSMITTER, 4);
        }
    }
    return true;
}

bool group_receiver_nvm_clear(void)
{
    uint32_t ret;
    int loop;
    for (loop = 0; loop < GROUP_RECEIVER_MAX_TRANSMITTER_NUM; loop++)
    {
        uint8_t flash_data[GROUP_FLASH_PARAMS_EACH_TRANSMITTER_SIZE] = {0};
        ret = ftl_load((void *)flash_data,
                       GROUP_FLASH_PARAMS_OFFSET + loop * GROUP_FLASH_PARAMS_EACH_TRANSMITTER_SIZE,
                       GROUP_FLASH_PARAMS_EACH_TRANSMITTER_SIZE);
        if (ret == 0 && flash_data[0] == 1)
        {
            flash_data[0] = 0;
            ftl_save((void *)flash_data, GROUP_FLASH_PARAMS_OFFSET + loop *
                     GROUP_FLASH_PARAMS_EACH_TRANSMITTER_SIZE, 4);
        }
    }
    return true;
}

static void group_cache_init(void)
{
    for (uint8_t loop = 0; loop < GROUP_RECEIVER_MAX_CACHE_SIZE; loop++)
    {
        plt_list_push(&grc.cache_free, &grc.cache[loop]);
    }
}

static bool group_cache_check(uint8_t bt_addr[6], uint8_t tid)
{
    bool ret = true;
    group_cache_t *pentry_pre = NULL;
    group_cache_t *pentry = (group_cache_t *)grc.cache_used.pfirst;
    while (pentry)
    {
        if (0 == memcmp(pentry->bt_addr, bt_addr, 6))
        {
            if (pentry->tid == tid)
            {
                ret = false;
            }
            else
            {
                pentry->tid = tid;
            }
            plt_list_delete(&grc.cache_used, pentry_pre, pentry);
            plt_list_push(&grc.cache_used, pentry);
            return ret;
        }
        pentry_pre = pentry;
        pentry = pentry->pnext;
    }

    if (grc.cache_free.count)
    {
        pentry = plt_list_pop(&grc.cache_free);
    }
    else
    {
        pentry = plt_list_pop(&grc.cache_used);
    }
    memcpy(pentry->bt_addr, bt_addr, 6);
    pentry->tid = tid;
    plt_list_push(&grc.cache_used, pentry);
    return ret;
}

static bool group_receiver_transmit(group_msg_type_t type, uint8_t *pdata, uint8_t len)
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
    pmsg->tid = grc.tid++;
    memcpy(&pmsg->cfg, pdata, len);
    gap_sched_task_p ptask = CONTAINER_OF(pbuffer, gap_sched_task_t, adv_data);
    ptask->adv_type = GAP_SCHED_ADV_TYPE_IND;
    ptask->adv_len = pbuffer[0] + 1;
    ptask->retrans_count = GROUP_RECEIVER_TX_TIMES - 1;
    ptask->retrans_interval = 10;
    printi("group_receiver_transmit: tid %d, type %d, len %d", pmsg->tid,
           pmsg->type, len + MEMBER_OFFSET(group_msg_t, cfg));
    dprinti((uint8_t *)pmsg, len + MEMBER_OFFSET(group_msg_t, cfg));
    gap_sched_try(ptask);
    return true;
}

static bool group_receiver_relay(uint8_t *addr, uint8_t *pdata, uint8_t len)
{
    if (len > 25)
    {
        printw("group_receiver_relay: fail, len %d", len);
        return false;
    }
    group_msg_t *pmsg;
    uint8_t *pbuffer = gap_sched_task_get();
    if (pbuffer == NULL)
    {
        return false;
    }
    memcpy(pbuffer, pdata, len);
    memcpy(pbuffer + len, addr, 6);
    pbuffer[0] += 6;
    pmsg = (group_msg_t *)(pbuffer + 5);
    pmsg->relay = 1;
    gap_sched_task_p ptask = CONTAINER_OF(pbuffer, gap_sched_task_t, adv_data);
    ptask->adv_type = GAP_SCHED_ADV_TYPE_IND;
    ptask->adv_len = len + 6;
    ptask->retrans_count = GROUP_RECEIVER_RELAY_TIMES - 1;
    ptask->retrans_interval = 10;
    printi("group_receiver_relay: len %d", ptask->adv_len);
    dprinti(pbuffer, ptask->adv_len);
    gap_sched_try(ptask);
    return true;
}

void group_receiver_receive(T_LE_SCAN_INFO *ple_scan_info)
{
    group_msg_t *pmsg;
    uint8_t *pbuffer = ple_scan_info->data;
    uint8_t len = ple_scan_info->data_len;
    uint8_t *addr;
    bool relay = false;
    uint8_t rbuffer[31];
    uint8_t rlen;
    /*
    if (ple_scan_info->adv_type != GAP_ADV_EVT_TYPE_NON_CONNECTABLE)
    {
        return;
    }
    */

    if (pbuffer[0] + 1 != len || len < 5 + MEMBER_OFFSET(group_msg_t, cfg))
    {
        return;
    }

    if (pbuffer[1] != GAP_ADTYPE_MANUFACTURER_SPECIFIC)
    {
        return;
    }

    uint16_t company_id = LE_EXTRN2WORD(pbuffer + 2);
    if (company_id != MANUFACTURE_ADV_DATA_COMPANY_ID)
    {
        return;
    }

    uint8_t manual_adv_type = pbuffer[4];
    if (manual_adv_type != MANUFACTURE_ADV_DATA_TYPE_GROUP)
    {
        return;
    }

    pmsg = (group_msg_t *)(pbuffer + 5);
    if (pmsg->rfu != 0)
    {
        return;
    }

    if (false == pmsg->relay)
    {
        addr = ple_scan_info->bd_addr;
    }
    else
    {
        if (len >= 11)
        {
            addr = pbuffer + len - 6;
            len -= 6;
        }
        else
        {
            return;
        }
    }
    int index = group_receiver_find(addr, pmsg->tid);
    if (index <= -2)
    {
        return;
    }
    if (index == -1 && false == group_cache_check(addr, pmsg->tid))
    {
        return;
    }
    memcpy(rbuffer, pbuffer, len);
    rlen = len;

    len -= 5;
    printi("group_receiver_receive: state %d, tid %d, type %d, relay %d, len %d",
           grc.state, pmsg->tid, pmsg->type, pmsg->relay, len);
    dprinti((uint8_t *)pmsg, len);
    len -= MEMBER_OFFSET(group_msg_t, cfg);

    if (pmsg->type == GROUP_MSG_TYPE_CFG && index >= 0)
    {
        relay = true;
    }

    if (grc.state == GROUP_RECEIVER_STATE_CFG && pmsg->type == GROUP_MSG_TYPE_CFG)
    {
        bool save_flash = false;
        if (pmsg->cfg.opcode == GROUP_CFG_OPCODE_GROUP &&
            len == MEMBER_OFFSET(group_cfg_t, group) + sizeof(uint8_t))
        {
            if (pmsg->cfg.group == GROUP_INVALID)
            {
                if (index >= 0)
                {
#if 1
                    /* delete all group */
                    if (group_receiver_group_delete(index))
                    {
                        save_flash = true;
                    }
#else
                    /* delete the transmitter */
                    group_receiver_free(index);
                    save_flash = true;
#endif
                }
            }
            else
            {
                if (index == -1)
                {
                    index = group_receiver_rank(true);
                    uint32_t rank = index >= 0 ? grc.gti[index].rank + 1 : 0x00000000;
                    index = group_receiver_allocate(addr, pmsg->tid, rank);
                    if (index < 0)
                    {
#if GROUP_RECEIVER_PREEMPTIVE_MODE
                        index = group_receiver_rank(false);
                        printw("group_receiver_receive: space %d emptived by rank %d", index, rank);
                        group_receiver_free(index);
                        index = group_receiver_allocate(addr, pmsg->tid, rank);
                        save_flash = true;
#else
                        printw("group_receiver_receive: no space left for the new transmitter");
                        return;
#endif
                    }
                    else
                    {
                        save_flash = true;
                    }
                    relay = true;
                }

                if (pmsg->cfg.group != GROUP_ALL)
                {
                    /* add one group */
                    if (group_receiver_group_add(index, pmsg->cfg.group))
                    {
                        save_flash = true;
                    }
                }
            }
        }

        if (save_flash)
        {
            group_receiver_nvm_save(index);
            if (grc.cfg_cb)
            {
                grc.cfg_cb(ple_scan_info, (uint8_t *)&pmsg->cfg, len);
            }
        }
    }

    //&& grc.state == GROUP_RECEIVER_STATE_NORMAL
    if (pmsg->type == GROUP_MSG_TYPE_CTL && pmsg->ctl.group != GROUP_INVALID)
    {
        if (index >= 0)
        {
            relay = true;
            if (pmsg->ctl.group == GROUP_ALL || group_receiver_group_find(index, pmsg->ctl.group))
            {
                if (grc.ctl_cb)
                {
                    grc.ctl_cb(ple_scan_info, (uint8_t *)&pmsg->ctl, len);
                }
            }
        }
#if GROUP_RECEIVER_RX_EVEN_NOT_CFG
        else
        {
            if (pmsg->ctl.group == GROUP_ALL && false == group_receiver_check())
            {
                relay = true;
                if (grc.ctl_cb)
                {
                    grc.ctl_cb(ple_scan_info, (uint8_t *)&pmsg->ctl, len);
                }
            }
        }
#endif
    }

    if (pmsg->type == GROUP_MSG_TYPE_SYNC)
    {
        relay = true;
        if (grc.si.timer && len == grc.si.sync_len)
        {
            if (pmsg->sync.weight > grc.si.weight)
            {
                return;
            }
            else if (pmsg->sync.weight == grc.si.weight)
            {
                if (memcmp(&pmsg->sync, &grc.si.sync_msg, len) >= 0)
                {
                    return;
                }
            }
            printi("group_receiver_receive: weight %d -> %d", grc.si.weight, pmsg->sync.weight);
            grc.si.quit = true;
            grc.si.weight = pmsg->sync.weight;
            /* callback now with better instant performance, but maybe generate multiple callbacks */
            if (grc.sync_cb)
            {
                grc.sync_cb(ple_scan_info, (uint8_t *)&pmsg->sync, len);
            }
        }
    }

    if (relay)
    {
        group_receiver_relay(addr, rbuffer, rlen);
    }
}

void group_receiver_init(void)
{
    group_receiver_nvm_load();
    group_cache_init();
    plt_rand(&grc.tid, 1);
}

void group_receiver_state_set(group_receiver_state_t state)
{
    grc.state = state;
}

group_receiver_state_t group_receiver_state_get(void)
{
    return grc.state;
}

void group_receiver_reg_cb(group_msg_type_t type, pf_group_receiver_receive_cb_t pf)
{
    if (type == GROUP_MSG_TYPE_CTL)
    {
        grc.ctl_cb = pf;
    }
    else if (type == GROUP_MSG_TYPE_CFG)
    {
        grc.cfg_cb = pf;
    }
    else if (type == GROUP_MSG_TYPE_SYNC)
    {
        grc.sync_cb = pf;
    }
}

extern void *evt_queue_handle;  //!< Event queue handle
extern void *io_queue_handle;   //!< IO queue handle
void group_receiver_sync_timeout(void *timer)
{
    uint8_t event = EVENT_IO_TO_APP;
    T_IO_MSG msg;
    msg.type = GROUP_SYNC_TIMEOUT_MSG;
    if (os_msg_send(io_queue_handle, &msg, 0) == false)
    {
    }
    else if (os_msg_send(evt_queue_handle, &event, 0) == false)
    {
    }
}

void group_receiver_handle_sync_timeout(void)
{
    if (plt_timer_is_active(grc.si.timer))
    {
        printw("group_receive_handle_sync_timeout: fail, timeout outdated!");
        return;
    }
    if (grc.si.phase)
    {
        printi("group_receiver_handle_sync_timeout: done");
        plt_timer_delete(grc.si.timer, 0);
        grc.si.timer = NULL;
        return;
    }

    grc.si.phase = true;
    if (grc.si.quit == false)
    {
        printi("group_receiver_handle_sync_timeout: sync send out");
        group_receiver_transmit(GROUP_MSG_TYPE_SYNC, (uint8_t *)&grc.si.sync_msg, grc.si.sync_len);
    }
    plt_timer_change_period(grc.si.timer, MAX(grc.si.sync_time, 10), 0);
}

void group_receiver_sync(group_sync_t *psync, uint8_t len)
{
    uint32_t delay_ms = 1000 * psync->weight / GROUP_SYNC_DELAY_FACTOR;
    if (grc.si.timer != NULL)
    {
        printw("group_receiver_sync: quit current sync-ing!");
        plt_timer_change_period(grc.si.timer, MAX(delay_ms, 10), 0);
    }
    else
    {
        grc.si.timer = plt_timer_create("grs", MAX(delay_ms, 10), false, 0, group_receiver_sync_timeout);
        plt_timer_start(grc.si.timer, 0);
    }
    printi("group_receiver_sync: weight %d, delay %dms", psync->weight, delay_ms);
    grc.si.quit = false;
    grc.si.phase = false;
    grc.si.sync_time = GROUP_SYNC_PERIOD_MS - delay_ms;
    memcpy(&grc.si.sync_msg, psync, len);
    grc.si.sync_len = len;
    grc.si.weight = psync->weight;
}

