#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <math.h>
#include "platform_os.h"
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_nvic.h"
#include "skyswitch.h"
#include "trace.h"
#include "app_task.h"

#include "data_uart.h"
#include "app_skyiot_dlps.h"
#include "app_skyiot_server.h"

// #define APP_DBG_PRINTF(fmt, ...)
#define APP_DBG_PRINTF   DBG_DIRECT


#if (SKY_SWITCH_TYPE == SKY_NIGHTLIGHT_TYPE)
#define SWITCH1_GPIO                P4_0                            //key 1引脚
#define SWITCH1_GPIO_PIN            GPIO_GetPin(SWITCH1_GPIO)
#define SWITCH_STU_GPIO             P5_0                            //防拆开关      P5_0
#define SWITCH_STU_GPIO_PIN         GPIO_GetPin(SWITCH_STU_GPIO)
#define SWITCH_ALM_GPIO             P4_2                            //门磁开关      //P3_2
#define SWITCH_ALM_GPIO_PIN         GPIO_GetPin(SWITCH_ALM_GPIO)

static uint8_t SwitchIO[SKYSWITC_NUMBERS]={SWITCH1_GPIO,SWITCH_STU_GPIO,SWITCH_ALM_GPIO};

#endif

#define KEY_DOWN                     1
#define KEY_UP                       0

#define MINPRESSTIME_2S				(40)	
#define MIDPRESSTIME_5S   			(90)  // 50ms定时器调用
//#define MAXPRESSTIME_8S             (200)
typedef enum{
	SCAN_KEY_INIT = 0x00,
	SCAN_KEY_PRESS,    
	SCAN_KEY_PRESS_CONFIRM,    
	SCAN_KEY_RELEASE,
}SCAN_KEY_STATUS_e;

//static int32_t presstime=0;
static SCAN_KEY_STATUS_e keystatus = SCAN_KEY_INIT;
//static KEY_PRESS_MODE_e  keymode   = KEY_MODE_INIT;

static SkySwitchManager *mSwitchManager=NULL;

//static plt_timer_t  skyble_switch_timer = NULL;     //按键定时器句柄



