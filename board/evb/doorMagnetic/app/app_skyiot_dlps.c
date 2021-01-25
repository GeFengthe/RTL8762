#include <string.h>
#include <stdlib.h>

#include "rtl876x_pinmux.h"
#include "rtl876x_io_dlps.h"
#include "rtl876x_gpio.h"
#include "platform_os.h"
#include "gap_scheduler.h"
#include "mesh_beacon.h"
#include "app_task.h"
#include "app_skyiot_dlps.h"
#include "app_skyiot_server.h"
#include "skyadc.h"
#include "trace.h"


// #define APP_DBG_PRINTF(fmt, ...)
#define APP_DBG_PRINTF   DBG_DIRECT

DLPS_CTRL_STATU_T DlpsCtrlStatu_t={(DLPS_JUST_SYSINIT_OK|DLPS_JUST_WAITING_TMR)};
static plt_timer_t skybleenterdlps_timer=NULL;

//static plt_timer_t skyblewakeup_timer=NULL;
static plt_timer_t skyblealmdlps_timer = NULL;
static uint8_t dlpsstatu = 0; // 1:ready 2:enter 3:exit other:invalid

uint8_t almdlps = 0;
uint8_t studlps = 0;



void SkyBleMesh_EnterDlps_TmrCnt_Handle(void)
{

	if(SkyBleMesh_Is_No_ReportMsg() == true){
		blemesh_report_ctrl_dlps(true);		
	}else{
		blemesh_report_ctrl_dlps(false);		
	}
	if(HAL_Switch_Is_Relese() == true){		
		switch_io_ctrl_dlps(true);
	}else{
		switch_io_ctrl_dlps(false);
	}
    DBG_DIRECT("DlpsCtrl=0x%x",DlpsCtrlStatu_t);
	if(DlpsCtrlStatu_t.dword == DLPS_JUST_WAITING_TMR){
		SkyBleMesh_ReadyEnterDlps_cfg();
		if(skybleenterdlps_timer){
			plt_timer_delete(skybleenterdlps_timer, 0);
			skybleenterdlps_timer = NULL;
		}
		Reenter_tmr_ctrl_dlps(true);    
	}

}

static void SkyBleMesh_EnterDlps_Timeout_cb(void *timer)
{
    static uint8_t startdelay = 100;
	if(startdelay){
		startdelay--; // when reset,check dlps after 5s 
		return;
	}
	
    T_IO_MSG msg;
    msg.type = IO_MSG_TYPE_TIMER;
    msg.subtype = ENTER_DLPS_TIMEOUT;
    app_send_msg_to_apptask(&msg);
}

void SkyBleMesh_EnterDlps_timer(void)
{	
	if(skybleenterdlps_timer == NULL){ 	
		skybleenterdlps_timer = plt_timer_create("dlps", 61, true, 0, SkyBleMesh_EnterDlps_Timeout_cb);     //进入低功耗定时器时间可能会影响是否会出现删除主定时器而未进低功耗模式(100)
		if (skybleenterdlps_timer != NULL){
			plt_timer_start(skybleenterdlps_timer, 0);
		}
	}
}

void SkyBleMesh_ReadyEnterDlps_cfg(void)
{	
	dlpsstatu = 1; // ready
    SkyBleMesh_StopMainLoop_tmr();
    if(almdlps ==1)
    {
        sky_almdlps_timer();
    }
//    DBG_DIRECT("----DLPS-dlpsstatu=%d",dlpsstatu);
}

void SkyBleMesh_EnterDlps_cfg(void)
{	

	dlpsstatu = 2; // enter
	
	// switch1
    HAL_Skymag_Dlps_Control(true);
	HAL_SwitchKey_Dlps_Control(true);
    
	// light 维持IO电平，视电路和单前状态标志而定，
	SkyBleMesh_DlpsLight_Handle(true);
}

