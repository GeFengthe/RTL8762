

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <math.h>
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "skyswitch.h"
#include "trace.h"


// #define APP_DBG_PRINTF(fmt, ...)
#define APP_DBG_PRINTF   DBG_DIRECT

#define LED_RED    LEDTURNOFF 
#define LED_BLUE   LEDTURNON  
#define LED_BLINK  LEDBLINK  


#define SWITCH1_GPIO             P2_4
#define SWITCH1_GPIO_PIN         GPIO_GetPin(SWITCH1_GPIO)
#define SWITCH2_GPIO             P1_2
#define SWITCH2_GPIO_PIN         GPIO_GetPin(SWITCH2_GPIO)
#define SWITCH1_LED_GPIO         P4_3
#define SWITCH1_LED_GPIO_PIN     GPIO_GetPin(SWITCH1_LED_GPIO)
#define SWITCH2_LED_GPIO         P4_2
#define SWITCH2_LED_GPIO_PIN     GPIO_GetPin(SWITCH2_LED_GPIO)
#define PROVISION_LED_GPIO       P4_1
#define PROVISION_LED_GPIO_PIN   GPIO_GetPin(PROVISION_LED_GPIO)

#define MAXPRESSTIME_5S   (250)  // 20ms定时器调用
typedef enum{
	SCAN_KEY_INIT = 0x00,
	SCAN_KEY_PRESS,    
	SCAN_KEY_PRESS_CONFIRM,    
	SCAN_KEY_RELEASE,
}SCAN_KEY_STATUS_e;


static uint8_t SwitchIO[SKYSWITC_NUMBERS]={SWITCH1_GPIO, SWITCH2_GPIO};
static uint8_t LedIO[SKYSWITC_NUMBERS]={SWITCH1_LED_GPIO, SWITCH2_LED_GPIO};

static int32_t presstime=0;
static SCAN_KEY_STATUS_e keystatus=SCAN_KEY_INIT;
static KEY_PRESS_MODE_e  keymode=KEY_MODE_INIT;

static SkySwitchManager *mSwitchManager=NULL;

static bool BlinkProLed=false;
/*
** GPIO
*/
static void HAL_GpioForSwitch_Init(void)
{
	/* Configure pad and pinmux firstly! */
	Pad_Config(SwitchIO[SKYSWITC1_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pinmux_Config(SwitchIO[SKYSWITC1_ENUM], DWGPIO);
	Pad_Config(SwitchIO[SKYSWITC2_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pinmux_Config(SwitchIO[SKYSWITC2_ENUM], DWGPIO);
		
	Pad_Config(LedIO[SKYSWITC1_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(LedIO[SKYSWITC1_ENUM], DWGPIO);
	Pad_Config(LedIO[SKYSWITC2_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(LedIO[SKYSWITC2_ENUM], DWGPIO);
	
	Pad_Config(PROVISION_LED_GPIO, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(PROVISION_LED_GPIO, DWGPIO);
	
	
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);	
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(SwitchIO[SKYSWITC1_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(SwitchIO[SKYSWITC2_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
	
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(LedIO[SKYSWITC1_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(LedIO[SKYSWITC2_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
	
    GPIO_InitStruct.GPIO_Pin    = PROVISION_LED_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
	
}

void HAL_ProvisionLed_Control(uint8_t mode)
{
	switch(mode){
		case LED_PRO_OFF:{
			GPIO_WriteBit(PROVISION_LED_GPIO_PIN, (BitAction)(LED_PRO_OFF));
		break;
		}
		case LED_PRO_ON:{
			GPIO_WriteBit(PROVISION_LED_GPIO_PIN, (BitAction)(LED_PRO_ON));
		break;
		}
		case LED_BLINK:{
			if(GPIO_ReadOutputDataBit(PROVISION_LED_GPIO_PIN)==1){
				GPIO_WriteBit(PROVISION_LED_GPIO_PIN, (BitAction)(LED_PRO_OFF));
			} else{
				GPIO_WriteBit(PROVISION_LED_GPIO_PIN, (BitAction)(LED_PRO_ON));
			}
		break;
		}
	}
}
void HAL_ProvLed_Dlps_Control(uint8_t val, bool isenter)
{
	PAD_OUTPUT_VAL outval;
	
	val = GPIO_ReadOutputDataBit(PROVISION_LED_GPIO_PIN);
	if(val){
		outval = PAD_OUT_HIGH;
	}else{
		outval = PAD_OUT_LOW;
	}
	if(isenter){
		Pad_Config(PROVISION_LED_GPIO, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, outval);
	}else{
		Pad_Config(PROVISION_LED_GPIO, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, outval);	
	}
		
}
void HAL_BlinkProLed_Enable(void)
{
	BlinkProLed = true;
}
void HAL_BlinkProLed_Disable(void)
{
	BlinkProLed = false;
}
bool HAL_BlinkProLed_Statu(void)
{
	return BlinkProLed;
}

void HAL_SwitchLed_Control(uint8_t index, uint8_t mode)
{
	if(index < SKYSWITC_NUMBERS){
		switch(mode){
			case LED_RED:{
				GPIO_WriteBit(GPIO_GetPin(LedIO[index]), (BitAction)(mode));
			break;
			}
			case LED_BLUE:{
				GPIO_WriteBit(GPIO_GetPin(LedIO[index]), (BitAction)(mode));
			break;
			}
			case LED_BLINK:{
				if(GPIO_ReadOutputDataBit(GPIO_GetPin(LedIO[index]))==1){					
					GPIO_WriteBit(GPIO_GetPin(LedIO[index]), (BitAction)(0));
				} else{
					GPIO_WriteBit(GPIO_GetPin(LedIO[index]), (BitAction)(1));
				}
			break;
			}
		}
		
	}	
}

void HAL_SwitchLed_Dlps_Control(uint8_t index, uint8_t val, bool isenter)
{
	PAD_OUTPUT_VAL outval;
	if(index < SKYSWITC_NUMBERS){
		val = GPIO_ReadOutputDataBit(GPIO_GetPin(LedIO[index]));
		if(val){
			outval = PAD_OUT_HIGH;
		}else{
			outval = PAD_OUT_LOW;
		}
		if(isenter){
			Pad_Config(LedIO[index], PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, outval);
		}else{
			Pad_Config(LedIO[index], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, outval);	
		}
		
	}	
}

static uint8_t ReadKeyStatu(void)
{
	uint8_t keyval=0 ;
	
	if(GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC1_ENUM]))==1){
		keyval |= (1<<0);
	} 
	
	if(GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC2_ENUM]))==1){
		keyval |= (1<<1);
	} 
	
	return keyval; 
}

static void Scan_Keyboard_Function(void)
{
	 // 20ms定时器调用
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
				if( oldkeypress==keypress){
					if( presstime < MAXPRESSTIME_5S ) {
						presstime++;
						if(presstime == MAXPRESSTIME_5S) {
							// Finish long press					
							keymode = KEY_LONGPRESS_MODE;
							
							mSwitchManager->keyval  = oldkeypress;
							mSwitchManager->keymode = keymode;
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
			
			if(presstime<MAXPRESSTIME_5S){
				// Finish short press
				keymode = KEY_SHORTPRESS_MODE;
				
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
	if(IsSwitchInited == false){
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
	uint8_t keypress = ReadKeyStatu();
	
	if(IsSwitchInited==true && keystatus==SCAN_KEY_INIT && keypress==0){
		return true;
	} else {
		return false;
	}

}

