
#include <string.h>
#include "rtl876x_wdg.h"
#include "soft_wdt.h"
#include "data_uart.h"
#include "trace.h"

// (char *fmt, ...)   DBG_DIRECT
#define SOFTWDT_PRINTF  DBG_DIRECT

#define SOFTWDT_FEED_INTERVAL   (50)       // ms
#define SOFTWDT_START_DELAY     (20000 / SOFTWDT_FEED_INTERVAL)   // 20000ms



//软件看门狗定时器数组
static SOFT_WATCH_DOG_TIMER SoftWatchDogTimerList[MAX_SWDT_ID];
static uint8_t StopWDTFedMake = true;
static uint8_t DisableWDTFedMake = false;


static uint32_t MsToOSTicks(uint16_t  ms)
{
	return ms / SOFTWDT_FEED_INTERVAL ;   // 以2ms定时器为基准。
}


void WDG_Peripheral_Init(void)
{
    WDG_ClockEnable();
    WDG_Config(150, 11, RESET_ALL);  
    WDG_Enable();
    //Add application code here
}

void OS_WDTInit(void)
{
	uint8_t i=0;
	for(i=0; i < MAX_SWDT_ID ; i++){
		SoftWatchDogTimerList[i].watchDogTimeOut = 0;
		SoftWatchDogTimerList[i].watchDogTime    = 0; 
		SoftWatchDogTimerList[i].watchDogState   = SWDT_STAT_IDLE;
	}
	
	StopWDTFedMake = false;

	WDG_Peripheral_Init();// 初始化看门狗
}

/*
**************************************************************************
* 函数名称： SoftWdtInit
* 功能描述： 软件看门狗的初始化
* 输入参数： 
* 输出参数： 
* 返 回 值：
* 其它说明：
* 修改日期        版本号     修改人	      修改内容
* -----------------------------------------------
* 2020/01/13	  V1.0.0	     qlj         XXXX
***************************************************************************
*/
bool SoftWdtInit(SWDT_ID SwdtId, uint16_t TimerTop)
{

	uint32_t osTick = 0;

	if(SwdtId >= MAX_SWDT_ID){
		return false;
	}
	
	if(SoftWatchDogTimerList[SwdtId].watchDogState == SWDT_STAT_IDLE){
	   osTick = MsToOSTicks(TimerTop);//将mS时间换算成时钟节拍
	   SoftWatchDogTimerList[SwdtId].watchDogTimeOut = osTick;
	   SoftWatchDogTimerList[SwdtId].watchDogTime    = osTick;
	   SoftWatchDogTimerList[SwdtId].watchDogState   = SWDT_STAT_RUN;

	   return true;
	}else{
	   return false;
	}
}
#if 0
/*
**************************************************************************
* 函数名称： SuspenWdt
* 功能描述： 挂起软件看门狗。
* 输入参数： 
* 输出参数： 
* 返 回 值：
* 其它说明：
* 修改日期        版本号     修改人	      修改内容
* -----------------------------------------------
* 2020/01/13	  V1.0.0	     qlj         XXXX
***************************************************************************
*/
void SuspenWdt(SWDT_ID SwdtId)
{	
	if(SwdtId >= MAX_SWDT_ID){
        return;
    }

    SoftWatchDogTimerList[SwdtId].watchDogState = SWDT_STAT_SUSPEN;
}

void RunWdt(SWDT_ID SwdtId)
{
	if(SwdtId >= MAX_SWDT_ID){
        return;
    }
		
    SoftWatchDogTimerList[SwdtId].watchDogState = SWDT_STAT_RUN;
}
#endif
/*
**************************************************************************
* 函数名称： 不使能 使能  系统看门狗
* 功能描述： 停止(打开) 系统看门狗，不进行倒计时，不喂狗
* 输入参数： 
* 输出参数： 
* 返 回 值：
* 其它说明： 不使能时需要 软件直接喂狗，否则会复位，  休眠时有此情况
* 修改日期        版本号     修改人	      修改内容
* -----------------------------------------------
* 2020/01/13	  V1.0.0	     qlj         XXXX
***************************************************************************
*/
void DisableSoftWdt( void )
{
	DisableWDTFedMake = true;
}
void EnableSoftWdt( void )
{
	DisableWDTFedMake = false;
}

