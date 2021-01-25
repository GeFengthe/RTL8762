
#ifndef _APP_SKYIOT_DLPS_H
#define _APP_SKYIOT_DLPS_H

#include "skyswitch.h"

#define DLPS_JUST_SYSINIT_OK    (0x00000001)
#define DLPS_JUST_WAITING_TMR   (0x00000002)
typedef union
{
    uint32_t dword;
    struct
    {
        uint32_t sysinit: 1; // system init ok  
        uint32_t dlpstmr: 1; // enter dlps tmr cnt
        uint32_t io:      1; // switch
        uint32_t unprov:  1; // unprov or proved
        uint32_t report:  1; // report attr,FIFO\FLAG
        uint32_t ledtmr:  1; // led relay timer1
        uint32_t gap:     1; // inf
        uint32_t factory: 1; // PCB & device check
        uint32_t alive:   1;
        uint32_t key:     1;        //配网按键
        uint32_t rsvd:    23;        //24
    } bit;
} DLPS_CTRL_STATU_T;
extern DLPS_CTRL_STATU_T DlpsCtrlStatu_t;
extern uint8_t almdlps;
extern uint8_t studlps;



extern void SkyBleMesh_EnterDlps_TmrCnt_Handle(void);
extern void SkyBleMesh_EnterDlps_timer(void);
extern bool switch_check_dlps_statu(void);

extern void switch_io_ctrl_dlps(bool allowenter);
extern void blemesh_sysinit_ctrl_dlps(bool allowenter);
extern void blemesh_unprov_ctrl_dlps(bool allowenter);
extern void blemesh_report_ctrl_dlps(bool allowenter);
extern void Reenter_tmr_ctrl_dlps(bool allowenter);
extern void Led_Relay_tmr_ctrl_dlps(bool allowenter);
extern void gap_ctrl_dlps(bool allowenter);
extern void blemesh_factory_ctrl_dlps(bool allowenter);
extern void door_alive_ctrl_dlps(bool allowenter);
extern void door_edpls_ctrl_dlps(bool allowenter);


extern void SkyBleMesh_ReadyEnterDlps_cfg(void);
extern void SkyBleMesh_EnterDlps_cfg(void);
extern void SkyBleMesh_ExitDlps_cfg(bool norexit);
extern void Sky_alive_dlps(void);

extern void blemesh_key_dlps(bool allowenter);      //配网按键低功耗处理
extern void sky_almdlps_timer(void);
extern void skyble_stuctrl(bool allow);

#endif // 

