
#ifndef __SOFT_WDT_H__
#define __SOFT_WDT_H__

#include <stdbool.h>
#include <stdint.h>

#define USE_SOFT_WATCHDOG   0
#define HARD_WDT_TIMEROUT   (40000)    // 4000*250us = 1s
/*
 ���������������Ź�ID.
 ��������ӵ�ÿ��ID�������ʼ����ʹ��.�������ã��ڴ�ɾ��ID.
 */
typedef enum{
	APPTASK_THREAD_SWDT_ID = 0,     // apptask -->while(1)
	SKYMESH_THREAD_SWDT_ID,       // SkyBleMesh_MainLoop_timer
	MAX_SWDT_ID		//-���������������Ź�����
}SWDT_ID;


typedef enum{
	SWDT_STAT_IDLE = 0,                        
	SWDT_STAT_SUSPEN,                      
	SWDT_STAT_RUN                           
}SWDT_STAT;

/*
uint32_t watchDogTime; �������������Ź��Ķ�ʱ���ˣ�ÿ������SoftWdtISR��ʱ���ֵ
�ᱻ��ȥ1,����0��˵������ˡ�ι�����ǰѸ�ֵ��Ϊ��ֵ.

*/
typedef struct soft_wach_dog_timer{
	uint32_t watchDogTimeOut;                // ���Ź�������ʱ��ֵ
	uint32_t watchDogTime;                   // ���Ź���ʱ��(����ʱ)
	SWDT_STAT  watchDogState;                // ���Ź���ʱ��״̬(SWDT_STAT)
}SOFT_WATCH_DOG_TIMER;		


/******************************************/
/*               �ӿں���[����]           */
/******************************************/
extern void OS_WDTInit(void);
extern bool SoftWdtInit(SWDT_ID SwdtId, uint16_t TimerTop);
extern void SoftWdtISR(void);
extern void SoftWdtFed(SWDT_ID SwdtId);
extern void DisableSoftWdt( void );
extern void EnableSoftWdt( void );


#endif	/* __STM32_WORKER_PORT_H__ */
