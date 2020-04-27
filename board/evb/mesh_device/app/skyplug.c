

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <math.h>
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "skyplug.h"


#define APP_DBG_PRINTF(fmt, ...)


#define SWITCH_GPIO              P0_1
#define SWITCH_GPIO_PIN          GPIO_GetPin(SWITCH_GPIO)
#define RELAY1_LED_GPIO          P0_3
#define RELAY1_LED_GPIO_PIN      GPIO_GetPin(RELAY1_LED_GPIO)
#define RELAY1_CTR_GPIO          P0_4
#define RELAY1_CTR_GPIO_PIN      GPIO_GetPin(RELAY1_CTR_GPIO)
#define PROVISION_LED_GPIO       P0_5
#define PROVISION_LED_GPIO_PIN   GPIO_GetPin(PROVISION_LED_GPIO)

#define MAXPRESSTIME_5S   (250)  // 20ms定时器调用
typedef enum{
	SCAN_KEY_INIT = 0x00,
	SCAN_KEY_PRESS,    
	SCAN_KEY_PRESS_CONFIRM,    
	SCAN_KEY_RELEASE,
}SCAN_KEY_STATUS_e;


static uint8_t SwitchIO[SKYSWITC_NUMBERS]={SWITCH_GPIO};
static uint8_t LedIO[SKYSWITC_NUMBERS]={RELAY1_LED_GPIO};
static uint8_t RelayIO[SKYSWITC_NUMBERS]={RELAY1_CTR_GPIO};

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
		
	Pad_Config(LedIO[SKYSWITC1_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pinmux_Config(LedIO[SKYSWITC1_ENUM], DWGPIO);
	Pad_Config(RELAY1_CTR_GPIO, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pinmux_Config(RELAY1_CTR_GPIO, DWGPIO);
	
	Pad_Config(PROVISION_LED_GPIO, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pinmux_Config(PROVISION_LED_GPIO, DWGPIO);
	
	
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);	
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(SwitchIO[SKYSWITC1_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
	
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(LedIO[SKYSWITC1_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(RELAY1_CTR_GPIO);
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
		case LEDTURNOFF:{
			GPIO_WriteBit(PROVISION_LED_GPIO_PIN, (BitAction)(LEDTURNOFF));
		break;
		}
		case LEDTURNON:{
			GPIO_WriteBit(PROVISION_LED_GPIO_PIN, (BitAction)(LEDTURNON));
		break;
		}
		case LEDBLINK:{
			if(GPIO_ReadOutputDataBit(PROVISION_LED_GPIO_PIN)==1){
				GPIO_WriteBit(PROVISION_LED_GPIO_PIN, (BitAction)(LEDTURNOFF));
			} else{
				GPIO_WriteBit(PROVISION_LED_GPIO_PIN, (BitAction)(LEDTURNON));
			}
		break;
		}
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
			case LEDTURNOFF:{
				GPIO_WriteBit(GPIO_GetPin(LedIO[index]), (BitAction)(mode));
			break;
			}
			case LEDTURNON:{
				GPIO_WriteBit(GPIO_GetPin(LedIO[index]), (BitAction)(mode));
			break;
			}
			case LEDBLINK:{
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

void HAL_SwitchRelay_Control(uint8_t index, uint8_t mode)
{
	if(index < SKYSWITC_NUMBERS){
		switch(mode){
			case RELAY_ON:{
				GPIO_WriteBit(GPIO_GetPin(RelayIO[index]), (BitAction)(mode));
			break;
			}
			case RELAY_OFF:{
				GPIO_WriteBit(GPIO_GetPin(RelayIO[index]), (BitAction)(mode));
			break;
			}
		}
		
	}	
}

static uint8_t ReadKeyStatu(void)
{
	uint8_t keyval=0 ;
	
	if(GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC1_ENUM]))==0){
		keyval |= (1<<0);
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



