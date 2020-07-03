/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      app_task.c
   * @brief     Routines to create App task and handle events & messages
   * @author    jane
   * @date      2017-06-02
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <os_msg.h>
#include <os_task.h>
#include <gap.h>
#include <gap_le.h>
#include <gap_msg.h>
#include <trace.h>
#include <app_task.h>
#include <app_msg.h>
#include <rtl876x.h>
#include <rtl876x_nvic.h>
#include <rtl876x_uart.h>
#include <rtl876x_rcc.h>
#include <rtl876x_pinmux.h>

#include "mesh_api.h"
#include "device_app.h"
#include "data_uart.h"
#include "user_cmd_parse.h"
#include "device_cmd.h"

#include "app_skyiot_server.h"
#include "app_skyiot_dlps.h"
#include "soft_wdt.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define EVENT_MESH                    0x80
#define APP_TASK_PRIORITY             1         //!< Task priorities
#define APP_TASK_STACK_SIZE           256 * 10  //!< Task stack size
#define MAX_NUMBER_OF_GAP_MESSAGE     0x20      //!< GAP message queue size
#define MAX_NUMBER_OF_IO_MESSAGE      0x20      //!< IO message queue size
#define MAX_NUMBER_OF_EVENT_MESSAGE   (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE + MESH_INNER_MSG_NUM) //!< Event message queue size

/*============================================================================*
 *                              Variables
 *============================================================================*/
void *app_task_handle;   //!< APP Task handle
void *evt_queue_handle;  //!< Event queue handle
void *io_queue_handle;   //!< IO queue handle

void *skyswitch_sem_handle=NULL;   //!< skyiot scan switch sem handle
void *skydlpstmr_sem_handle=NULL;    //!< skyiot dlps tmr sem handle

/*============================================================================*
 *                              Functions
 *============================================================================*/
void app_main_task(void *p_param);

bool app_send_msg_to_apptask(T_IO_MSG *p_msg)
{
    uint8_t event = EVENT_IO_TO_APP;

    if (os_msg_send(io_queue_handle, p_msg, 0) == false)
    {
        APP_PRINT_ERROR0("send_io_msg_to_app fail");
        return false;
    }
    if (os_msg_send(evt_queue_handle, &event, 0) == false)
    {
        APP_PRINT_ERROR0("send_evt_msg_to_app fail");
        return false;
    }
    return true;
}
void app_send_switch_sem(void)
{
	if(skyswitch_sem_handle){
		os_sem_give(skyswitch_sem_handle);
	}
}
void app_send_dlpstmr_sem(void)
{
	if(skydlpstmr_sem_handle){
		os_sem_give(skydlpstmr_sem_handle);
	}
}

void app_send_uart_msg(uint8_t data)
{
    uint8_t event = EVENT_IO_TO_APP;
    T_IO_MSG msg;
    msg.type = IO_MSG_TYPE_UART;
    msg.subtype = data;
    if (os_msg_send(io_queue_handle, &msg, 0) == false)
    {
    }
    else if (os_msg_send(evt_queue_handle, &event, 0) == false)
    {
    }
}

/**
 * @brief  Initialize App task
 * @return void
 */
void app_task_init()
{
    os_task_create(&app_task_handle, "app", app_main_task, 0, APP_TASK_STACK_SIZE,
                   APP_TASK_PRIORITY);
}

void uart_init(void)
{
    data_uart_init(P3_0, P3_1, app_send_uart_msg);
}

/**
 * @brief        App task to handle events & messages
 * @param[in]    p_param    Parameters sending to the task
 * @return       void
 */
void app_main_task(void *p_param)
{
	uint32_t tmpi=0;
    uint8_t event;

	#if USE_SOFT_WATCHDOG
	SoftWdtInit(APPTASK_THREAD_SWDT_ID, 3000);    // config softwdt as 3000ms 
	#endif
	
    os_msg_queue_create(&io_queue_handle, MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
    os_msg_queue_create(&evt_queue_handle, MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint8_t));
    gap_start_bt_stack(evt_queue_handle, io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);

    mesh_start(EVENT_MESH, EVENT_IO_TO_APP, evt_queue_handle, io_queue_handle);
    uart_init();
    user_cmd_init("MeshDevice");
	
	os_sem_create(&skyswitch_sem_handle, 0, 1);
	os_sem_create(&skydlpstmr_sem_handle, 0, 1);
	
	SkyBleMesh_MainLoop_timer();	
	SkyBleMesh_EnterDlps_timer();
	Reenter_tmr_ctrl_dlps(false);
    while (true)
    {
		#if USE_SOFT_WATCHDOG
		SoftWdtFed(APPTASK_THREAD_SWDT_ID); 
		#endif
		
        if (os_msg_recv(evt_queue_handle, &event, 0xFFFFFFFF) == true)
        {
            if (event == EVENT_IO_TO_APP)
            {
                T_IO_MSG io_msg;
                if (os_msg_recv(io_queue_handle, &io_msg, 0) == true)
                {
                    app_handle_io_msg(io_msg);
                }
            }
            else if (event == EVENT_MESH)
            {
                mesh_inner_msg_handle(event);
            }
            else
            {
                gap_handle_msg(event);
            }
        }
		
		if (os_sem_take(skyswitch_sem_handle, 0) == true){			
			HAL_Switch_HandleTimer(NULL);
		}
		if (os_sem_take(skydlpstmr_sem_handle, 0) == true){			
			SkyBleMesh_EnterDlps_TmrCnt_Handle();
		}
				
		if(++tmpi >= 5000){
			DBG_DIRECT("app_main_task %d %d \r\n", mesh_node_state_restore(), DlpsCtrlStatu_t.dword);
			tmpi = 0;
		}
				
		#if MESH_TEST_PRESSURE == 1
		SkyBleMesh_Test_Timeout_cb(NULL);
		
		if(test_flag==1){
			test_update_attr();
			test_flag = 0;
		}
		#endif
    }
}

void test_dlps_function(bool enter)
{ 
	DBG_DIRECT("test_dlps_function %d \n",enter);	
	
	if(enter){	
		blemesh_sysinit_ctrl_dlps(true);
	}else{	
	
		uint16_t scan_interval = 400;  //!< 250ms
		uint16_t scan_window   = 0x30; //!< 30ms
		gap_sched_params_set(GAP_SCHED_PARAMS_SCAN_INTERVAL, &scan_interval, sizeof(scan_interval));
		gap_sched_params_set(GAP_SCHED_PARAMS_SCAN_WINDOW, &scan_window, sizeof(scan_window));
		
		blemesh_sysinit_ctrl_dlps(false);
	}
}
