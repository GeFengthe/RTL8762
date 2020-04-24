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


mesh_model_info_t datatrans_server;    // vendor model



typedef void (*Vendor_handle_rx_cb)(uint8_t opcode, uint8_t len, uint8_t *data);
static Vendor_handle_rx_cb    VendorHandleRx_cb = NULL;


static int32_t datatrans_server_data(const mesh_model_info_p pmodel_info,
                                     uint32_t opcode, void *pargs)
{
	uint8_t opcode_1B=0;
	
    UNUSED(pmodel_info);
	
				datatrans_server_write_t *pdata = pargs;
	data_uart_debug("datatrans_server_data opcode %d, %d \n", opcode, pdata->data_len );
	
	opcode_1B = ((opcode>>16)&0xff); // qlj 如果不行 利用type字段传进来
	// application call back
	if(VendorHandleRx_cb){
		VendorHandleRx_cb(opcode_1B, pdata->data_len, pdata->data);
	}
	
//    switch (type)
//    {
//		case DATATRANS_SERVER_WRITE:{
//				// datatrans_server_write_t *pdata = pargs;
//				data_uart_debug("remote write %d bytes: ", pdata->data_len);
//				data_uart_dump(pdata->data, pdata->data_len);
//				
//				uint8_t opcode_1B =  pdata->data[0]; // qlj 如果不行 利用type字段传进来
//				// application call back
//				if(VendorHandleRx_cb){
//					VendorHandleRx_cb(opcode_1B, pdata->data_len, pdata->data);
//				}
//			}
//			break;
//		case DATATRANS_SERVER_READ:
//			break;
//		default:
//			break;
//    }

    return 0;
}

void datatrans_server_model_init(void *rx_cb)
{
    /* register data transmission server model */
    datatrans_server.model_data_cb = datatrans_server_data;
    datatrans_server_reg(0, &datatrans_server);
	
	if(rx_cb){
		VendorHandleRx_cb = (Vendor_handle_rx_cb)rx_cb;
	}
	
}
