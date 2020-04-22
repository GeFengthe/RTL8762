/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      strip_app.c
* @brief     ali mesh power strip application
* @details
* @author    astor
* @date      2019-4-11
* @version   v1.0
* *********************************************************************************************************
*/

#include "mesh_api.h"
#include "strip_app.h"

#include "light_lightness.h"
#include "light_ctl.h"
#include "ali_model.h"
#include "light_cwrgb_app.h"

#include "rtl876x_gpio.h"
#include "board.h"
#include "app_msg.h"
#include "app_task.h"
#include "strip_timer.h"
#include "system_clock.h"

#define MESH_ALI_SUB_ADDR                                               0xF000

static const uint16_t strip_sub_addr[] = {0xC002, 0xCFFF};
static bool key_press = false;
static timer_data_t temp_buf;

uint8_t temp_tid = 0x04;
uint32_t sys_clk_update = BASE_UNIX_TIME;

strip_data_t strip_data_current =
{
    .unix_time = 0,
    .strip_on_off = 0, //!< 0: off; 1: on
    .timezone = 0,
    .time_sync_period = 0x5A0,
    .time_sync_retry_delay = 0x0A,
    .time_sync_retry_times = 0x0A,
    .has_updated = false,
    .need_retry = false
};

static bool prd_exist = false;
static uint8_t prd_schedule;
static uint16_t prd_unix_time;
static uint8_t prd_state;
static uint16_t ali_gateway_addr = 0xF000;

typedef struct
{
    uint16_t attr_type;
    uint8_t error_code;
    uint8_t index;
} error_data_t;

/* power strip models */
static mesh_model_info_t generic_on_off_server;
static mesh_model_info_t ali_server;


ali_attr_t dcd_tim_set(uint8_t *pdata, ali_attr_set_t *pmsg, uint8_t timer_amount);
ali_attr_t dcd_prd_set(uint8_t *pdata, ali_attr_set_t *pmsg, uint8_t timer_amount);

static int32_t generic_on_off_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                          void *pargs)
{
    generic_on_off_t current_on_off = GENERIC_OFF;
    if (strip_data_current.strip_on_off)
    {
        current_on_off = GENERIC_ON;
    }

    switch (type)
    {
    case GENERIC_ON_OFF_SERVER_GET:
        {
            generic_on_off_server_get_t *pdata = pargs;
            pdata->on_off = current_on_off;
        }
        break;
    case GENERIC_ON_OFF_SERVER_GET_DEFAULT_TRANSITION_TIME:
        {
        }
        break;
    case GENERIC_ON_OFF_SERVER_SET:
        {
            generic_on_off_server_set_t *pdata = pargs;
            if (pdata->total_time.num_steps == pdata->remaining_time.num_steps)
            {
                if (pdata->on_off != current_on_off)
                {
                    if (GENERIC_ON == pdata->on_off)
                    {
                        strip_data_current.strip_on_off = GENERIC_ON;
                        /* TODO: turn on the strip here! */
                        light_rgb_turn_on();
                        light_cwrgb_turn_on();
                    }
                    else
                    {
                        strip_data_current.strip_on_off = GENERIC_OFF;
                        /* TODO: turn off the strip here! */
                        light_rgb_turn_off();
                        light_cwrgb_turn_off();
                    }
                }
            }
        }
        break;
    default:
        break;
    }

    return 0;
}


