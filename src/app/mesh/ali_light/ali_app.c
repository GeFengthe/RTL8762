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
#include "ali_model.h"
#include "ali_app.h"
#include "light_cwrgb_app.h"
#include "light_storage_app.h"

static const uint16_t sub_addr[] = {0xC000, 0xCFFF};

/* light models */
static mesh_model_info_t ali_server;

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
                attr[loop].attr_type = pmsg->attr_type[loop];
                switch (pmsg->attr_type[loop])
                {
                case ALI_ATTR_TYPE_HSL_COLOR:
                    {
                        attr[loop].param_len = 6;

                        light_hsl_t hsl = light_get_hsl();
                        attr[loop].attr_param = (uint8_t *)&hsl;
                        break;
                    }
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
            ali_attr_t attr[2];
            ali_attr_error_t error[2];
            uint8_t *pdata = (uint8_t *)pmsg->attr_type;
            while (pdata < pbuffer + pmesh_msg->msg_len && loop < sizeof(attr) / sizeof(ali_attr_t))
            {
                ali_attr_type_t attr_type = LE_EXTRN2WORD(pdata);
                attr[loop].attr_type = attr_type;
                attr[loop].attr_param = pdata;
                pdata += sizeof(ali_attr_type_t);
                switch (attr_type)
                {
                case ALI_ATTR_TYPE_HSL_COLOR:
                    {
                        /* provide the length of the attribute */
                        attr[loop].param_len = 6;
                        /* process the message */
                        light_hsl_t hsl;
                        hsl.lightness = LE_EXTRN2WORD(pdata);
                        hsl.hue = LE_EXTRN2WORD(pdata + 2);
                        hsl.saturation = LE_EXTRN2WORD(pdata + 4);
                        light_cw_turn_off();
                        light_set_hsl(hsl);
                        light_state_store();
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
                pdata += attr[loop].param_len;
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

void ali_server_models_init(uint8_t element_index)
{
    /* register fan models */
    ali_server.model_receive = ali_server_receive;
    ali_model_reg(element_index, &ali_server, TRUE);
}

void ali_server_models_sub(void)
{
    for (uint8_t loop = 0; loop < sizeof(sub_addr) / sizeof(uint16_t); loop++)
    {
        mesh_model_sub_all(sub_addr[loop]);
    }
}