/*
** GPIO
*/
static void HAL_GpioForSwitch_Init(void)
{
    NVIC_InitTypeDef    NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GPIO28_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 6;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
	/* Configure pad and pinmux firstly! */
    
	Pad_Config(SwitchIO[SKYSWITC1_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pinmux_Config(SwitchIO[SKYSWITC1_ENUM], DWGPIO);	
    
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);	
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(SwitchIO[SKYSWITC1_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd  = ENABLE;
    GPIO_InitStruct.GPIO_ITTrigger = GPIO_INT_Trigger_EDGE;                 //
    GPIO_InitStruct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;         
    GPIO_InitStruct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
    GPIO_InitStruct.GPIO_DebounceTime = 10;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_INTConfig(SWITCH1_GPIO_PIN,ENABLE);
    GPIO_ClearINTPendingBit(SWITCH1_GPIO);
    

	
	/* Configure pad and pinmux firstly! */
	Pad_Config(SwitchIO[SKYSWITC_STU_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pinmux_Config(SwitchIO[SKYSWITC_STU_ENUM], DWGPIO);	
    
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_StructInit(&GPIO_InitStruct);	
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(SwitchIO[SKYSWITC_STU_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    
    	/* Configure pad and pinmux firstly! */
	Pad_Config(SwitchIO[SKYSWITC_ALM_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pinmux_Config(SwitchIO[SKYSWITC_ALM_ENUM], DWGPIO);	
    
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_StructInit(&GPIO_InitStruct);	
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(SwitchIO[SKYSWITC_ALM_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
}

void HAL_Skymag_Dlps_Control(bool isenter)
{
    if(isenter)
    {
//        DBG_DIRECT("---SKYMAG stu=%d,alm=%d",GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC_STU_ENUM])) == 1,GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC_ALM_ENUM])) == 1);
        if(almdlps ==0)
        {
            if(GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC_ALM_ENUM])) == 1)
            {
                Pad_Config(SWITCH_ALM_GPIO, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
                System_WakeUpPinEnable(SWITCH_ALM_GPIO,PAD_WAKEUP_POL_LOW,0);
            }else
            {
                Pad_Config(SWITCH_ALM_GPIO,PAD_SW_MODE,PAD_IS_PWRON,PAD_PULL_NONE,PAD_OUT_DISABLE,PAD_OUT_LOW);
                System_WakeUpPinEnable(SWITCH_ALM_GPIO,PAD_WAKEUP_POL_HIGH,0);
            }
        }else
        {
            Pad_Config(SWITCH_ALM_GPIO, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
        }
        if(studlps == 0)
        {
            if(GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC_STU_ENUM])) == 1)
            {
                Pad_Config(SWITCH_STU_GPIO, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
                System_WakeUpPinEnable(SWITCH_STU_GPIO,PAD_WAKEUP_POL_LOW,0);
            }else
            {
                Pad_Config(SWITCH_STU_GPIO,PAD_SW_MODE,PAD_IS_PWRON,PAD_PULL_NONE,PAD_OUT_DISABLE,PAD_OUT_HIGH);
                System_WakeUpPinEnable(SWITCH_STU_GPIO,PAD_WAKEUP_POL_HIGH,0);
            }
        }else
        {
            Pad_Config(SWITCH_STU_GPIO,PAD_SW_MODE,PAD_IS_PWRON,PAD_PULL_NONE,PAD_OUT_DISABLE,PAD_OUT_HIGH);
        }
        
    }else{
        Pad_Config(SWITCH_ALM_GPIO,PAD_PINMUX_MODE,PAD_IS_PWRON,PAD_PULL_NONE,PAD_OUT_DISABLE,PAD_OUT_LOW);
        Pad_Config(SWITCH_STU_GPIO,PAD_PINMUX_MODE,PAD_IS_PWRON,PAD_PULL_UP,PAD_OUT_DISABLE,PAD_OUT_HIGH);
        Pad_ClearWakeupINTPendingBit(SWITCH_ALM_GPIO);
        System_WakeUpPinDisable(SWITCH_ALM_GPIO);
        Pad_ClearWakeupINTPendingBit(SWITCH_STU_GPIO);
        System_WakeUpPinDisable(SWITCH_STU_GPIO);
    }
}

void HAL_SwitchKey_Dlps_Control(bool isenter)
{
	if(isenter){
		Pad_Config(SWITCH1_GPIO, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
		System_WakeUpPinEnable(SWITCH1_GPIO, PAD_WAKEUP_POL_LOW, 0);
	}else{
		Pad_Config(SWITCH1_GPIO, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
			Pad_ClearWakeupINTPendingBit(SWITCH1_GPIO);
			System_WakeUpPinDisable(SWITCH1_GPIO);
			switch_io_ctrl_dlps(false);		
	}	
}
//门磁开关与防拆
uint8_t ReadStatus(void)
{
    uint8_t skystatu=0;
    if(GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC_ALM_ENUM])) == 1)
    {
        skystatu |= (1<<0);
    }
    if(GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC_STU_ENUM])) == 1)
    {
        skystatu |= (1<<1);
    }
    return skystatu;

}

 uint8_t ReadKeyStatu(void)
{
	uint8_t keyval=0 ;
	
	if(GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC1_ENUM]))==0){
		keyval |= (1<<0);
	} 	
	return keyval; 
}



static bool IsSwitchInited = false;
//void HAL_Switch_HandleTimer(void *timer)
//{
//	if(IsSwitchInited == false || SkyBleMesh_Batt_Station() != BATT_NORMAL){
//		return;
//	}
//		
//	// 20ms定时器调用
//	Scan_Keyboard_Function();	

//}

bool HAL_Switch_Init(SkySwitchManager *manager)
{
	if( manager == NULL){
		return false;
	}
		
	mSwitchManager = manager;
	HAL_GpioForSwitch_Init();

	mSwitchManager->keyval  = 0;
	mSwitchManager->keymode = 0;

	IsSwitchInited = true;
	
	return true;	
}



bool HAL_Switch_Is_Relese(void)
{
	uint8_t keypress=0;
	if(mSwitchManager->keyval==0 && mSwitchManager->keymode==0) {  // 没有未处理键值
		
		keypress = ReadKeyStatu();
        DBG_DIRECT("-----keypress=%d---\r\n",keypress);
		if(IsSwitchInited==true && keystatus==SCAN_KEY_INIT && keypress==0){ // 按键释放
			return true;
		} 
	}  
	return false;
}
//void SkyBleSwitch_timer_cb(void *timer)
//{
//    T_IO_MSG msg;
//    msg.type = IO_MSG_TYPE_TIMER;
//    msg.subtype = SWITCH_TIMEOUT;
//    app_send_msg_to_apptask(&msg);
//}

//void SkyBleSwitch_timer(void)
//{
//    if(skyble_switch_timer ==NULL)
//    {
//        skyble_switch_timer = plt_timer_create("switch",30,true,0,SkyBleSwitch_timer_cb);
//    }
//    if(skyble_switch_timer !=NULL)
//    {
//        plt_timer_start(skyble_switch_timer,0);
//    }
//    
//}

//按键中断
void GPIO28_Handler(void)
{
    if(GPIO_GetINTStatus(SWITCH1_GPIO_PIN) !=RESET)
    {
        if(GPIO_ReadInputDataBit(SWITCH1_GPIO_PIN) ==0)
        {
            mSwitchManager->keyval =KEY_DOWN;
            blemesh_key_dlps(false);
        }
       GPIO_ClearINTPendingBit(SWITCH1_GPIO_PIN);
    }
    
}

void Scan_Keyboard_Function(void)
{
    uint8_t keypress = 0;
    static uint16_t keycnt =0;
    if(mSwitchManager->keyval == KEY_DOWN)
    {
        keypress = ReadKeyStatu();
        if(keypress == KEY_DOWN)
        {
            keycnt ++;
            if((keycnt >= MIDPRESSTIME_5S)&&(mSwitchManager->keymode !=KEY_LONGPRESS_MODE))
            {
                mSwitchManager->keymode = KEY_LONGPRESS_MODE;
                keycnt = 0;
            }
        }else{
            mSwitchManager->keyval = KEY_UP;
            keycnt = 0;
            blemesh_key_dlps(true);
        }
    }
}