bool ali_server_receive(mesh_msg_t *pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    ali_gateway_addr = pmesh_msg->src;
    switch (pmesh_msg->access_opcode)
    {
    case MESH_MSG_ALI_ATTR_GET:
        if (pmesh_msg->msg_len > MEMBER_OFFSET(ali_attr_get_t, attr_type))
        {
            ali_attr_get_t *pmsg = (ali_attr_get_t *)pbuffer;
            uint8_t *pdata = (uint8_t *)pmsg->attr_type;
            uint8_t attr_num = (pmesh_msg->msg_len - MEMBER_OFFSET(ali_attr_get_t,
                                                                   attr_type)) / sizeof(ali_attr_type_t);
            ali_attr_t *attr = plt_malloc(sizeof(ali_attr_t) * attr_num, RAM_TYPE_DATA_OFF);
            ali_attr_error_t *error = plt_malloc(sizeof(ali_attr_error_t) * attr_num, RAM_TYPE_DATA_OFF);
            for (uint8_t loop = 0; loop < attr_num; loop++)
            {
                switch (pmsg->attr_type[loop])
                {
                case ALI_ATTR_TYPE_TIMEZONE_SETTING:
                    attr[loop].attr_type = ALI_ATTR_TYPE_TIMEZONE_SETTING;
                    attr[loop].param_len = 1;
                    attr[loop].attr_param = (uint8_t *)&strip_data_current.timezone;
                    break;
                case ALI_ATTR_TYPE_EVENT:
                    error[loop].attr_type = pmsg->attr_type[loop];
                    error[loop].error_code = ALI_ERROR_CODE_NOT_SUPPORTED_ATTR;
                    attr[loop].attr_type = ALI_ATTR_TYPE_ERROR;
                    attr[loop].param_len = sizeof(ali_attr_error_t);
                    attr[loop].attr_param = (uint8_t *)&error[loop];
                    break;
                case ALI_ATTR_TYPE_UNIX_TIME:
                    attr[loop].attr_type = ALI_ATTR_TYPE_UNIX_TIME;
                    attr[loop].param_len = 4;
                    attr[loop].attr_param = (uint8_t *) &strip_data_current.unix_time;
                    break;
                case ALI_ATTR_TYPE_TIMING_SETTING:
                    {
                        uint8_t timer_amount = pmesh_msg->msg_len - 6;
                        uint8_t *timer_data = plt_malloc(timer_amount * 8, RAM_TYPE_DATA_ON);
                        pdata += sizeof(ali_attr_type_t);
                        DBG_DIRECT("index = %d", *pdata);
                        bool got_timer = false;
                        for (int n = 0; n < timer_amount; n++)
                        {
                            for (int i = 0; i < TIMER_MAXIMUM; i++)
                            {
                                if (timer_list[i].index == *pdata)
                                {
                                    DBG_DIRECT("timer with index %d is %x", *pdata, timer_list[i].unix_time);
                                    memcpy(timer_data + 8 * n, &timer_list[i], sizeof(timer_data_t));
                                    attr[loop].attr_type = ALI_ATTR_TYPE_TIMING_SETTING;
                                    attr[loop].param_len = 8 * timer_amount;
                                    attr[loop].attr_param = timer_data;
                                    got_timer = true;
                                }
                            }
                        }
                        if (!got_timer)
                        {
                            error[loop].attr_type = pmsg->attr_type[loop];
                            error[loop].error_code = ALI_ERROR_CODE_INVALID_PARAMETER;
                            attr[loop].attr_type = ALI_ATTR_TYPE_ERROR;
                            attr[loop].param_len = sizeof(ali_attr_error_t);
                            attr[loop].attr_param = (uint8_t *)&error[loop];
                        }
                    }
                    break;
                case ALI_ATTR_TYPE_TIME_SYNC:
                    attr[loop].attr_type = ALI_ATTR_TYPE_TIME_SYNC;
                    attr[loop].param_len = 4;
                    attr[loop].attr_param = (uint8_t *)&strip_data_current.time_sync_period;
                    break;
                default:
                    error[loop].attr_type = pmsg->attr_type[loop];
                    error[loop].error_code = ALI_ERROR_CODE_NOT_SUPPORTED_ATTR;
                    attr[loop].attr_type = ALI_ATTR_TYPE_ERROR;
                    attr[loop].param_len = sizeof(ali_attr_error_t);
                    attr[loop].attr_param = (uint8_t *)&error[loop];
                    break;
                }
            }
            ali_attr_msg(&ali_server, pmesh_msg->src, pmesh_msg->app_key_index, MESH_MSG_ALI_ATTR_STAT,
                         pmsg->tid, attr, attr_num);
            plt_free(attr, RAM_TYPE_DATA_OFF);
            plt_free(error, RAM_TYPE_DATA_OFF);
        }
        break;
    case MESH_MSG_ALI_ATTR_SET:
    case MESH_MSG_ALI_ATTR_SET_UNACK:
        if (pmesh_msg->msg_len > MEMBER_OFFSET(ali_attr_set_t, attr_type))
        {
            ali_attr_set_t *pmsg = (ali_attr_set_t *)pbuffer;
            uint8_t loop = 0;
            ali_attr_t attr[4];
            ali_attr_error_t error[4];
            uint8_t *pdata = (uint8_t *)pmsg->attr_type;
            while (pdata != pbuffer + pmesh_msg->msg_len && loop < sizeof(attr) / sizeof(ali_attr_t))
            {
                ali_attr_type_t attr_type = LE_EXTRN2WORD(pdata);
                switch (attr_type)
                {
                case ALI_ATTR_TYPE_TIMING_SETTING:
                    {
                        pdata += sizeof(ali_attr_type_t);
                        uint8_t timer_amount = (pmesh_msg->msg_len - 6) / 8;
                        for (int i = 0; i < timer_amount; i++)
                        {
                            attr[loop] = dcd_tim_set(pdata, pmsg, timer_amount);
                            pdata += 8;
                        }
                    }
                    break;
                case ALI_ATTR_TYPE_PERIODIC_SETTING:
                    {
                        pdata += sizeof(ali_attr_type_t);
                        uint8_t timer_amount = (pmesh_msg->msg_len - 6) / 7;
                        DBG_DIRECT("pmesh_msg->msg_len - 6 = %d", pmesh_msg->msg_len - 6);
                        for (int i = 0; i < timer_amount; i++)
                        {
                            attr[loop] = dcd_prd_set(pdata, pmsg, timer_amount);
                            pdata += 7;
                        }
                    }
                    break;
                case ALI_ATTR_TYPE_UNIX_TIME:
                    pdata += sizeof(ali_attr_type_t);
                    for (int i = 3; i >= 0; i--)
                    {
                        strip_data_current.unix_time = (strip_data_current.unix_time << 8) + *(pdata + i);
                    }
                    sys_clk_update = strip_data_current.unix_time;
                    strip_data_current.has_updated = true;
                    pdata += 4;
                    DBG_DIRECT("current unix time is %x", strip_data_current.unix_time);
                    unix2UTC(strip_data_current.unix_time);
                    attr[loop].attr_type = attr_type;
                    attr[loop].param_len = 4;
                    attr[loop].attr_param = (uint8_t *)&strip_data_current.unix_time;
                    break;
                case ALI_ATTR_TYPE_TIMEZONE_SETTING:
                    pdata += sizeof(ali_attr_type_t);
                    strip_data_current.timezone = *pdata;
                    pdata++;
                    attr[loop].attr_type = ALI_ATTR_TYPE_TIMEZONE_SETTING;
                    attr[loop].param_len = 1;
                    attr[loop].attr_param = &strip_data_current.timezone;
                    break;
                case ALI_ATTR_TYPE_REMOVE_TIMING:
                    {
                        uint8_t timer_amount = pmesh_msg->msg_len - 6;
                        pdata += sizeof(ali_attr_type_t);
                        for (int i = 0; i < timer_amount; i++)
                        {
                            remove_timer(*pdata);
                            pdata++;
                        }
                        attr[loop].attr_type = ALI_ATTR_TYPE_REMOVE_TIMING;
                        attr[loop].param_len = timer_amount;
                        attr[loop].attr_param = (uint8_t *)pmsg->attr_type + sizeof(ali_attr_type_t);
                    }
                    break;
                case ALI_ATTR_TYPE_TIME_SYNC:
                    pdata += sizeof(ali_attr_type_t);
                    strip_data_current.time_sync_period = (*pdata++);
                    strip_data_current.time_sync_period = (strip_data_current.time_sync_period << 8) + (*pdata++);
                    strip_data_current.time_sync_retry_delay = *pdata++;
                    strip_data_current.time_sync_retry_times = *pdata++;
                    attr[loop].attr_type = ALI_ATTR_TYPE_TIME_SYNC;
                    attr[loop].param_len = 4;
                    attr[loop].attr_param = (uint8_t *)&strip_data_current.time_sync_period;
                    break;
                default:
                    error[loop].attr_type = attr_type;
                    error[loop].error_code = ALI_ERROR_CODE_NOT_SUPPORTED_ATTR;
                    attr[loop].attr_type = ALI_ATTR_TYPE_ERROR;
                    attr[loop].param_len = sizeof(ali_attr_error_t);
                    attr[loop].attr_param = (uint8_t *)&error[loop];
                    loop++;
                    DBG_DIRECT("ali_server_receive: unsupported 0x%04x", attr_type);
                    goto ali_attr_set_fail;
                }
                loop++;
            }
ali_attr_set_fail:
            if (pmesh_msg->access_opcode == MESH_MSG_ALI_ATTR_SET)
            {
                ali_attr_msg(&ali_server, pmesh_msg->src, pmesh_msg->app_key_index, MESH_MSG_ALI_ATTR_STAT,
                             pmsg->tid, attr, loop);
            }
        }
        break;
    case MESH_MSG_ALI_ATTR_CONF:
        if (pmesh_msg->msg_len == sizeof(ali_attr_conf_t))
        {

        }
        break;
    case MESH_MSG_ALI_TRANSPARENT_MSG:
        break;
    case MESH_MSG_ALI_ATTR_RESP:
        if (pmesh_msg->msg_len > MEMBER_OFFSET(ali_attr_set_t, attr_type))
        {
            ali_attr_set_t *pmsg = (ali_attr_set_t *)pbuffer;
            uint8_t loop = 0;
            uint8_t *pdata = (uint8_t *)pmsg->attr_type;
            while (pdata != pbuffer + pmesh_msg->msg_len && loop < 4)
            {
                ali_attr_type_t attr_type = LE_EXTRN2WORD(pdata);
                DBG_DIRECT("current attr: %x", attr_type);
                switch (attr_type)
                {
                case ALI_ATTR_TYPE_UNIX_TIME:
                    pdata += sizeof(ali_attr_type_t);
                    for (int i = 3; i >= 0; i--)
                    {
                        strip_data_current.unix_time = (strip_data_current.unix_time << 8) + *(pdata + i);
                    }
                    pdata += 4;
                    DBG_DIRECT("current unix time is %x", strip_data_current.unix_time);
                    unix2UTC(strip_data_current.unix_time);
                    break;
                default:
                    loop++;
                    DBG_DIRECT("ali_server_receive1: unsupported 0x%04x", attr_type);
                }
                loop++;
            }
        }
        break;
    default:
        ret = FALSE;
        break;
    }
    return ret;
}