/*
**************************************************************************
* 函数名称： SoftWdtISR
* 功能描述： 每次执行软件看门狗计数watchDogTime值减一操作,当watchDogTime值
* 为0，表示软件看门狗溢出，需要复位系统。
* 输入参数： 
* 输出参数： 
* 返 回 值：
* 其它说明： SoftWdtISR函数只对状态为运行的软件看门狗定时器进行检查
* 修改日期        版本号     修改人	      修改内容
* -----------------------------------------------
* 2020/01/13	  V1.0.0	     qlj         XXXX
***************************************************************************
*/
void SoftWdtISR(void)
{
	uint8_t i = 0;
	static uint16_t Init_Tim_Count = 0,Init_Is_Over = 0;

	if(DisableWDTFedMake == true || StopWDTFedMake == true)       { 
		// 这里是直接重启进程。
//		WDG_ClockEnable();
//		WDG_Enable();
		DBG_DIRECT("[SoftWdtISR] reboot!");
		WDG_SystemReset(RESET_ALL, RESET_REASON_WDG_TIMEOUT); 
		
		return;        
	}
	if(Init_Is_Over == 0) {
		Init_Tim_Count++;
	}
	for(i=0; i<MAX_SWDT_ID; i++) {
		if(SoftWatchDogTimerList[i].watchDogState == SWDT_STAT_RUN) {
			if(--(SoftWatchDogTimerList[i].watchDogTime) == 0) { 
				SOFTWDT_PRINTF("\r\n [SoftWdtISR] FeedSoftWdt TimeOut, Wdtid is %d !!! \r\n", i );
				StopWDTFedMake = true;    // 软看门狗超时，停止喂硬狗
				return;
			}
		}
	}       

	if((Init_Tim_Count < SOFTWDT_START_DELAY) || (Init_Is_Over == 1)) {  // 延时，给时间让各任务注册
		WDG_Restart(); 
	} else {
		for(i=0; i<MAX_SWDT_ID; i++) {
			if(SoftWatchDogTimerList[i].watchDogState == SWDT_STAT_IDLE) {
				// 检查是否列表所以软看门狗都初始化了。
				SOFTWDT_PRINTF("\r\n [SoftWdtISR] InitSoftWdt TimeOut, Wdtid is %d !!! \r\n", i );
				StopWDTFedMake = true;
				return;
			}
		}
		Init_Is_Over = 1;	//-所有的软件定时器已经启动
		WDG_Restart(); 
	}
}

/*
**************************************************************************
* 函数名称： SoftWdtFed
* 功能描述： 软件看门狗喂食
* 输入参数： 
* 输出参数： 
* 返 回 值：
* 其它说明： 根据ID找到相应的软件看门狗，然后把看门狗定时器数值恢复初值
* 修改日期        版本号     修改人	      修改内容
* -----------------------------------------------
* 2020/01/13	  V1.0.0	     qlj         XXXX
***************************************************************************
*/
void SoftWdtFed(SWDT_ID SwdtId)
{

	if(SwdtId >= MAX_SWDT_ID){
		return;
	}
	
	SoftWatchDogTimerList[SwdtId].watchDogTime = SoftWatchDogTimerList[SwdtId].watchDogTimeOut;
}




#if 0
//-如何定时查询软件看门狗的喂狗情况？ UC/OS-II系统时间节拍钩子函数是个不错的选择。随后再给出具有体的实现。
void OSTimeTickHook (void)
{
    SoftWdtISR();         //定时查询每一个软件看门狗喂狗状况。
}

/*
**************************************************************************
* 函数名称： KeyBoardTask
* 功能描述： 软件看门狗的使用例子
* 输入参数： 
* 输出参数： 
* 返 回 值：
* 其它说明： 
* 修改日期        版本号     修改人	      修改内容
* -----------------------------------------------
* 2020/01/13	  V1.0.0	     qlj         XXXX
***************************************************************************
*/
Void KeyBoardTask(void *pdata){
        //任务开始的地方,初始化软件看门狗
    SoftWdtInit(KEY_TASK_SWDT_ID,2000);

    While(1){
        const DEV_FUN* Device = NULL;
        HAL_ERR_CODE err;
        uint8 key_value[2] = {0};
        uint8 data_addr = 0;
       
        SoftWdtFed(KEY_TASK_SWDT_ID);        //任务主循环中喂狗
        //这里要预示我下一篇文档的内容，关于如何实现一个硬件抽象层。
        Device = DeviceOpen(I2C0_ID,&err);       

        ……………………………..//以下代码省略。
        DeviceClose(&Device);
    }
}
#endif
