
#ifndef __SOFT_WDT_H__
#define __SOFT_WDT_H__

#include <stdbool.h>
#include <stdint.h>

#define USE_SOFT_WATCHDOG   0
#define HARD_WDT_TIMEROUT   (40000)    // 4000*250us = 1s
/*
 往这里添加软件看门狗ID.
 在这里添加的每个ID都必须初始化并使用.如若不用，在此删处ID.
 */
typedef enum{
	APPTASK_THREAD_SWDT_ID = 0,     // apptask -->while(1)
	SKYMESH_THREAD_SWDT_ID,       // SkyBleMesh_MainLoop_timer
	MAX_SWDT_ID		//-决定了最大软件看门狗数量
}SWDT_ID;


typedef enum{
	SWDT_STAT_IDLE = 0,                        
	SWDT_STAT_SUSPEN,                      
	SWDT_STAT_RUN                           
}SWDT_STAT;

/*
uint32_t watchDogTime; 这个就是软件看门狗的定时器了，每当调用SoftWdtISR的时候该值
会被减去1,减到0了说明溢出了。喂狗就是把该值赋为初值.

*/
typedef struct soft_wach_dog_timer{
	uint32_t watchDogTimeOut;                // 看门狗计数超时初值
	uint32_t watchDogTime;                   // 看门狗定时器(倒计时)
	SWDT_STAT  watchDogState;                // 看门狗定时器状态(SWDT_STAT)
}SOFT_WATCH_DOG_TIMER;		


/******************************************/
/*               接口函数[声明]           */
/******************************************/
extern void OS_WDTInit(void);
extern bool SoftWdtInit(SWDT_ID SwdtId, uint16_t TimerTop);
extern void SoftWdtISR(void);
extern void SoftWdtFed(SWDT_ID SwdtId);
extern void DisableSoftWdt( void );
extern void EnableSoftWdt( void );


#endif	/* __STM32_WORKER_PORT_H__ */