void strip_server_models_init(uint8_t element_index)
{
    /* binding models */
    ali_server.pmodel_bound = &generic_on_off_server;

    /* register models */
    generic_on_off_server.model_data_cb = generic_on_off_server_data;
    generic_on_off_server_reg(element_index, &generic_on_off_server);

    ali_server.model_receive = ali_server_receive;
    ali_model_reg(element_index, &ali_server, TRUE);

    mesh_model_pub_params_t pub_params;
    pub_params.pub_addr = MESH_ALI_SUB_ADDR;
    pub_params.pub_key_info.app_key_index = 0;
    pub_params.pub_key_info.frnd_flag = 0;
    pub_params.pub_ttl = 10;
    mesh_model_pub_params_set(generic_on_off_server.pmodel, pub_params);

    timer_list_init();
    period_list_init();
}

void strip_server_models_sub(void)
{
    for (uint8_t loop = 0; loop < sizeof(strip_sub_addr) / sizeof(uint16_t); loop++)
    {
        mesh_model_sub(generic_on_off_server.pmodel, strip_sub_addr[loop]);
        mesh_model_sub(ali_server.pmodel, strip_sub_addr[loop]);
    }
    strip_data_current.strip_on_off = GENERIC_OFF;
    strip_data_current.timezone = 8;
    strip_data_current.unix_time = 0xFFFFFFFF;
}

