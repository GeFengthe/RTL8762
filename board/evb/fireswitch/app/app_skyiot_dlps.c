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


DLPS_CTRL_STATU_T DlpsCtrlStatu_t={(DLPS_JUST_SYSINIT_OK|DLPS_JUST_WAITING_TMR)};
static plt_timer_t skybleenterdlps_timer=NULL;

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
    app_send_dlpstmr_sem();
}

void SkyBleMesh_EnterDlps_timer(void)
{	
	if(skybleenterdlps_timer == NULL){ 	
		skybleenterdlps_timer = plt_timer_create("dlps", 50, true, 0, SkyBleMesh_EnterDlps_Timeout_cb);
		if (skybleenterdlps_timer != NULL){
			plt_timer_start(skybleenterdlps_timer, 0);
		}
	}
}

void SkyBleMesh_ReadyEnterDlps_cfg(void)
{	
	// ble 
	beacon_stop();  	
	if(SkyBleMesh_IsProvision_Sate() == false){ // unprov  未配网休眠不SCAN
		gap_sched_scan(false);   // gap layer scam
	}else{	
	}
		
	// sw timer
	SkyBleMesh_StopMainLoop_tmr();
	SkyBleMesh_StopScanSwitch_tmr();	
}

void SkyBleMesh_EnterDlps_cfg(void)
{	
	// debug uart
    Pad_ControlSelectValue(P3_0, PAD_SW_MODE);
    Pad_ControlSelectValue(P3_1, PAD_SW_MODE);
	
	// switch1
	Pad_Config(P2_3, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	System_WakeUpPinEnable(P2_3, PAD_WAKEUP_POL_LOW, 0);
	Pad_Config(P2_4, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	System_WakeUpPinEnable(P2_4, PAD_WAKEUP_POL_LOW, 0);
	Pad_Config(P2_2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	System_WakeUpPinEnable(P2_2, PAD_WAKEUP_POL_LOW, 0);	
	// light 维持IO电平，视电路和单前状态标志而定，如绿板LOW是亮灯。
	SkyBleMesh_DlpsLight_Handle(true);
}

void SkyBleMesh_ExitDlps_cfg(void)
{
	// debug uart
    Pad_ControlSelectValue(P3_0, PAD_PINMUX_MODE);
    Pad_ControlSelectValue(P3_1, PAD_PINMUX_MODE);
	
	// switch1
	Pad_Config(P2_3, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	if(System_WakeUpInterruptValue(P2_3) == 1){		// 也可读IO状态。 USE_GPIO_DLPS 须为1
        Pad_ClearWakeupINTPendingBit(P2_3);
		System_WakeUpPinDisable(P2_3);
		switch_io_ctrl_dlps(false);		
	}
	Pad_Config(P2_4, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	if(System_WakeUpInterruptValue(P2_4) == 1){		// 也可读IO状态。 USE_GPIO_DLPS 须为1
        Pad_ClearWakeupINTPendingBit(P2_4);
		System_WakeUpPinDisable(P2_4);
		switch_io_ctrl_dlps(false);		
	}
	Pad_Config(P2_2, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
	if(System_WakeUpInterruptValue(P2_2) == 1){		// 也可读IO状态。 USE_GPIO_DLPS 须为1
        Pad_ClearWakeupINTPendingBit(P2_2);
		System_WakeUpPinDisable(P2_2);
		switch_io_ctrl_dlps(false);		
	}
	// light
	SkyBleMesh_DlpsLight_Handle(false);
	
	// ble
	if(SkyBleMesh_IsProvision_Sate()){ // provisioned
		beacon_start(); // 配网才会打开，这个要验证下。 未配网不广播。
	}	
	
	// sw timer
	SkyBleMesh_StartMainLoop_tmr();
	SkyBleMesh_StartScanSwitch_tmr();
	
	SkyBleMesh_EnterDlps_timer();
	Reenter_tmr_ctrl_dlps(false);
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

void Switch_Relay1_tmr_ctrl_dlps(bool allowenter)
{
    if(allowenter){
        DlpsCtrlStatu_t.bit.sw1tmr = 0;
    }else{
        DlpsCtrlStatu_t.bit.sw1tmr = 1;
    }
}
void Switch_Relay2_tmr_ctrl_dlps(bool allowenter)
{
    if(allowenter){
        DlpsCtrlStatu_t.bit.sw2tmr = 0;
    }else{
        DlpsCtrlStatu_t.bit.sw2tmr = 1;
    }
}
void Switch_Relay3_tmr_ctrl_dlps(bool allowenter)
{
    if(allowenter){
        DlpsCtrlStatu_t.bit.sw3tmr = 0;
    }else{
        DlpsCtrlStatu_t.bit.sw3tmr = 1;
    }
}





