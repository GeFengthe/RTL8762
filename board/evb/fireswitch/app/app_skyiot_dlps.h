
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
        uint32_t sw1tmr:  1; // switch relay timer1
        uint32_t sw2tmr:  1; // switch relay timer2
        uint32_t sw3tmr:  1; // switch relay timer3
        uint32_t rsvd: 24;
    } bit;
} DLPS_CTRL_STATU_T;
extern DLPS_CTRL_STATU_T DlpsCtrlStatu_t;



extern void SkyBleMesh_EnterDlps_TmrCnt_Handle(void);
extern void SkyBleMesh_EnterDlps_timer(void);
extern bool switch_check_dlps_statu(void);

extern void switch_io_ctrl_dlps(bool allowenter);
extern void blemesh_sysinit_ctrl_dlps(bool allowenter);
extern void blemesh_unprov_ctrl_dlps(bool allowenter);
extern void blemesh_report_ctrl_dlps(bool allowenter);
extern void Reenter_tmr_ctrl_dlps(bool allowenter);
extern void Switch_Relay1_tmr_ctrl_dlps(bool allowenter);
extern void Switch_Relay2_tmr_ctrl_dlps(bool allowenter);
extern void Switch_Relay3_tmr_ctrl_dlps(bool allowenter);


extern void SkyBleMesh_ReadyEnterDlps_cfg(void);
extern void SkyBleMesh_EnterDlps_cfg(void);
extern void SkyBleMesh_ExitDlps_cfg(void);


#endif // 