/**
 * @brief suppose there is a switch on power strip, send state change publish
 * @note do not handle the event during interrupt, send a message to task instead
*/
void GPIO_CTRL_Handler(void)
{
    T_IO_MSG bee_io_msg;
    GPIO_MaskINTConfig(GPIO_CTRL_PIN, ENABLE);
    uint8_t keystatus = GPIO_ReadInputDataBit(GPIO_CTRL_PIN);
    if (keystatus)
    {
        GPIO->INTPOLARITY &= ~GPIO_CTRL_PIN;
        key_press = true;
    }
    else
    {
        GPIO->INTPOLARITY |= GPIO_CTRL_PIN;
        if (key_press)
        {
            key_press = false;
            bee_io_msg.type = IO_MSG_TYPE_GPIO;
            if (strip_data_current.strip_on_off == GENERIC_ON)
            {
                bee_io_msg.subtype = GENERIC_OFF;
                strip_data_current.strip_on_off = GENERIC_OFF;
            }
            else
            {
                bee_io_msg.subtype = GENERIC_ON;
                strip_data_current.strip_on_off = GENERIC_ON;
            }
            app_send_msg_to_apptask(&bee_io_msg);
        }
    }
    GPIO_ClearINTPendingBit(GPIO_CTRL_PIN);
    GPIO_MaskINTConfig(GPIO_CTRL_PIN, DISABLE);
}

