/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      fan_app.c
* @brief     ali fan application
* @details
* @author    bill
* @date      2019-3-26
* @version   v1.0
* *********************************************************************************************************
*/

#include "mesh_api.h"
#include "ali_ctl_light_app.h"
#include "generic_on_off.h"
#include "light_lightness.h"
#include "light_ctl.h"
#include "scene.h"
#include "ali_model.h"
#include "light_cwrgb_app.h"
#include "light_storage_app.h"
#include "vendor_timer.h"
#include "system_clock.h"
#include "app_msg.h"
#include "app_task.h"
#include  "light_swtimer.h"
#include "platform_utils.h"
#include "board.h"
static const uint16_t light_ctl_sub_addr[] = {0xC000, 0xCFFF};

#define MESH_ALI_SUB_ADDR 0xF000
#define MESH_MSG_ALI_ATTR_REQUEST                           0xDEA801

static timer_data_t temp_buf;
uint8_t temp_tid = 0x80;
uint32_t sys_clk_update = BASE_UNIX_TIME;

generic_data_t generic_data_current;
light_flash_vendor_light_state_t vendor_light_state;
static bool prd_exist = false;
static uint8_t prd_schedule;
static uint16_t prd_unix_time;
static uint8_t prd_state;

void message_report(uint8_t event);
ali_attr_t dcd_tim_set(uint8_t *pdata, ali_attr_set_t *pmsg);
ali_attr_t dcd_prd_set(uint8_t *pdata, ali_attr_set_t *pmsg);

static light_ctl_t scene_read = {65535, 0x4e20, 0};
static light_ctl_t scene_cinema = {65535, 0, 0};
static light_ctl_t scene_warm = {0, 0x4e20, 0};
static light_ctl_t scene_night = {0, 2000, 0};

static light_ctl_t scene_get_up = {0, 0, 0};
static light_ctl_t scene_sleep = {0, 0, 0};
static light_ctl_t scene_eye_care = {0, 0, 0};
static light_ctl_t scene_normal = {0, 0, 0};
static light_ctl_t scene_leisure = {0, 0, 0};
static light_ctl_t scene_timing_rest = {0, 0, 0};

static scene_storage_memory_t scene_storage_memory[] =
{
    {3, &scene_read},
    {4, &scene_cinema},
    {5, &scene_warm},
    {6, &scene_night},
    {8, &scene_get_up},
    {14, &scene_sleep},
    {57, &scene_eye_care},
    {353, &scene_normal},
    {378, &scene_leisure},
    {409, &scene_timing_rest}
};

static uint16_t current_scene;
static generic_data_t generic_data_current;

/* ctl light models */
static mesh_model_info_t generic_on_off_server;
static mesh_model_info_t light_lightness_server;
static mesh_model_info_t light_ctl_server;
static mesh_model_info_t light_scene_server;
static mesh_model_info_t ali_server;
static msg_info_t msg_report_info;
plt_timer_t power_on_msg_timer;
//static plt_timer_t light_pub_timer = NULL;

static int32_t generic_on_off_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                          void *pargs)
{
    generic_on_off_t current_on_off = GENERIC_OFF;
    if ((light_get_cold()->lightness) ||
        (light_get_warm()->lightness))
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
                        light_cw_turn_on();
                        light_rgb_turn_on();
                    }
                    else
                    {
                        light_cw_turn_off();
                        light_rgb_turn_off();
                    }
                    light_state_store();
                }
            }
        }
        break;
    default:
        break;
    }

    return 0;
}



