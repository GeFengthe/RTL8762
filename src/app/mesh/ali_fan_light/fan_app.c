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
#include "fan_app.h"
#include "generic_on_off.h"
#include "light_lightness.h"
#include "light_ctl.h"
#include "scene.h"
#include "ali_model.h"

static const uint16_t fan_sub_addr[] = {0xC007, 0xCFFF};

typedef struct
{
    uint8_t level; //!< 0~100, or 150~156
    uint8_t direction; //!< 0: positive; 1: negative
} fan_data_t;

static fan_data_t fan_data_current;
/* The last fan state may be restored from the nvm */
static fan_data_t fan_data_last = {50, 0};

static fan_data_t scene_normal = {80, 0};
static fan_data_t scene_sleep = {20, 1};
static fan_data_t scene_confort = {50, 0};

static scene_storage_memory_t scene_storage_memory[] =
{
    {ALI_SCENE_NUM_WIND_NORMAL, &scene_normal},
    {ALI_SCENE_NUM_WIND_SLEEP, &scene_sleep},
    {ALI_SCENE_NUM_WIND_COMFORT, &scene_confort}
};

static uint16_t current_scene;

/* fan models */
static mesh_model_info_t generic_on_off_server;
static mesh_model_info_t scene_server;
static mesh_model_info_t scene_setup_server;
static mesh_model_info_t ali_server;