/**
 * @brief erase flash with specific type and address
 * @param type  erase type
 * @param addr  address to erase when erase block or sector
 * @return true if success
 * @note use this function only when no concern to be preempted, otherwise, use the locked one
*/
void handle_pub_evt(generic_on_off_t on_off)
{
    if (on_off == GENERIC_ON)
    {
        mesh_msg_send_cause_t ret = generic_on_off_publish(&generic_on_off_server, GENERIC_ON);
        DBG_DIRECT("cause ret = %d", ret);
    }
    else
    {
        mesh_msg_send_cause_t ret = generic_on_off_publish(&generic_on_off_server, GENERIC_OFF);
        DBG_DIRECT("cause ret = %d", ret);
    }
}

/**
 * @brief set disposable timer
 * @param pdata  addr of received, start from the next byte of opcode
 * @param pmsg  ali attribute data struct
 * @return attribute used to report to cloud
 * @note local system time will be updated before setting timer
*/
ali_attr_t dcd_tim_set(uint8_t *pdata, ali_attr_set_t *pmsg, uint8_t timer_amount)
{
    ali_attr_t attr;
    uint32_t factor = 0x01;
    uint8_t state;
    uint16_t model_id = 0;
    uint32_t unix_time = 0;
    uint8_t index = 0;
    timer_data_t *timer_data;
    timer_data = &temp_buf;
    T_IO_MSG bee_timer_msg;
//    pdata += sizeof(ali_attr_type_t);
    index = *pdata;
    pdata++;
    for (int dig = 0; dig < 4; dig++)
    {
        unix_time = unix_time + *(pdata) * factor;
        factor = (factor << 8);
        pdata++;
    }
    unix_time = (unix_time / 60) * 60;
    timer_data->attr_type = (*pdata) + *(pdata + 1) * 0x0100;
    pdata = pdata + 2;
    state = *pdata;
    timer_data->index = index;
    timer_data->unix_time = unix_time;
    timer_data->on_off = state;
    uint8_t error_code = is_in_timer_list(temp_buf);
    if (error_code)
    {
        attr.attr_type = ALI_ATTR_TYPE_ERROR;
        attr.param_len = 4;
        error_data_t error;
        error.attr_type = ALI_ATTR_TYPE_TIMING_SETTING;
        error.error_code = error_code;
        error.index = index;
        attr.attr_param = (uint8_t *)&error;
        return attr;
    }
    bee_timer_msg.type = IO_MSG_TYPE_RTC;
    bee_timer_msg.subtype = IO_MSG_TIMER_ALARM;
    bee_timer_msg.u.buf = timer_data;
    app_send_msg_to_apptask(&bee_timer_msg);

    attr.attr_type = ALI_ATTR_TYPE_TIMING_SETTING;
    attr.param_len = 8 * timer_amount;
    attr.attr_param = (uint8_t *)pmsg->attr_type + sizeof(ali_attr_type_t);
    DBG_DIRECT("ali_server_receive: set index %x to %x, model is %x, on_off states = %d", index,
               unix_time,
               model_id, state);
    return attr;
}