static int32_t light_lightness_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                           void *pargs)
{
    switch (type)
    {
    case LIGHT_LIGHTNESS_SERVER_GET:
        {
            light_lightness_server_get_t *pdata = pargs;
            pdata->lightness = light_get_ctl().lightness;
        }
        break;
    case LIGHT_LIGHTNESS_SERVER_GET_LINEAR:
        {
            light_lightness_server_get_t *pdata = pargs;
            pdata->lightness = light_lightness_actual_to_linear(light_get_ctl().lightness);
        }
        break;
    case LIGHT_LIGHTNESS_SERVER_GET_DEFAULT:
        {
        }
        break;
    case LIGHT_LIGHTNESS_SERVER_GET_LAST:
        {
            light_lightness_server_get_t *pdata = pargs;
            pdata->lightness = light_get_cold()->lightness_last;
        }
        break;
    case LIGHT_LIGHTNESS_SERVER_GET_RANGE:
        {
        }
        break;
    case LIGHT_LIGHTNESS_SERVER_GET_DEFAULT_TRANSITION_TIME:
        {
        }
        break;
    case LIGHT_LIGHTNESS_SERVER_SET:
        {
            light_lightness_server_set_t *pdata = pargs;
            if (pdata->total_time.num_steps == pdata->remaining_time.num_steps)
            {
                light_ctl_t ctl = light_get_ctl();
                ctl.lightness = pdata->lightness;
                light_set_ctl(ctl);
                light_state_store();
            }
        }
        break;
    case LIGHT_LIGHTNESS_SERVER_SET_LINEAR:
        {
            light_lightness_server_set_t *pdata = pargs;
            if (pdata->total_time.num_steps == pdata->remaining_time.num_steps)
            {
                light_ctl_t ctl = light_get_ctl();
                ctl.lightness = light_lightness_linear_to_actual(pdata->lightness);
                light_set_ctl(ctl);
                light_state_store();
            }
        }
        break;
    case LIGHT_LIGHTNESS_SERVER_SET_LAST:
        {
        }
        break;
    case LIGHT_LIGHTNESS_SERVER_SET_DEFAULT:
        {
        }
        break;
    case LIGHT_LIGHTNESS_SERVER_SET_RANGE:
        {
        }
        break;
    default:
        break;
    }

    return 0;
}

static int32_t light_ctl_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                     void *pargs)
{
    switch (type)
    {
    case LIGHT_CTL_SERVER_GET:
        {
            light_ctl_server_get_t *pdata = pargs;
            light_ctl_t ctl = light_get_ctl();
            pdata->lightness = ctl.lightness;
            pdata->temperature = ctl.temperature;
        }
        break;
    case LIGHT_CTL_SERVER_GET_DEFAULT:
        {
        }
        break;
    case LIGHT_CTL_SERVER_GET_TEMPERATURE:
        {
            light_ctl_server_get_temperature_t *pdata = pargs;
            light_ctl_t ctl = light_get_ctl();
            pdata->temperature = ctl.temperature;
            pdata->delta_uv = ctl.delta_uv;
        }
        break;
    case LIGHT_CTL_SERVER_GET_TEMPERATURE_RANGE:
        {
        }
        break;
    case LIGHT_CTL_SERVER_GET_DEFAULT_TRANSITION_TIME:
        {
        }
        break;
    case LIGHT_CTL_SERVER_SET:
        {
            light_ctl_server_set_t *pdata = pargs;
            if (pdata->total_time.num_steps == pdata->remaining_time.num_steps)
            {
                light_ctl_t ctl = light_get_ctl();
                ctl.lightness = pdata->lightness;
                ctl.temperature = pdata->temperature;
                ctl.delta_uv = pdata->delta_uv;
                light_set_ctl(ctl);
                light_state_store();
            }
        }
        break;
    case LIGHT_CTL_SERVER_SET_TEMPERATURE:
        {
            light_ctl_server_set_temperature_t *pdata = pargs;
            if (pdata->total_time.num_steps == pdata->remaining_time.num_steps)
            {
                light_ctl_t ctl = light_get_ctl();
                ctl.temperature = pdata->temperature;
                ctl.delta_uv = pdata->delta_uv;
                light_set_ctl(ctl);
                light_state_store();
            }
        }
        break;
    case LIGHT_CTL_SERVER_SET_DEFAULT:
        {
        }
        break;
    case LIGHT_CTL_SERVER_SET_TEMPERATURE_RANGE:
        {
        }
        break;
    default:
        break;
    }

    return 0;
}

