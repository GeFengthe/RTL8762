/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     datatrans_server_app.c
  * @brief    Source file for data transmission server model.
  * @details  Data types and external functions declaration.
  * @author   hector_huang
  * @date     2018-10-29
  * @version  v1.0
  * *************************************************************************************
  */
#include "datatrans_server_app.h"
#include "datatrans_model.h"


mesh_model_info_t 	VendorModel_Server;    // vendor model



typedef void (*Vendor_handle_rx_cb)(uint8_t opcode, uint8_t len, uint8_t *data);
static Vendor_handle_rx_cb    VendorHandleRx_cb = NULL;


static int32_t vendor_model_server_data(const mesh_model_info_p pmodel_info,
                                     uint32_t type, void *pargs)
{
	uint8_t opcode_1B=0;
	
    UNUSED(pmodel_info);
	
	Receive_meshdata_t *pdata = pargs;
//	data_uart_debug("vendor_model_server_data opcode %08X, %d \n", pdata->opcode_3B, pdata->data_len );
	
	opcode_1B = ((pdata->opcode_3B>>16)&0xff); 
	// application call back
	if(VendorHandleRx_cb){
		VendorHandleRx_cb(opcode_1B, pdata->data_len, pdata->data);
	}

    return 0;
}

void Vendor_Model_Init(void *rx_cb)
{
    /* register data transmission server model */
    VendorModel_Server.model_data_cb = vendor_model_server_data;
    datatrans_server_reg(0, &VendorModel_Server);
	
	if(rx_cb){
		VendorHandleRx_cb = (Vendor_handle_rx_cb)rx_cb;
	}
	
}
