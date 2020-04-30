/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
* @file     datatrans_model_server.c
* @brief    Source file for data transmission server model.
* @details  Data types and external functions declaration.
* @author   hector_huang
* @date     2018-10-29
* @version  v1.0
* *************************************************************************************
*/

/* Add Includes here */
#include "datatrans_model.h"


static mesh_msg_send_cause_t datatrans_server_send(const mesh_model_info_p pmodel_info,
                                                   uint16_t dst, uint16_t app_key_index,
                                                   uint8_t *pmsg, uint16_t msg_len)
{
    mesh_msg_t mesh_msg;
    mesh_msg.pmodel_info = pmodel_info;
    access_cfg(&mesh_msg);
    mesh_msg.pbuffer = pmsg;
    mesh_msg.msg_len = msg_len;
	
    mesh_msg.dst = dst;
    mesh_msg.app_key_index = app_key_index;
    // mesh_msg.delay_time = 10;
	
//	mesh_msg.ttl = 3;   // 指定下ttl 否则会是30次
//	data_uart_debug("datatrans_server_send %d \r\n", mesh_msg.ttl);
	
    return access_send(&mesh_msg);
}

static mesh_msg_send_cause_t datatrans_send_data(const mesh_model_info_p pmodel_info,
                                                 uint16_t dst, uint16_t app_key_index,
                                                 uint16_t data_len, uint8_t *data)
{
    mesh_msg_send_cause_t ret;
    uint8_t *pmsg = NULL;
    uint16_t msg_len = data_len ;  // OPCODE_3B + usedata
	
    pmsg = plt_malloc(msg_len, RAM_TYPE_DATA_ON);
    if (NULL == pmsg) {
        return MESH_MSG_SEND_CAUSE_NO_MEMORY;
    }
	
    memcpy(pmsg, data, msg_len);
    ret = datatrans_server_send(pmodel_info, dst, app_key_index, pmsg, msg_len);
	
    plt_free(pmsg, RAM_TYPE_DATA_ON);

    return ret;
}

mesh_msg_send_cause_t datatrans_publish(const mesh_model_info_p pmodel_info,
                                        uint16_t data_len, uint8_t *data)
{
	bool checkret=false;
    mesh_msg_send_cause_t ret = MESH_MSG_SEND_CAUSE_INVALID_DST;
	checkret = mesh_model_pub_check(pmodel_info);
//	data_uart_debug("mesh_model_pub_check %d \r\n", checkret);
    // if (checkret)
    {
		// mesh_node.app_key_list[appkey_index=0].app_key_index_g = 0x123.
        ret = datatrans_send_data(pmodel_info, 0x7fff, 0, data_len, data); 
    }

    return ret;
}

static bool datatrans_server_receive(mesh_msg_p pmesh_msg)
{
    bool ret = TRUE;
    uint8_t *pbuffer = pmesh_msg->pbuffer + pmesh_msg->msg_offset;
    mesh_model_info_p pmodel_info = pmesh_msg->pmodel_info;
	
	uint16_t tmp_cpyid = (BLEMESH_VENDOR_COMPANY_ID<<8) | (BLEMESH_VENDOR_COMPANY_ID>>8);	
		
	if( (pmesh_msg->access_opcode&0xFFFF) == tmp_cpyid ){
		
        datatrans_write_t *pmsg = (datatrans_write_t *)pbuffer;
		uint16_t data_len = pmesh_msg->msg_len - sizeof(datatrans_write_t);
		Receive_meshdata_t recedata = {pmesh_msg->access_opcode, data_len, NULL, DATATRANS_SUCCESS};
		if (NULL != pmodel_info->model_data_cb)
		{
			recedata.data = pmsg->data;
			pmodel_info->model_data_cb(pmodel_info, 0, &recedata);  // receive date from gateway
		}	
		
	} else {		
		ret = FALSE;
	}
	
    return ret;
}

bool datatrans_server_reg(uint8_t element_index, mesh_model_info_p pmodel_info)
{
    if (NULL == pmodel_info)
    {
        return FALSE;
    }

    pmodel_info->model_id = MESH_VENDOR_MODELID_SERVER;
    if (NULL == pmodel_info->model_receive)
    {
        pmodel_info->model_receive = datatrans_server_receive;
        if (NULL == pmodel_info->model_data_cb)
        {
            printw("datatrans server reg: missing model data process callback!");
        }
    }

    return mesh_model_reg(element_index, pmodel_info);
}