void SkyBleMesh_ExitDlps_cfg(bool norexit)
{
	dlpsstatu = 3; // exit
	if(System_WakeUpInterruptValue(P4_2) == TRUE)      //是否是门磁唤醒判断
	{
		skyble_almctrl(true);
	}else
	{
		skyble_almctrl(false);
	}
    if(System_WakeUpInterruptValue(P5_0) == TRUE)
    {
        skyble_stuctrl(true);
    }else
    {
        skyble_stuctrl(false);
    }
	if(norexit == true){ // 正常退出DLPS

		HAL_SwitchKey_Dlps_Control(false);
		SkyBleMesh_DlpsLight_Handle(false);
        HAL_Skymag_Dlps_Control(false);
	}
	if(SkyBleMesh_IsProvision_Sate() && SkyBleMesh_Batt_Station() == BATT_NORMAL){ // provisioned且电量正常
        beacon_start(); // 配网才会打开，这个要验证下，未配网不广播

	}	
	
	// sw timer
	SkyBleMesh_StartMainLoop_tmr();
	SkyBleMesh_EnterDlps_timer();
}
void sky_almdlps_timer_cb(void * timer)
{
    DBG_DIRECT("----alm_timer--\r\n");
    SkyBleMesh_ExitDlps_cfg(true);
}
//门磁引脚定时器
void sky_almdlps_timer(void)
{
    if(skyblealmdlps_timer == NULL)
    {
        skyblealmdlps_timer = plt_timer_create("almdlps",2000,false,0,sky_almdlps_timer_cb);
    }
    plt_timer_start(skyblealmdlps_timer,0);
    
}

bool switch_check_dlps_statu(void)
{
    if (DlpsCtrlStatu_t.dword == 0){
        return true;
    }else{
		return false;
	}
}
void switch_io_ctrl_dlps(bool allowenter)
{
    if(allowenter){
        DlpsCtrlStatu_t.bit.io = 0;
    }else{
        DlpsCtrlStatu_t.bit.io = 1;
    }
}
void blemesh_sysinit_ctrl_dlps(bool allowenter)
{
    if(allowenter){
        DlpsCtrlStatu_t.bit.sysinit = 0;
    }else{
        DlpsCtrlStatu_t.bit.sysinit = 1;
    }
}
void blemesh_unprov_ctrl_dlps(bool allowenter)
{
    if(allowenter){
        DlpsCtrlStatu_t.bit.unprov = 0;
    }else{
        DlpsCtrlStatu_t.bit.unprov = 1;
    }
}
void blemesh_report_ctrl_dlps(bool allowenter)
{
    if(allowenter){
        DlpsCtrlStatu_t.bit.report = 0;
    }else{
        DlpsCtrlStatu_t.bit.report = 1;
    }
}
void Reenter_tmr_ctrl_dlps(bool allowenter)
{
    if(allowenter){
        DlpsCtrlStatu_t.bit.dlpstmr = 0;
    }else{
        DlpsCtrlStatu_t.bit.dlpstmr = 1;
    }
}

void Led_Relay_tmr_ctrl_dlps(bool allowenter)
{
    if(allowenter){
        DlpsCtrlStatu_t.bit.ledtmr = 0;
    }else{
        DlpsCtrlStatu_t.bit.ledtmr = 1;
    }
}

void gap_ctrl_dlps(bool allowenter)
{
    if(allowenter){
        DlpsCtrlStatu_t.bit.gap = 0;
    }else{
        DlpsCtrlStatu_t.bit.gap = 1;
    }
}
void blemesh_factory_ctrl_dlps(bool allowenter)
{
    if(allowenter){
        DlpsCtrlStatu_t.bit.factory = 0;
    }else{
        DlpsCtrlStatu_t.bit.factory = 1;
    }
}
void door_alive_ctrl_dlps(bool allowenter)
{
    if(allowenter){
        DlpsCtrlStatu_t.bit.alive = 0;
    }else{
        DlpsCtrlStatu_t.bit.alive = 1;
    }
}
void blemesh_key_dlps(bool allowenter)
{
    if(allowenter)
    {
        DlpsCtrlStatu_t.bit.key = 0;
    }else
    {
        DlpsCtrlStatu_t.bit.key = 1;
    }
}
//alm属性唤醒低功耗控制函数
void skyble_almctrl(bool allow)
{
	if(allow)
	{
		almdlps = 1;		
	}else
	{
		almdlps  = 0;
	}
    DBG_DIRECT("----almdlps=%d\r\n",almdlps );
}
//stu属性唤醒低功耗
void skyble_stuctrl(bool allow)
{
    if(allow)
    {
        studlps = 1;
    }else
    {
        studlps = 0;
    }
    DBG_DIRECT("----studlps=%d\r\n",studlps);
}


