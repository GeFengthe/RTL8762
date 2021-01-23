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

static plt_timer_t skyblewakeup_timer=NULL;
static uint8_t dlpsstatu = 0; // 1:ready 2:enter 3:exit other:invalid

uint32_t etime=0;


static void SkyBleMesh_Wakeup_Timeout_cb(void *timer)
{
 
    DBG_DIRECT("------Waketime --------\r\n");
//	APP_DBG_PRINTF(" %s %d",__func__, dlpsstatu);
	if(dlpsstatu == 1){
		// 没能进DLPS，恢复关闭的tmr等
		SkyBleMesh_ExitDlps_cfg(false);
	}
}

static void SkyBleMesh_StopWakeup_tmr(void)
{	
	if(skyblewakeup_timer){
		plt_timer_delete(skyblewakeup_timer, 0);
		skyblewakeup_timer = NULL;
	}
}
static void SkyBleMesh_StartWakeup_tmr(void)
{	
	if(skyblewakeup_timer == NULL){		
		skyblewakeup_timer = plt_timer_create("WAKEUP", 1000*10, true, 0, SkyBleMesh_Wakeup_Timeout_cb);
		if (skyblewakeup_timer != NULL){
			plt_timer_start(skyblewakeup_timer, 0);
		}
	}
}


void SkyBleMesh_EnterDlps_TmrCnt_Handle(void)
{
    if(attrdlps == 0)
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
    DBG_DIRECT("----DLPS-dlpsstatu=%d",dlpsstatu);
}

void SkyBleMesh_EnterDlps_cfg(void)
{	
	// APP_DBG_PRINTF(" SkyBleMesh_EnterDlps_cfg");
//    SkyBleMesh_StopWakeup_tmr();
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
	if(norexit == true){ // 正常退出DLPS

		HAL_SwitchKey_Dlps_Control(false);
		// light
		SkyBleMesh_DlpsLight_Handle(false);
        HAL_Skymag_Dlps_Control(false);
	}
	if(SkyBleMesh_IsProvision_Sate() && SkyBleMesh_Batt_Station() == BATT_NORMAL){ // provisioned且电量正常
        beacon_start(); // 配网才会打开，这个要验证下，未配网不广播

	}	
	
	// sw timer
	SkyBleMesh_StartMainLoop_tmr();
	SkyBleMesh_EnterDlps_timer();
//	Reenter_tmr_ctrl_dlps(false);
//	SkyBleMesh_StopWakeup_tmr();
}
void Sky_alive_dlps(void)
{
    static uint32_t alive_cnt = 0;
    if(DlpsCtrlStatu_t.bit.alive == 1)
    {
        alive_cnt ++;
        if(alive_cnt == 60)
        {
            door_alive_ctrl_dlps(true);
            alive_cnt =0;
        }
    }else{
        alive_cnt =0;
    }
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



