
#include <string.h>
#include "rtl876x_wdg.h"
#include "soft_wdt.h"
#include "data_uart.h"
#include "trace.h"

// (char *fmt, ...)   DBG_DIRECT
#define SOFTWDT_PRINTF  DBG_DIRECT

#define SOFTWDT_FEED_INTERVAL   (50)       // ms
#define SOFTWDT_START_DELAY     (20000 / SOFTWDT_FEED_INTERVAL)   // 20000ms



//������Ź���ʱ������
static SOFT_WATCH_DOG_TIMER SoftWatchDogTimerList[MAX_SWDT_ID];
static uint8_t StopWDTFedMake = true;
static uint8_t DisableWDTFedMake = false;


static uint32_t MsToOSTicks(uint16_t  ms)
{
	return ms / SOFTWDT_FEED_INTERVAL ;   // ��2ms��ʱ��Ϊ��׼��
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

	WDG_Peripheral_Init();// ��ʼ�����Ź�
}

/*
**************************************************************************
* �������ƣ� SoftWdtInit
* ���������� ������Ź��ĳ�ʼ��
* ��������� 
* ��������� 
* �� �� ֵ��
* ����˵����
* �޸�����        �汾��     �޸���	      �޸�����
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
	   osTick = MsToOSTicks(TimerTop);//��mSʱ�任���ʱ�ӽ���
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
* �������ƣ� SuspenWdt
* ���������� ����������Ź���
* ��������� 
* ��������� 
* �� �� ֵ��
* ����˵����
* �޸�����        �汾��     �޸���	      �޸�����
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
* �������ƣ� ��ʹ�� ʹ��  ϵͳ���Ź�
* ���������� ֹͣ(��) ϵͳ���Ź��������е���ʱ����ι��
* ��������� 
* ��������� 
* �� �� ֵ��
* ����˵���� ��ʹ��ʱ��Ҫ ���ֱ��ι��������Ḵλ��  ����ʱ�д����
* �޸�����        �汾��     �޸���	      �޸�����
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
* �������ƣ� SoftWdtISR
* ���������� ÿ��ִ��������Ź�����watchDogTimeֵ��һ����,��watchDogTimeֵ
* Ϊ0����ʾ������Ź��������Ҫ��λϵͳ��
* ��������� 
* ��������� 
* �� �� ֵ��
* ����˵���� SoftWdtISR����ֻ��״̬Ϊ���е�������Ź���ʱ�����м��
* �޸�����        �汾��     �޸���	      �޸�����
* -----------------------------------------------
* 2020/01/13	  V1.0.0	     qlj         XXXX
***************************************************************************
*/
void SoftWdtISR(void)
{
	uint8_t i = 0;
	static uint16_t Init_Tim_Count = 0,Init_Is_Over = 0;

	if(DisableWDTFedMake == true || StopWDTFedMake == true)       { 
		// ������ֱ���������̡�
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
				StopWDTFedMake = true;    // ���Ź���ʱ��ֹͣιӲ��
				return;
			}
		}
	}       

	if((Init_Tim_Count < SOFTWDT_START_DELAY) || (Init_Is_Over == 1)) {  // ��ʱ����ʱ���ø�����ע��
		WDG_Restart(); 
	} else {
		for(i=0; i<MAX_SWDT_ID; i++) {
			if(SoftWatchDogTimerList[i].watchDogState == SWDT_STAT_IDLE) {
				// ����Ƿ��б��������Ź�����ʼ���ˡ�
				SOFTWDT_PRINTF("\r\n [SoftWdtISR] InitSoftWdt TimeOut, Wdtid is %d !!! \r\n", i );
				StopWDTFedMake = true;
				return;
			}
		}
		Init_Is_Over = 1;	//-���е������ʱ���Ѿ�����
		WDG_Restart(); 
	}
}

/*
**************************************************************************
* �������ƣ� SoftWdtFed
* ���������� ������Ź�ιʳ
* ��������� 
* ��������� 
* �� �� ֵ��
* ����˵���� ����ID�ҵ���Ӧ��������Ź���Ȼ��ѿ��Ź���ʱ����ֵ�ָ���ֵ
* �޸�����        �汾��     �޸���	      �޸�����
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
//-��ζ�ʱ��ѯ������Ź���ι������� UC/OS-IIϵͳʱ����Ĺ��Ӻ����Ǹ������ѡ������ٸ����������ʵ�֡�
void OSTimeTickHook (void)
{
    SoftWdtISR();         //��ʱ��ѯÿһ��������Ź�ι��״����
}

/*
**************************************************************************
* �������ƣ� KeyBoardTask
* ���������� ������Ź���ʹ������
* ��������� 
* ��������� 
* �� �� ֵ��
* ����˵���� 
* �޸�����        �汾��     �޸���	      �޸�����
* -----------------------------------------------
* 2020/01/13	  V1.0.0	     qlj         XXXX
***************************************************************************
*/
Void KeyBoardTask(void *pdata){
        //����ʼ�ĵط�,��ʼ��������Ź�
    SoftWdtInit(KEY_TASK_SWDT_ID,2000);

    While(1){
        const DEV_FUN* Device = NULL;
        HAL_ERR_CODE err;
        uint8 key_value[2] = {0};
        uint8 data_addr = 0;
       
        SoftWdtFed(KEY_TASK_SWDT_ID);        //������ѭ����ι��
        //����ҪԤʾ����һƪ�ĵ������ݣ��������ʵ��һ��Ӳ������㡣
        Device = DeviceOpen(I2C0_ID,&err);       

        ����������������������..//���´���ʡ�ԡ�
        DeviceClose(&Device);
    }
}
#endif