static int32_t generic_on_off_server_data(const mesh_model_info_p pmodel_info, uint32_t type,
                                          void *pargs)
{
    generic_on_off_t current_on_off = GENERIC_OFF;
    if (fan_data_current.level)
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
                        fan_data_current = fan_data_last;
                        /* TODO: turn on the fan here! */
                    }
                    else
                    {
                        fan_data_last = fan_data_current;
                        fan_data_current.level = 0;
                        /* TODO: turn off the fan here! */
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
                    fan_data_t *pstate = pdata->pmemory;
                    fan_data_current = *pstate;
                    /* TODO: set the fan state here! */
                    /* TODO: the fan may publish the fan state here! */
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
            uint8_t attr_num = (pmesh_msg->msg_len - MEMBER_OFFSET(ali_attr_get_t,
                                                                   attr_type)) / sizeof(ali_attr_type_t);
            ali_attr_t *attr = plt_malloc(sizeof(ali_attr_t) * attr_num, RAM_TYPE_DATA_OFF);
            ali_attr_error_t *error = plt_malloc(sizeof(ali_attr_error_t) * attr_num, RAM_TYPE_DATA_OFF);
            for (uint8_t loop = 0; loop < attr_num; loop++)
            {
                switch (pmsg->attr_type[loop])
                {
                case ALI_ATTR_TYPE_LEVEL:
                    attr[loop].attr_type = ALI_ATTR_TYPE_LEVEL;
                    attr[loop].param_len = 1;
                    attr[loop].attr_param = (uint8_t *)&fan_data_current.level;
                    break;
                case ALI_ATTR_TYPE_FAN_DIRECTION:
                    attr[loop].attr_type = ALI_ATTR_TYPE_FAN_DIRECTION;
                    attr[loop].param_len = 1;
                    attr[loop].attr_param = (uint8_t *)&fan_data_current.direction;
                    break;
                case ALI_ATTR_TYPE_SCENE_MODE:
                    attr[loop].attr_type = ALI_ATTR_TYPE_SCENE_MODE;
                    attr[loop].param_len = 2;
                    attr[loop].attr_param = (uint8_t *)&current_scene;
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
                case ALI_ATTR_TYPE_LEVEL:
                    fan_data_current.level = *(pdata + sizeof(ali_attr_type_t));
                    pdata += sizeof(ali_attr_type_t) + 1;
                    attr[loop].attr_type = ALI_ATTR_TYPE_LEVEL;
                    attr[loop].param_len = 1;
                    attr[loop].attr_param = (uint8_t *)&fan_data_current.level;
                    /* TODO: set the level of fan here! */
                    printi("ali_server_receive: set level to %d", fan_data_current.level);
                    break;
                case ALI_ATTR_TYPE_FAN_DIRECTION:
                    fan_data_current.direction = *(pdata + sizeof(ali_attr_type_t));
                    pdata += sizeof(ali_attr_type_t) + 1;
                    attr[loop].attr_type = ALI_ATTR_TYPE_FAN_DIRECTION;
                    attr[loop].param_len = 1;
                    attr[loop].attr_param = (uint8_t *)&fan_data_current.direction;
                    /* TODO: set the direction of fan here! */
                    printi("ali_server_receive: set direction to %d", fan_data_current.direction);
                    break;
                case ALI_ATTR_TYPE_SCENE_MODE:
                    {
                        uint16_t scene = LE_EXTRN2WORD(pdata + sizeof(ali_attr_type_t));
                        pdata += sizeof(ali_attr_type_t) + 2;
                        uint16_t loop_scene;
                        for (loop_scene = 0; loop_scene < sizeof(scene_storage_memory) / sizeof(scene_storage_memory_t);
                             loop_scene++)
                        {
                            if (scene_storage_memory[loop_scene].scene_number == scene)
                            {
                                break;
                            }
                        }
                        if (loop_scene < sizeof(scene_storage_memory) / sizeof(scene_storage_memory_t))
                        {
                            current_scene = scene;
                            attr[loop].attr_type = ALI_ATTR_TYPE_SCENE_MODE;
                            attr[loop].param_len = 2;
                            attr[loop].attr_param = (uint8_t *)&current_scene;

                            fan_data_current = *(fan_data_t *)scene_storage_memory[loop_scene].pmemory;
                            /* TODO: set the fan state here! */
                            /* TODO: the fan may publish the fan state here! */
                            printi("ali_server_receive: set scene to %d", current_scene);
                        }
                        else
                        {
                            error[loop].attr_type = attr_type;
                            error[loop].error_code =
                                ALI_ERROR_CODE_NOT_FOUND_INDEX; //TODO: which error code if scene not supported?
                            attr[loop].attr_type = ALI_ATTR_TYPE_ERROR;
                            attr[loop].param_len = sizeof(ali_attr_error_t);
                            attr[loop].attr_param = (uint8_t *)&error[loop];
                        }
                    }
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

void fan_server_models_init(uint8_t element_index)
{
    /* binding models */
    scene_server.pmodel_bound = &generic_on_off_server;
    scene_setup_server.pmodel_bound = &scene_server;
    ali_server.pmodel_bound = &scene_setup_server;

    /* register fan models */
    generic_on_off_server.model_data_cb = generic_on_off_server_data;
    generic_on_off_server_reg(element_index, &generic_on_off_server);

    scene_server.model_data_cb = scene_server_data;
    scene_server_reg(element_index, &scene_server);
    scene_setup_server.model_data_cb = scene_server_data;
    scene_setup_server_reg(element_index, &scene_setup_server);
    scene_server_set_storage_memory(&scene_server, scene_storage_memory,
                                    sizeof(scene_storage_memory) / sizeof(scene_storage_memory_t));
    scene_setup_server_set_storage_memory(&scene_setup_server, scene_storage_memory,
                                          sizeof(scene_storage_memory) / sizeof(scene_storage_memory_t));

    ali_server.model_receive = ali_server_receive;
    ali_model_reg(element_index, &ali_server, TRUE);
}

void fan_server_models_sub(void)
{
    for (uint8_t loop = 0; loop < sizeof(fan_sub_addr) / sizeof(uint16_t); loop++)
    {
        mesh_model_sub(generic_on_off_server.pmodel, fan_sub_addr[loop]);
        mesh_model_sub(scene_server.pmodel, fan_sub_addr[loop]);
        mesh_model_sub(scene_setup_server.pmodel, fan_sub_addr[loop]);
        mesh_model_sub(ali_server.pmodel, fan_sub_addr[loop]);
    }
}