/**
 * @brief period timer set
 * @param pdata  addr of received, start from the next byte of opcode
 * @param pmsg  ali attribute data struct
 * @note this routine hasn't been completed by Ali yet.
*/
ali_attr_t dcd_prd_set(uint8_t *pdata, ali_attr_set_t *pmsg, uint8_t timer_amount)
{
    ali_attr_t attr;
    periodic_data_t temp_data;
    temp_data.index = *pdata;
    pdata++;
    temp_data.relative_time = (*(pdata + 1) << 8) + *(pdata);
    pdata += 2;
//    uint8_t opnum = (uint8_t)((prd_unix_time&0xF000)>>12);
    temp_data.relative_time = temp_data.relative_time & 0x0fff;
    temp_data.schedule =  *pdata;
    pdata++;
    temp_data.attr_type = (*pdata) + *(pdata + 1) * 0x0100;
    pdata = pdata + 2;
    temp_data.on_off = *pdata;
    is_in_period_list(temp_data);
    attr.attr_type = ALI_ATTR_TYPE_PERIODIC_SETTING;
    attr.param_len = 7 * timer_amount;
    attr.attr_param = (uint8_t *)pmsg->attr_type + sizeof(ali_attr_type_t);
    DBG_DIRECT("ali_server_receive: set index %d to %x on %x, model is %x, on_off states = %d",
               temp_data.index, temp_data.relative_time,
               temp_data.schedule, temp_data.attr_type, temp_data.on_off);
    return attr;
}

/**
 * @brief request for a time update
 * @note send a request mesh message
*/
void send_update_request(void)
{
    ali_attr_t attr[1];
    attr[0].attr_type = ALI_ATTR_TYPE_UNIX_TIME;
    attr[0].param_len = 0;

    ali_attr_msg(&ali_server, ali_gateway_addr, 0, MESH_MSG_ALI_ATTR_REQ,
                 temp_tid, attr, 1);
}


/**
 * @brief report timer timeout message to host
 * @param num  index of timer
 * @note this routine hasn't been completed by Ali yet.
*/
void clear_timer(uint8_t num)
{
    ali_attr_t attr[1];
    attr[0].attr_type = ALI_ATTR_TYPE_EVENT;
    attr[0].param_len = 2;
    uint8_t val[2] = {0x11, timer_list[num].index};
    attr[0].attr_param = val;
    ali_attr_msg(&ali_server, ali_gateway_addr, 0, MESH_MSG_ALI_ATTR_IND,
                 temp_tid, attr, 1);
    timer_list[0].index = 0xFF;
    timer_list[0].unix_time = 0xFFFFFFFF;
    timer_list[0].on_off = GENERIC_OFF;
    timer_list[0].attr_type = 0xFF;
    temp_tid++;
}

void clear_period(uint8_t num)
{
    ali_attr_t attr[1];
    attr[0].attr_type = ALI_ATTR_TYPE_EVENT;
    attr[0].param_len = 2;
    uint8_t val[2] = {0x11, period_list[num].index};
    attr[0].attr_param = val;
    ali_attr_msg(&ali_server, ali_gateway_addr, 0, MESH_MSG_ALI_ATTR_IND,
                 temp_tid, attr, 1);
//    period_list[0].index = 0xFF;
//    period_list[0].relative_time = 0xFFFF;
//    period_list[0].on_off = GENERIC_OFF;
//    period_list[0].schedule = 0;
//    period_list[0].attr_type = 0xFF;
    temp_tid++;
}

/**
 * @brief get information of timer
 * @return status of timer
*/
uint16_t get_prd_unix_time(void)
{
    return prd_unix_time;
}

uint8_t get_prd_schedule(void)
{
    return prd_schedule;
}

uint8_t get_prd_state(void)
{
    return prd_state;
}

bool is_prd_exist(void)
{
    return prd_exist;
}

