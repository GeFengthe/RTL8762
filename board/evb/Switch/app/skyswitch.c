

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <math.h>
#include "platform_os.h"
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "skyswitch.h"
#include "trace.h"

#include "data_uart.h"
#include "app_skyiot_dlps.h"
#include "app_skyiot_server.h"

// #define APP_DBG_PRINTF(fmt, ...)
#define APP_DBG_PRINTF   DBG_DIRECT


#if (SKY_SWITCH_TYPE == SKY_NIGHTLIGHT_TYPE)
#define SWITCH1_GPIO                P4_0                            //key 1引脚
#define SWITCH1_GPIO_PIN            GPIO_GetPin(SWITCH1_GPIO)
#define SWITCH_ALM_GPIO             P4_2                            //KEY2开关
#define SWITCH_ALM_GPIO_PIN         GPIO_GetPin(SWITCH_ALM_GPIO)
//static plt_timer_t skyalmstatue_timer =NULL;

static uint8_t SwitchIO[SKYSWITC_NUMBERS]={SWITCH1_GPIO,SWITCH_ALM_GPIO};

#endif



#define MINPRESSTIME_2S				(40)	
#define MIDPRESSTIME_5S   			(85)  // 50ms定时器调用
#define MAXPRESSTIME_8S             (200)

#define MINALM_1S                   1000
#define MAXALM_2S                   2000

#define ALM_STATE_0         0
#define ALM_STATE_1         1
#define ALM_STATE_2         2
#define ALM_STATE_3         3

#define ALM_NO              0
#define ALM_CLICK           1
#define ALM_DOUBLE          2
#define ALM_LONG            3

typedef enum{
	SCAN_KEY_INIT = 0x00,
	SCAN_KEY_PRESS,    
	SCAN_KEY_PRESS_CONFIRM,    
	SCAN_KEY_RELEASE,
}SCAN_KEY_STATUS_e;



static int32_t presstime=0;
static SCAN_KEY_STATUS_e keystatus = SCAN_KEY_INIT;
static KEY_PRESS_MODE_e  keymode   = KEY_MODE_INIT;

static SkySwitchManager *mSwitchManager=NULL;
uint8_t door_flag =0;