static int32_t scene_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                 void *pargs)
{
    switch (type)
    {
    case SCENE_SERVER_GET:
        {
            scene_server_get_t *pdata = pargs;
            pdata->current_scene = current_scene;
        }
        break;
    case SCENE_SERVER_GET_REGISTER_STATUS:
        {
        }
        break;
    case SCENE_SERVER_GET_DEFAULT_TRANSITION_TIME:
        {
        }
        break;
    case SCENE_SERVER_STORE:
        {
        }
        break;
    case SCENE_SERVER_RECALL:
        {
            scene_server_recall_t *pdata = pargs;
            if (pdata->remaining_time.num_steps == pdata->total_time.num_steps)
            {
                if (NULL != pdata->pmemory)
                {
                    light_ctl_t *pstate = pdata->pmemory;
                    printi("lightness %x,temperature %x,delta_uv %x", pstate->lightness, pstate->temperature,
                           pstate->delta_uv);
                    light_ctl_t ctl = {pstate->lightness, pstate->temperature, pstate->delta_uv};
                    light_set_ctl(ctl);

                    light_state_store();

                    light_ctl_publish(&light_ctl_server, pstate->lightness, pstate->temperature);
                }
                current_scene = pdata->scene_number;
            }
        }
        break;
    case SCENE_SERVER_DELETE:
        {
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
                case ALI_ATTR_TYPE_ON_OFF:     //0x0100
                    {
                        generic_on_off_server_get_t get_data = {GENERIC_OFF};
                        attr[loop].attr_type = ALI_ATTR_TYPE_LEVEL;
                        attr[loop].param_len = 1;
                        generic_on_off_server_data(&generic_on_off_server, GENERIC_ON_OFF_SERVER_GET, &get_data);
                        attr[loop].attr_param = (uint8_t *)&get_data.on_off;
                    }
                    break;
                case ALI_ATTR_TYPE_HSL_COLOR:  //0x0123
                    {
                        light_hsl_t hsl_data;
                        attr[loop].attr_type = ALI_ATTR_TYPE_HSL_COLOR;
                        attr[loop].param_len = 3;
                        hsl_data = light_get_hsl();
                        attr[loop].attr_param = (uint8_t *)&hsl_data;
                    }
                    break;
                case ALI_ATTR_TYPE_LIGHTNESS_LEVEL: //0x0121
                    {
                        light_lightness_server_get_t lightness_data;
                        attr[loop].attr_type = ALI_ATTR_TYPE_LIGHTNESS_LEVEL;
                        attr[loop].param_len = 2;
                        lightness_data.lightness = light_get_cold()->lightness;
                        attr[loop].attr_param = (uint8_t *)&lightness_data.lightness;
                    }
                    break;
                case ALI_ATTR_TYPE_COLOR_TEMPERATURE: //0x0122
                    {
                        light_ctl_t ctl = light_get_ctl();
                        attr[loop].attr_type = ALI_ATTR_TYPE_COLOR_TEMPERATURE;
                        attr[loop].param_len = 2;
                        attr[loop].attr_param = (uint8_t *)&ctl.temperature;
                    }
                    break;
                case ALI_ATTR_TYPE_MAIN_LIGHT_ONOFF:  //0x0534 need add
                    attr[loop].attr_type = ALI_ATTR_TYPE_BACKGROUND_LIGHT_ONOFF;
                    attr[loop].param_len = 1;
                    attr[loop].attr_param = (uint8_t *)&vendor_light_state.main_on_off;
                    break;
                case ALI_ATTR_TYPE_BACKGROUND_LIGHT_ONOFF:
                    attr[loop].attr_type = ALI_ATTR_TYPE_BACKGROUND_LIGHT_ONOFF;
                    attr[loop].param_len = 1;
                    attr[loop].attr_param = (uint8_t *)&vendor_light_state.bg_on_off;
                    break;
                case ALI_ATTR_TYPE_SCENE_MODE:        //0xf004
                    attr[loop].attr_type = ALI_ATTR_TYPE_SCENE_MODE;
                    attr[loop].param_len = 2;
                    attr[loop].attr_param = (uint8_t *)&current_scene;
                    break;
                case ALI_ATTR_TYPE_EVENT:// 0xf009
                    break;
                case ALI_ATTR_TYPE_TIMEZONE_SETTING: //0xF01E
                    attr[loop].attr_type = ALI_ATTR_TYPE_TIMEZONE_SETTING;
                    attr[loop].param_len = 1;
                    attr[loop].attr_param = (uint8_t *)&generic_data_current.timezone;
                    break;
                case ALI_ATTR_TYPE_UNIX_TIME:
                    attr[loop].attr_type = ALI_ATTR_TYPE_UNIX_TIME; //0xF01F
                    attr[loop].param_len = 4;
                    attr[loop].attr_param = (uint8_t *) &generic_data_current.unix_time;
                    break;
                case ALI_ATTR_TYPE_TIMING_SETTING:         //0xF010
                    pdata += sizeof(ali_attr_type_t);
                    DBG_DIRECT("index = %d", *pdata);
                    for (int i = 0; i < TIMER_MAXIMUM; i++)
                    {
                        if (timer_list[i].index == *pdata)
                        {
                            DBG_DIRECT("timer with index %d is %x", *pdata, timer_list[i].unix_time);
                            attr[loop].attr_type = ALI_ATTR_TYPE_TIMING_SETTING;
                            attr[loop].param_len = 4;
                            attr[loop].attr_param = (uint8_t *) &timer_list[i].unix_time;
                            goto ali_attr_timer_got;
                        }
                    }
                    error[loop].attr_type = pmsg->attr_type[loop];
                    error[loop].error_code = ALI_ERROR_CODE_INVALID_PARAMETER;
                    attr[loop].attr_type = ALI_ATTR_TYPE_ERROR;
                    attr[loop].param_len = sizeof(ali_attr_error_t);
                    attr[loop].attr_param = (uint8_t *)&error[loop];
ali_attr_timer_got:
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
                case ALI_ATTR_TYPE_ON_OFF:   //0x0100   //ALI_ATTR_TYPE_LEVEL:
                    //cloud not send this opcode to device
                    break;
                case ALI_ATTR_TYPE_HSL_COLOR:  //0x0123
                    {
                        light_hsl_t *p_hsl_data;
                        light_hsl_t hsl_data;
                        p_hsl_data = (light_hsl_t *)(pdata + sizeof(ali_attr_type_t));
                        pdata += sizeof(ali_attr_type_t) + 3;
                        hsl_data.lightness = p_hsl_data->lightness;
                        hsl_data.hue = p_hsl_data->hue;
                        hsl_data.saturation = p_hsl_data->saturation;
                        light_set_hsl(hsl_data);
                        light_state_store();
                        attr[loop].attr_type = ALI_ATTR_TYPE_HSL_COLOR;
                        attr[loop].param_len = 3;
                        attr[loop].attr_param = (uint8_t *)&hsl_data;
                        /* TODO: set hsl here! */
                        printi("ali_server_receive: set hsl(lightness:%x,hue:%x,saturation:%x) to %d", hsl_data.lightness,
                               hsl_data.hue, hsl_data.saturation);
                    }
                    break;
                case ALI_ATTR_TYPE_LIGHTNESS_LEVEL: //0x0121
                    //cloud not send this opcode to device
                    break;
                case ALI_ATTR_TYPE_COLOR_TEMPERATURE: //0x0122
                    //cloud not send this opcode to device
                    break;
                case ALI_ATTR_TYPE_MAIN_LIGHT_ONOFF:  //0x0534 need add
                    vendor_light_state.main_on_off = *(pdata + sizeof(ali_attr_type_t));
                    pdata += sizeof(ali_attr_type_t) + 1;
                    attr[loop].attr_type = ALI_ATTR_TYPE_MAIN_LIGHT_ONOFF;
                    attr[loop].param_len = 1;
                    attr[loop].attr_param = (uint8_t *)&vendor_light_state.main_on_off;
                    /* TODO: set main_on_off here! */
                    printi("ali_server_receive: set main_on_off to %d", vendor_light_state.main_on_off);
                    break;
                case ALI_ATTR_TYPE_BACKGROUND_LIGHT_ONOFF: //0x0533 need add
                    vendor_light_state.bg_on_off = *(pdata + sizeof(ali_attr_type_t));
                    pdata += sizeof(ali_attr_type_t) + 1;
                    attr[loop].attr_type = ALI_ATTR_TYPE_BACKGROUND_LIGHT_ONOFF;
                    attr[loop].param_len = 1;
                    attr[loop].attr_param = (uint8_t *)&vendor_light_state.bg_on_off;
                    /* TODO: set bg_on_off here! */
                    printi("ali_server_receive: set bg_on_off to %d", vendor_light_state.bg_on_off);
                    break;
                case ALI_ATTR_TYPE_SCENE_MODE:             //0xf004
                    //cloud not send this opcode to device
                    break;
                case ALI_ATTR_TYPE_EVENT:// 0xf009
                    break;
                case ALI_ATTR_TYPE_TIMING_SETTING:
                    attr[loop] = dcd_tim_set(pdata, pmsg);
                    pdata += sizeof(ali_attr_type_t) + 8;
                    break;
                case ALI_ATTR_TYPE_PERIODIC_SETTING:
                    attr[loop] = dcd_prd_set(pdata, pmsg);
                    pdata += sizeof(ali_attr_type_t) + 7;
                    break;
                case ALI_ATTR_TYPE_UNIX_TIME:
                    pdata += sizeof(ali_attr_type_t);
                    for (int i = 3; i >= 0; i--)
                    {
                        generic_data_current.unix_time = (generic_data_current.unix_time << 8) + *(pdata + i);
                    }
                    sys_clk_update = generic_data_current.unix_time;
                    pdata += 4;
                    DBG_DIRECT("current unix time is %x", generic_data_current.unix_time);
                    unix2UTC(generic_data_current.unix_time);
                    attr[loop].attr_type = attr_type;
                    attr[loop].param_len = 0;
                    break;
                case ALI_ATTR_TYPE_TIMEZONE_SETTING:
                    pdata += sizeof(ali_attr_type_t);
                    generic_data_current.timezone = *pdata;
                    pdata++;
                    attr[loop].attr_type = ALI_ATTR_TYPE_TIMEZONE_SETTING;
                    attr[loop].param_len = 1;
                    attr[loop].attr_param = &generic_data_current.timezone;
                    break;
                case ALI_ATTR_TYPE_REMOVE_TIMING:
                    pdata += sizeof(ali_attr_type_t);
                    remove_timer(*pdata);
                    pdata++;
                    attr[loop].attr_type = ALI_ATTR_TYPE_REMOVE_TIMING;
                    attr[loop].param_len = 0;
                    break;
                default:
                    error[loop].attr_type = attr_type;
                    error[loop].error_code = ALI_ERROR_CODE_NOT_SUPPORTED_ATTR;
                    attr[loop].attr_type = ALI_ATTR_TYPE_ERROR;
                    attr[loop].param_len = sizeof(ali_attr_error_t);
                    attr[loop].attr_param = (uint8_t *)&error[loop];
                    loop++;
                    printi("ali_server_receive: unsupported 0x%04x", attr_type);
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
            ali_attr_conf_t *pmsg = (ali_attr_conf_t *)pbuffer;
            for (int i = 0; i < 3; i++)
            {
                if (pmsg->tid == msg_report_info.arr_ali_attr[i].tid)
                {
                    msg_report_info.arr_ali_attr[i].cnt = 0;
                    plt_timer_delete(msg_report_info.msg_report_timer[i], 0);
                }
            }
        }
        break;
    case MESH_MSG_ALI_TRANSPARENT_MSG:
        break;
    default:
        ret = FALSE;
        break;
    }
    return ret;
}

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
ali_attr_t dcd_tim_set(uint8_t *pdata, ali_attr_set_t *pmsg)
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
    pdata += sizeof(ali_attr_type_t);
    index = *pdata;
    pdata++;
    for (int dig = 0; dig < 4; dig++)
    {
        unix_time = unix_time + *(pdata) * factor;
        factor = (factor << 8);
        pdata++;
    }
    unix_time = (unix_time / 60) * 60;
    model_id = (*pdata) + *(pdata + 1) * 0x0100;
    pdata = pdata + 2;
    state = *pdata;
    timer_data->index = index;
    timer_data->unix_time = unix_time;
    timer_data->on_off = state;
    uint8_t error_code = is_in_list(temp_buf);
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
    attr.param_len = 8;
    attr.attr_param = (uint8_t *)pmsg->attr_type + sizeof(ali_attr_type_t);
    DBG_DIRECT("ali_server_receive: set time to %x, model is %x, on_off states = %d", unix_time,
               model_id, state);
    return attr;
}
/**
 * @brief period timer set
 * @param pdata  addr of received, start from the next byte of opcode
 * @param pmsg  ali attribute data struct
 * @note this routine hasn't been completed by Ali yet.
*/
ali_attr_t dcd_prd_set(uint8_t *pdata, ali_attr_set_t *pmsg)
{
    ali_attr_t attr;
    uint16_t factor = 0x01;
    prd_state = 0;
    uint16_t model_id;
    prd_unix_time = 0;
    pdata += sizeof(ali_attr_type_t) + 1;
    for (int dig = 0; dig < 2; dig++)
    {
        prd_unix_time = prd_unix_time + *(pdata) * factor;
        factor = (factor << 8);
        pdata++;
    }
//    uint8_t opnum = (uint8_t)((prd_unix_time&0xF000)>>12);
    prd_unix_time = prd_unix_time & 0x0fff;
    prd_schedule =  *pdata;
    pdata++;
    model_id = (*pdata) + *(pdata + 1) * 0x0100;
    pdata = pdata + 2;
    prd_state = *pdata;
    attr.attr_type = ALI_ATTR_TYPE_PERIODIC_SETTING;
    attr.param_len = 7;
    attr.attr_param = (uint8_t *)pmsg->attr_type + sizeof(ali_attr_type_t);
    DBG_DIRECT("ali_server_receive: set period to %x on %x, model is %x, on_off states = %d",
               prd_unix_time,
               prd_schedule, model_id, prd_state);
    prd_exist = true;
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

    ali_attr_msg(&ali_server, light_ctl_sub_addr[0], 0, MESH_MSG_ALI_ATTR_REQUEST,
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
    uint8_t val[2] = {0x11, num};
    attr[0].attr_param = val;
    ali_attr_msg(&ali_server, light_ctl_sub_addr[0], 0, MESH_MSG_ALI_ATTR_IND,
                 temp_tid, attr, 1);
    temp_tid++;
    if (temp_tid == 192)
    {
        temp_tid = 0x80;
    }
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

static void msg_report_cb_1(void *ptimer)
{
    DBG_DIRECT("power on msg_report_cb_1!");
    ali_attr_t attr[1];
    memcpy(attr, &msg_report_info.arr_ali_attr[0], sizeof(arr_ali_attr_t));
    if (msg_report_info.arr_ali_attr[0].cnt > 0)
    {
        msg_report_info.arr_ali_attr[0].cnt--;

        ali_attr_msg(&ali_server, MESH_ALI_SUB_ADDR, 0, msg_report_info.arr_ali_attr[0].opcode,
                     msg_report_info.arr_ali_attr[0].tid, attr, 1);

        if (msg_report_info.arr_ali_attr[0].cnt == 0)
        {
            //memset(&(msg_report_info.arr_ali_attr[2]),0,sizeof(arr_ali_attr_t));
            plt_timer_delete(msg_report_info.msg_report_timer[0], 0);
        }
        else
        {
            plt_timer_reset(msg_report_info.msg_report_timer[0], 0);
        }
    }
}
static void msg_report_cb_2(void *ptimer)
{
    ali_attr_t attr[1];
    memcpy(attr, &msg_report_info.arr_ali_attr[1], sizeof(arr_ali_attr_t));
    if (msg_report_info.arr_ali_attr[1].cnt > 0)
    {
        msg_report_info.arr_ali_attr[1].cnt--;

        ali_attr_msg(&ali_server, MESH_ALI_SUB_ADDR, 0, msg_report_info.arr_ali_attr[1].opcode,
                     msg_report_info.arr_ali_attr[1].tid, attr, 1);

        if (msg_report_info.arr_ali_attr[1].cnt == 0)
        {
            //memset(&(msg_report_info.arr_ali_attr[2]),0,sizeof(arr_ali_attr_t));
            plt_timer_delete(msg_report_info.msg_report_timer[1], 0);
        }
        else
        {
            plt_timer_start(ptimer, 0);
        }
    }
}
static void msg_report_cb_3(void *ptimer)
{
    ali_attr_t attr[1];
    memcpy(attr, &msg_report_info.arr_ali_attr[2], sizeof(arr_ali_attr_t));
    if (msg_report_info.arr_ali_attr[2].cnt > 0)
    {
        msg_report_info.arr_ali_attr[2].cnt--;

        ali_attr_msg(&ali_server, MESH_ALI_SUB_ADDR, 0, msg_report_info.arr_ali_attr[2].opcode,
                     msg_report_info.arr_ali_attr[2].tid, attr, 1);

        if (msg_report_info.arr_ali_attr[2].cnt == 0)
        {
            //memset(&(msg_report_info.arr_ali_attr[2]),0,sizeof(arr_ali_attr_t));
            plt_timer_delete(msg_report_info.msg_report_timer[2], 0);
        }
        else
        {
            plt_timer_start(ptimer, 0);
        }
    }
}

static void power_on_message_cb(void *ptimer)
{
    APP_PRINT_INFO0("power on send message report!");
    //DBG_DIRECT("power on send message report!");
    message_report(0x03); // power on event
    plt_timer_delete(power_on_msg_timer, 0);
}

/**
 * @brief report timer timeout message to host
 * @param num  index of timer
 * @note this routine hasn't been completed by Ali yet.
*/
void message_info_set()
{
    memset(msg_report_info.arr_ali_attr, 0, sizeof(msg_report_info));
    msg_report_info.report_cb[0] = msg_report_cb_1;
    msg_report_info.report_cb[1] = msg_report_cb_2;
    msg_report_info.report_cb[2] = msg_report_cb_3;
}

uint8_t val[3] = {0};
void power_on_message()
{
    uint32_t rantime;
    message_info_set();
    rantime = platform_random(10000);
    if (rantime < 10)
    {
        rantime = 10;
    }
    else
    {
        rantime = (rantime / 10) * 10;
    }
    //DBG_DIRECT("power on message%d!",rantime);
    power_on_msg_timer = plt_timer_create("power_on_msg", rantime, false, 0, power_on_message_cb);
    plt_timer_start(power_on_msg_timer, 0);
}
void message_report(uint8_t event)
{
    ali_attr_t attr[1];
    attr[0].attr_type = ALI_ATTR_TYPE_EVENT;
    attr[0].param_len = 1;
    //uint8_t val[1] = {event};
    //attr[0].attr_param = val;
    for (int i = 0; i < 3; i++)
    {
        if (msg_report_info.arr_ali_attr[i].cnt == 0)
        {
            val[i] = event;
            attr[0].attr_param = &val[i];
            memcpy(&(msg_report_info.arr_ali_attr[i].ali_attr), attr, sizeof(ali_attr_t));
            msg_report_info.arr_ali_attr[i].cnt = 3;
            msg_report_info.arr_ali_attr[i].opcode = MESH_MSG_ALI_ATTR_IND;
            msg_report_info.arr_ali_attr[i].tid = temp_tid;
            if (msg_report_info.msg_report_timer[i] == NULL)
            {
//                  DBG_DIRECT("plt_timer_create msg report!");
                msg_report_info.msg_report_timer[i] = plt_timer_create("msg report", MSG_RPT_TIME_OUT, false, 0,
                                                                       msg_report_info.report_cb[i]);
            }
            plt_timer_start(msg_report_info.msg_report_timer[i], 0);
            break;
        }
        if (i == 2)
        {
            APP_PRINT_INFO0("no capabilities to send message report!");
            return;
        }
    }
    ali_attr_msg(&ali_server, MESH_ALI_SUB_ADDR, 0, MESH_MSG_ALI_ATTR_IND,
                 temp_tid, attr, 1);
    temp_tid++;
    if (temp_tid == 192)
    {
        temp_tid = 0x80;
    }
}

void light_ctl_server_models_init(uint8_t element_index)
{
    /* binding models */
    light_lightness_server.pmodel_bound = &generic_on_off_server;
    light_ctl_server.pmodel_bound = &light_lightness_server;
    light_scene_server.pmodel_bound = &light_ctl_server;
    ali_server.pmodel_bound = &light_scene_server;

    /* register light ctl models */
    generic_on_off_server.model_data_cb = generic_on_off_server_data; //generic on/off 0x1000
    generic_on_off_server_reg(element_index, &generic_on_off_server);

    light_lightness_server.model_data_cb = light_lightness_server_data;//lightness server  0x1300
    light_lightness_server_reg(element_index, &light_lightness_server);

    light_ctl_server.model_data_cb = light_ctl_server_data;   //light CTL server 0x1303
    light_ctl_server_reg(element_index, &light_ctl_server);

    light_scene_server.model_data_cb = scene_server_data;
    scene_server_reg(element_index, &light_scene_server);
    scene_server_set_storage_memory(&light_scene_server, scene_storage_memory,
                                    sizeof(scene_storage_memory) / sizeof(scene_storage_memory_t));

    ali_server.model_receive = ali_server_receive;
    ali_model_reg(element_index, &ali_server, TRUE);
}


void light_ctl_server_models_sub(void)
{
    for (uint8_t loop = 0; loop < sizeof(light_ctl_sub_addr) / sizeof(uint16_t); loop++)
    {
        mesh_model_sub(generic_on_off_server.pmodel, light_ctl_sub_addr[loop]);
        mesh_model_sub(light_lightness_server.pmodel, light_ctl_sub_addr[loop]);
        mesh_model_sub(light_ctl_server.pmodel, light_ctl_sub_addr[loop]);
        mesh_model_sub(light_scene_server.pmodel, light_ctl_sub_addr[loop]);
        mesh_model_sub(ali_server.pmodel, light_ctl_sub_addr[loop]);
    }
}
#if 0
void handle_light_pub_evt(void)
{
    generic_on_off_server_get_t get_data = {GENERIC_OFF};
    generic_on_off_server_data(&generic_on_off_server, GENERIC_ON_OFF_SERVER_GET, &get_data);
    mesh_msg_send_cause_t ret = generic_on_off_publish(&generic_on_off_server, get_data.on_off);
    APP_PRINT_INFO1("PUB Status is %x", get_data.on_off);
}
#endif

void light_ctl_server_models_pub_start()
{
    mesh_model_pub_start(generic_on_off_server.pmodel);
}
void light_ctl_server_models_pub(void)
{
    mesh_model_pub_params_t pub_params;
    pub_params.pub_addr = MESH_ALI_SUB_ADDR;
    pub_params.pub_key_info.app_key_index = 0;
    pub_params.pub_key_info.frnd_flag = 0;
    pub_params.pub_ttl = 10;
    pub_params.pub_period.resol = 0;    // 10 sec
    pub_params.pub_period.steps = 18;  //18sec = 3min
    mesh_model_pub_params_set(generic_on_off_server.pmodel, pub_params);
    mesh_flash_store(MESH_FLASH_PARAMS_MODEL_PUB_PARAMS, generic_on_off_server.pmodel);
}