/*
** GPIO
*/
static void HAL_GpioForSwitch_Init(void)
{
	/* Configure pad and pinmux firstly! */
	Pad_Config(SwitchIO[SKYSWITC1_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pinmux_Config(SwitchIO[SKYSWITC1_ENUM], DWGPIO);	

    
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);	
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(SwitchIO[SKYSWITC1_ENUM])|GPIO_GetPin(SwitchIO[SKYSWITC_ALM_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    
    /* Configure pad and pinmux firstly! */
    Pad_Config(SwitchIO[SKYSWITC_ALM_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pinmux_Config(SwitchIO[SKYSWITC_ALM_ENUM], DWGPIO);	

//    
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
    GPIO_StructInit(&GPIO_InitStruct);	
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(SwitchIO[SKYSWITC_ALM_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
}

void HAL_SwitchKey_Dlps_Control(bool isenter)
{
	if(isenter){
		Pad_Config(SWITCH1_GPIO, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
		System_WakeUpPinEnable(SWITCH1_GPIO, PAD_WAKEUP_POL_LOW, 0);
        Pad_Config(SWITCH_ALM_GPIO,PAD_SW_MODE,PAD_IS_PWRON,PAD_PULL_NONE,PAD_OUT_DISABLE,PAD_OUT_HIGH);
        System_WakeUpPinEnable(SWITCH_ALM_GPIO,PAD_WAKEUP_POL_LOW,0);
	}else{
		Pad_Config(SWITCH1_GPIO, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
        Pad_Config(SWITCH_ALM_GPIO,PAD_PINMUX_MODE,PAD_IS_PWRON,PAD_PULL_NONE,PAD_OUT_DISABLE,PAD_OUT_HIGH);
        Pad_ClearWakeupINTPendingBit(SWITCH1_GPIO);
        System_WakeUpPinDisable(SWITCH1_GPIO);
        Pad_ClearWakeupINTPendingBit(SWITCH_ALM_GPIO);
            System_WakeUpPinDisable(SWITCH_ALM_GPIO);
			switch_io_ctrl_dlps(false);			
        }	
}
uint8_t ReadalmStatu(void)
{
    uint8_t almval=0;
    if(GPIO_ReadInputDataBit(SWITCH_ALM_GPIO_PIN) == 0)
    {
        almval |= (1<<0);
        DBG_DIRECT("---switch =%d---\r\n",almval);
        SkyBleMesh_lightctrl_ON();
    }else{
        SkyBleMesh_lightctrl_OFF();
    }
    
    return almval;  
}

static uint8_t alm_driver(void)
{
    static uint8_t alm_state_buffer1 = ALM_STATE_0;
    static uint8_t alm_timer_cnt1 = 0;
    uint8_t alm_return = ALM_NO;
    uint8_t alm;
    alm = ReadalmStatu();
    switch(alm_state_buffer1)
    {
        case ALM_STATE_0:
            if(alm == 1)
            {
                alm_state_buffer1 = ALM_STATE_1;
            }
            break;
        case ALM_STATE_1:
            if(alm == 1)
            {
                alm_timer_cnt1 = 0;
                alm_state_buffer1 = ALM_STATE_2;
            }else
            {
                alm_state_buffer1 = ALM_STATE_0;  //防抖动
            }
            break;
        case ALM_STATE_2:
            if(alm ==0)
            {
                alm_return = ALM_CLICK;
                alm_state_buffer1 = ALM_STATE_0;
            }else if(++alm_timer_cnt1 >= 40)
            {
                alm_return = ALM_LONG;
                alm_state_buffer1 = ALM_STATE_3;
            }
            break;
        case ALM_STATE_3:
            if(alm == 0)
            {
                alm_state_buffer1 = ALM_STATE_0;
            }
            break;
    }
    return alm_return;
}

uint8_t alm_read(void)
{
    static uint8_t alm_state_buffer2 = ALM_STATE_0;
    static uint8_t alm_timer_cnt2 = 0;
    uint8_t alm_return = ALM_NO;
    uint8_t alm;
    
    alm = alm_driver();
    switch(alm_state_buffer2)
    {
        case ALM_STATE_0:
            if(alm == ALM_CLICK)
            {
                alm_timer_cnt2 = 0;
                alm_state_buffer2 = ALM_STATE_1;
            }else
            {
                alm_return = alm;
            }
            break;
        case ALM_STATE_1:
            if( alm == ALM_CLICK)
            {
                alm_return = ALM_DOUBLE;
                alm_state_buffer2 = ALM_STATE_0;
            }else if(++alm_timer_cnt2 >=20)
            {
                alm_return = ALM_CLICK;
                alm_state_buffer2 = ALM_STATE_0;
            }
            
            break;
    }
    return alm_return;
}


// 入网函数
 uint8_t ReadKeyStatu(void)
{
	uint8_t keyval=0 ;
	
	if(GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC1_ENUM]))==0){
		keyval |= (1<<0);
	}
	return keyval; 
}


//50ms 主定时器判断
void Scan_Keyboard_Time(void)
{
    if(ReadKeyStatu() == 1 && mSwitchManager->keymode !=KEY_LONGPRESS_MODE)
    {
        mSwitchManager->keyval ++;
    }else
    {
        mSwitchManager->keyval =0;
    }
    if(mSwitchManager->keyval > KEY_LONG_MODE)
    {
        mSwitchManager->keymode = KEY_LONGPRESS_MODE;
    }
    
}


static void Scan_Keyboard_Function(void)
{
	 // 50ms定时器调用
	uint8_t keypress=0;
	static uint8_t oldkeypress=0;
	
	if(mSwitchManager == NULL){
		return;
	}
	
	keypress = ReadKeyStatu();
	switch(keystatus)
	{
		case SCAN_KEY_INIT:{		
			if( keypress ) {
				presstime = 0;
				keystatus = SCAN_KEY_PRESS;
				oldkeypress = keypress;
			}
			break;
		}
		case SCAN_KEY_PRESS:{			
			if( keypress && (oldkeypress==keypress)) {
				keystatus = SCAN_KEY_PRESS_CONFIRM;
			} else {
				keystatus = SCAN_KEY_INIT;  // shake
			}
			break;
		}
		case SCAN_KEY_PRESS_CONFIRM:{
			if( keypress ) {	
				if(oldkeypress==keypress){
					if( presstime < MAXPRESSTIME_8S ) {
						presstime++;						
						if(presstime == MAXPRESSTIME_8S) {
							// Finish long press					
							// keystatus = SCAN_KEY_INIT; // wait for release
							APP_DBG_PRINTF("%s keyval:%02X mode:%d\r\n",__func__, oldkeypress, keymode);
						}
					} 
				}else{
					keystatus = SCAN_KEY_INIT; 
				}
			} else {
				keystatus = SCAN_KEY_RELEASE;
			}
			break;
		}
		case SCAN_KEY_RELEASE:{
			keystatus = SCAN_KEY_INIT;
			
			if(presstime < MINPRESSTIME_2S){
				// Finish short press
				keymode = KEY_SINGLE_MODE;
				
				mSwitchManager->keyval  = oldkeypress;
				mSwitchManager->keymode = keymode;
				APP_DBG_PRINTF("%s keyval:%02X mode:%d\r\n",__func__, oldkeypress, keymode);
			}else if(presstime >= MINPRESSTIME_2S && presstime < MIDPRESSTIME_5S){
                keymode = KEY_SHORTPRESS_MODE;		
                
                mSwitchManager->keyval  = oldkeypress;
                mSwitchManager->keymode = keymode;
                APP_DBG_PRINTF("%s keyval:%02X mode:%d\r\n",__func__, oldkeypress, keymode);
            }else if(presstime >= MIDPRESSTIME_5S && presstime < MAXPRESSTIME_8S){
                keymode = KEY_LONGPRESS_MODE;		
                
				mSwitchManager->keyval  = oldkeypress;
				mSwitchManager->keymode = keymode;
				APP_DBG_PRINTF("%s keyval:%02X mode:%d\r\n",__func__, oldkeypress, keymode);
			} else{
				presstime = 0;
			}
				
			
			keymode = KEY_MODE_INIT;
			break;
		}
		
		default:{
			keystatus = SCAN_KEY_INIT;
			break;
		}	
	}
}

static bool IsSwitchInited = false;
void HAL_Switch_HandleTimer(void *timer)
{
	if(IsSwitchInited == false || SkyBleMesh_Batt_Station() != BATT_NORMAL){
		return;
	}
		
	// 20ms定时器调用
	Scan_Keyboard_Function();	

}

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
		if(IsSwitchInited==true && keystatus==SCAN_KEY_INIT && keypress==0){ // 按键释放
			return true;
		} 
	}  
	
	return false;
}

