

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <math.h>
#include "platform_os.h"
#include "rtl876x_rcc.h"
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_nvic.h"
#include "skyswitch.h"
#include "trace.h"

#include "data_uart.h"
#include "app_skyiot_dlps.h"

// #define APP_DBG_PRINTF(fmt, ...)
#define APP_DBG_PRINTF   DBG_DIRECT

#define LED_RED    LEDTURNOFF 
#define LED_BLUE   LEDTURNON  
#define LED_BLINK  LEDBLINK  

#define SWITCH1_RELAY_CTL_USED  1
#if SWITCH1_RELAY_CTL_USED == 1
// 这里IO统一在tmr里面处理，如有例外加保护，注意重入
#define SWITCH1_RELAYON_GPIO          P3_3
#define SWITCH1_RELAYON_GPIO_PIN      GPIO_GetPin(SWITCH1_RELAYON_GPIO)
#define SWITCH1_RELAYOFF_GPIO         P4_1
#define SWITCH1_RELAYOFF_GPIO_PIN     GPIO_GetPin(SWITCH1_RELAYOFF_GPIO)

#define SWITCH2_RELAYON_GPIO          P3_2
#define SWITCH2_RELAYON_GPIO_PIN      GPIO_GetPin(SWITCH2_RELAYON_GPIO)
#define SWITCH2_RELAYOFF_GPIO         P4_0
#define SWITCH2_RELAYOFF_GPIO_PIN     GPIO_GetPin(SWITCH2_RELAYOFF_GPIO)

#define SWITCH3_RELAYON_GPIO          P0_6
#define SWITCH3_RELAYON_GPIO_PIN      GPIO_GetPin(SWITCH3_RELAYON_GPIO)
#define SWITCH3_RELAYOFF_GPIO         P0_5
#define SWITCH3_RELAYOFF_GPIO_PIN     GPIO_GetPin(SWITCH3_RELAYOFF_GPIO)

static uint8_t RelayOnIO[SKYSWITC_NUMBERS]  = {SWITCH1_RELAYON_GPIO, SWITCH2_RELAYON_GPIO, SWITCH3_RELAYON_GPIO};
static uint8_t RelayOffIO[SKYSWITC_NUMBERS] = {SWITCH1_RELAYOFF_GPIO, SWITCH2_RELAYOFF_GPIO, SWITCH3_RELAYOFF_GPIO};
#endif

// 过零检查,用中断实现
#define CHECK_ZVD_GPIO               P2_5 
#define CHECK_ZVD_GPIO_PIN           GPIO_GetPin(CHECK_ZVD_GPIO)
#define CHECK_ZVD_PIN_INPUT_IRQN     GPIO21_IRQn
#define CHECK_ZVD_PIN_INPUT_Handler  GPIO21_Handler

#define SWITCH1_GPIO             P2_3  // switch 1\2 文档与硬件是反的，这里按硬件处理
#define SWITCH1_GPIO_PIN         GPIO_GetPin(SWITCH1_GPIO)
#define SWITCH2_GPIO             P2_4
#define SWITCH2_GPIO_PIN         GPIO_GetPin(SWITCH2_GPIO)
#define SWITCH3_GPIO             P2_2
#define SWITCH3_GPIO_PIN         GPIO_GetPin(SWITCH3_GPIO)

#define PROVISION_LED_GPIO       P2_7
#define PROVISION_LED_GPIO_PIN   GPIO_GetPin(PROVISION_LED_GPIO)

#define MAXPRESSTIME_5S   (250)  // 20ms定时器调用
typedef enum{
	SCAN_KEY_INIT = 0x00,
	SCAN_KEY_PRESS,    
	SCAN_KEY_PRESS_CONFIRM,    
	SCAN_KEY_RELEASE,
}SCAN_KEY_STATUS_e;


static uint8_t SwitchIO[SKYSWITC_NUMBERS]={SWITCH1_GPIO, SWITCH2_GPIO, SWITCH3_GPIO};

static int32_t presstime=0;
static SCAN_KEY_STATUS_e keystatus = SCAN_KEY_INIT;
static KEY_PRESS_MODE_e  keymode   = KEY_MODE_INIT;

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
	Pad_Config(SwitchIO[SKYSWITC3_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pinmux_Config(SwitchIO[SKYSWITC3_ENUM], DWGPIO);
	
	Pad_Config(CHECK_ZVD_GPIO, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pinmux_Config(CHECK_ZVD_GPIO, DWGPIO);
	#if SWITCH1_RELAY_CTL_USED == 1	
	Pad_Config(RelayOnIO[SKYSWITC1_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(RelayOnIO[SKYSWITC1_ENUM], DWGPIO);
	Pad_Config(RelayOnIO[SKYSWITC2_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(RelayOnIO[SKYSWITC2_ENUM], DWGPIO);
	Pad_Config(RelayOnIO[SKYSWITC3_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(RelayOnIO[SKYSWITC3_ENUM], DWGPIO);
	
	Pad_Config(RelayOffIO[SKYSWITC1_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(RelayOffIO[SKYSWITC1_ENUM], DWGPIO);
	Pad_Config(RelayOffIO[SKYSWITC2_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(RelayOffIO[SKYSWITC2_ENUM], DWGPIO);
	Pad_Config(RelayOffIO[SKYSWITC3_ENUM], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(RelayOffIO[SKYSWITC3_ENUM], DWGPIO);
	#endif
	
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
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(SwitchIO[SKYSWITC3_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
	
	#if SWITCH1_RELAY_CTL_USED == 1	
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(RelayOnIO[SKYSWITC1_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(RelayOnIO[SKYSWITC2_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(RelayOnIO[SKYSWITC3_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
	
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(RelayOffIO[SKYSWITC1_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(RelayOffIO[SKYSWITC2_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(RelayOffIO[SKYSWITC3_ENUM]);
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
	#endif
	
    GPIO_InitStruct.GPIO_Pin    = PROVISION_LED_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
	
	
    GPIO_InitStruct.GPIO_Pin    = CHECK_ZVD_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
	#if 1
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
	#else
    GPIO_InitStruct.GPIO_ITCmd  = ENABLE;
    GPIO_InitStruct.GPIO_ITTrigger  = GPIO_INT_Trigger_LEVEL;
    GPIO_InitStruct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_HIGH;
    GPIO_InitStruct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_DISABLE;
    GPIO_Init(&GPIO_InitStruct);	
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = CHECK_ZVD_PIN_INPUT_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    GPIO_MaskINTConfig(CHECK_ZVD_GPIO_PIN, DISABLE);
    GPIO_INTConfig(CHECK_ZVD_GPIO_PIN, ENABLE);
	#endif
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


#if SWITCH1_RELAY_CTL_USED == 1
plt_timer_t Sw1OnCtrl_timer = NULL;
plt_timer_t Sw2OnCtrl_timer = NULL;
plt_timer_t Sw3OnCtrl_timer = NULL;

static void HAL_Sw1Relay_OnCtl_Timeout_cb(void *timer)
{
	static uint8_t enrtecnt=0;
	
	enrtecnt++;
	APP_DBG_PRINTF("%s Read_ZVD_Statu:%02X r\n",__func__, enrtecnt);
	if( enrtecnt == 1 ){ // on
		if(mSwitchManager->status[SKYSWITC1_ENUM] == 0){
			GPIO_WriteBit(GPIO_GetPin(RelayOffIO[SKYSWITC1_ENUM]), Bit_SET);	
		}else{
			GPIO_WriteBit(GPIO_GetPin(RelayOnIO[SKYSWITC1_ENUM]), Bit_SET);	
		}			
		
		if(Sw1OnCtrl_timer){
			plt_timer_change_period(Sw1OnCtrl_timer, 20, 0);
		}

	} else { // ==2 off
		
		if(mSwitchManager->status[SKYSWITC1_ENUM] == 0){
			GPIO_WriteBit(GPIO_GetPin(RelayOffIO[SKYSWITC1_ENUM]), Bit_RESET);	
		}else{
			GPIO_WriteBit(GPIO_GetPin(RelayOnIO[SKYSWITC1_ENUM]), Bit_RESET);	
		}
		
		if(Sw1OnCtrl_timer){
			plt_timer_stop(Sw1OnCtrl_timer, 0);			
			Switch_Relay1_tmr_ctrl_dlps(true);
		}
		enrtecnt = 0;
	}
}

#define TMP_WAIT_ZVD_SIGNAL_CNT   1000   
static void HAL_Sw2Relay_OnCtl_Timeout_cb(void *timer)
{	
	static uint16_t enrtecnt=0; // 0~4:tmr计数 5:on 6:off
	static uint8_t oldzvd=1;
	
	enrtecnt++;
	APP_DBG_PRINTF("%s Read_ZVD_Statu:%02X r\n",__func__, enrtecnt);
	if(enrtecnt < TMP_WAIT_ZVD_SIGNAL_CNT){
		if(Read_ZVD_Statu()!=oldzvd){ 
			
			data_uart_debug("%s Read_ZVD_Statu:%02X %d %d r\n",__func__, Read_ZVD_Statu(), oldzvd, enrtecnt);
			if(oldzvd==1){
				enrtecnt = TMP_WAIT_ZVD_SIGNAL_CNT-1;
				plt_timer_change_period(Sw2OnCtrl_timer, 6, 0);
				return;
			}
		}
		oldzvd = Read_ZVD_Statu();
	}	
	else if( enrtecnt == TMP_WAIT_ZVD_SIGNAL_CNT ){ // on
		if(mSwitchManager->status[SKYSWITC2_ENUM] == 0){
			GPIO_WriteBit(GPIO_GetPin(RelayOffIO[SKYSWITC2_ENUM]), Bit_SET);	
		}else{
			GPIO_WriteBit(GPIO_GetPin(RelayOnIO[SKYSWITC2_ENUM]), Bit_SET);	
		}			
		
		if(Sw2OnCtrl_timer){
			plt_timer_change_period(Sw2OnCtrl_timer, 20, 0);
		}
		enrtecnt++;
	} else if(enrtecnt>TMP_WAIT_ZVD_SIGNAL_CNT){ // ==6 off
		
		if(mSwitchManager->status[SKYSWITC2_ENUM] == 0){
			GPIO_WriteBit(GPIO_GetPin(RelayOffIO[SKYSWITC2_ENUM]), Bit_RESET);	
		}else{
			GPIO_WriteBit(GPIO_GetPin(RelayOnIO[SKYSWITC2_ENUM]), Bit_RESET);	
		}
		
		if(Sw2OnCtrl_timer){
			plt_timer_stop(Sw2OnCtrl_timer, 0);
			Switch_Relay2_tmr_ctrl_dlps(true);
		}
		data_uart_debug("%s Sw2OnCtrl_timer:%d r\n",__func__, enrtecnt);
		
		enrtecnt = 0;
		
	}
}

static void HAL_Sw3Relay_OnCtl_Timeout_cb(void *timer)
{
	static uint8_t enrtecnt=0;
	
	enrtecnt++;
	APP_DBG_PRINTF("%s Read_ZVD_Statu:%02X r\n",__func__, enrtecnt);
	if( enrtecnt == 1 ){ // on
		if(mSwitchManager->status[SKYSWITC3_ENUM] == 0){
			GPIO_WriteBit(GPIO_GetPin(RelayOffIO[SKYSWITC3_ENUM]), Bit_SET);	
		}else{
			GPIO_WriteBit(GPIO_GetPin(RelayOnIO[SKYSWITC3_ENUM]), Bit_SET);	
		}			
		
		if(Sw3OnCtrl_timer){
			plt_timer_change_period(Sw3OnCtrl_timer, 20, 0);
		}

	} else { // ==2 off
		
		if(mSwitchManager->status[SKYSWITC3_ENUM] == 0){
			GPIO_WriteBit(GPIO_GetPin(RelayOffIO[SKYSWITC3_ENUM]), Bit_RESET);	
		}else{
			GPIO_WriteBit(GPIO_GetPin(RelayOnIO[SKYSWITC3_ENUM]), Bit_RESET);	
		}
		
		if(Sw3OnCtrl_timer){
			plt_timer_stop(Sw3OnCtrl_timer, 0);
			Switch_Relay3_tmr_ctrl_dlps(true);
		}
		enrtecnt = 0;
	}
}

#endif
void HAL_SwitchLed_Control(uint8_t index, uint8_t mode)
{
	APP_DBG_PRINTF("%s Read_ZVD_Statu:%02X %d r\n",__func__, index, mode);
	#if SWITCH1_RELAY_CTL_USED == 1
	if(index < SKYSWITC_NUMBERS){
		switch(mode){
			case LEDTURNON:{
				if(index == SKYSWITC1_ENUM){
					Switch_Relay1_tmr_ctrl_dlps(false);
					if(Sw1OnCtrl_timer == NULL){
						Sw1OnCtrl_timer = plt_timer_create("s1c1", 6, true, 111, HAL_Sw1Relay_OnCtl_Timeout_cb);
						if (Sw1OnCtrl_timer != NULL){
							plt_timer_start(Sw1OnCtrl_timer, 0);
						}	
					}else{		
						if(plt_timer_is_active(Sw1OnCtrl_timer)==false){
							plt_timer_change_period(Sw1OnCtrl_timer, 6, 0);
						}
					}

				} else if(index == SKYSWITC2_ENUM){
					Switch_Relay2_tmr_ctrl_dlps(false);
					if(Sw2OnCtrl_timer == NULL){
						Sw2OnCtrl_timer = plt_timer_create("s2c1", 5, true, 111, HAL_Sw2Relay_OnCtl_Timeout_cb);
						if (Sw2OnCtrl_timer != NULL){
							plt_timer_start(Sw2OnCtrl_timer, 0);
						}	
					}else{		
						if(plt_timer_is_active(Sw2OnCtrl_timer)==false){
							plt_timer_change_period(Sw2OnCtrl_timer, 5, 0);
						}
					}

				} else if(index == SKYSWITC3_ENUM){
					Switch_Relay3_tmr_ctrl_dlps(false);
					if(Sw3OnCtrl_timer == NULL){
						Sw3OnCtrl_timer = plt_timer_create("s3c1", 6, true, 111, HAL_Sw3Relay_OnCtl_Timeout_cb);
						if (Sw3OnCtrl_timer != NULL){
							plt_timer_start(Sw3OnCtrl_timer, 0);
						}	
					}else{		
						if(plt_timer_is_active(Sw3OnCtrl_timer)==false){
							plt_timer_change_period(Sw3OnCtrl_timer, 6, 0);
						}
					}

				}
			break;
			}
			case LEDTURNOFF:{
				if(index == SKYSWITC1_ENUM){
					Switch_Relay1_tmr_ctrl_dlps(false);
					if(Sw1OnCtrl_timer == NULL){
						Sw1OnCtrl_timer = plt_timer_create("s1c1", 6, true, 111, HAL_Sw1Relay_OnCtl_Timeout_cb);
						if (Sw1OnCtrl_timer != NULL){
							plt_timer_start(Sw1OnCtrl_timer, 0);
						}	
					}else{		
						if(plt_timer_is_active(Sw1OnCtrl_timer)==false){
							plt_timer_change_period(Sw1OnCtrl_timer, 6, 0);
						}
					}

				} else if(index == SKYSWITC2_ENUM){
					Switch_Relay2_tmr_ctrl_dlps(false);
					if(Sw2OnCtrl_timer == NULL){
						Sw2OnCtrl_timer = plt_timer_create("s2c1", 5, true, 111, HAL_Sw2Relay_OnCtl_Timeout_cb);
						if (Sw2OnCtrl_timer != NULL){
							plt_timer_start(Sw2OnCtrl_timer, 0);
						}	
					}else{		
						if(plt_timer_is_active(Sw2OnCtrl_timer)==false){
							plt_timer_change_period(Sw2OnCtrl_timer, 5, 0);
						}
					}

				} else if(index == SKYSWITC3_ENUM){
					Switch_Relay3_tmr_ctrl_dlps(false);					
					if(Sw3OnCtrl_timer == NULL){
						Sw3OnCtrl_timer = plt_timer_create("s3c1", 6, true, 111, HAL_Sw3Relay_OnCtl_Timeout_cb);
						if (Sw3OnCtrl_timer != NULL){
							plt_timer_start(Sw3OnCtrl_timer, 0);
						}	
					}else{		
						if(plt_timer_is_active(Sw3OnCtrl_timer)==false){
							plt_timer_change_period(Sw3OnCtrl_timer, 6, 0);
						}
					}
				}
			break;
			}
		}
	}	
	#endif
}

void HAL_SwitchLed_Dlps_Control(uint8_t index, uint8_t val, bool isenter)
{
	#if SWITCH1_RELAY_CTL_USED == 1
	// 这里先成对处理，可以直接给低
	PAD_OUTPUT_VAL outval;
	if(index < SKYSWITC_NUMBERS){
//		val = GPIO_ReadOutputDataBit(GPIO_GetPin(RelayOnIO[index]));
//		if(val){
//			outval = PAD_OUT_HIGH;
//		}else{
			outval = PAD_OUT_LOW;
//		}		
		if(isenter){
			Pad_Config(RelayOnIO[index], PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, outval);
		}else{
			Pad_Config(RelayOnIO[index], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, outval);	
		}
		
//		val = GPIO_ReadOutputDataBit(GPIO_GetPin(RelayOffIO[index]));
//		if(val){
//			outval = PAD_OUT_HIGH;
//		}else{
			outval = PAD_OUT_LOW;
//		}
		if(isenter){
			Pad_Config(RelayOffIO[index], PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, outval);
		}else{
			Pad_Config(RelayOffIO[index], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, outval);	
		}
	}	
	#endif
}


static uint8_t ReadKeyStatu(void)
{
	uint8_t keyval=0 ;
	
	if(GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC1_ENUM]))==0){
		keyval |= (1<<0);
	} 	
	if(GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC2_ENUM]))==0){
		keyval |= (1<<1);
	} 
	if(GPIO_ReadInputDataBit(GPIO_GetPin(SwitchIO[SKYSWITC3_ENUM]))==0){
		keyval |= (1<<2);
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
	uint8_t keypress=0;
	if(mSwitchManager->keyval==0 && mSwitchManager->keymode==0) {  // 没有未处理键值
		
		keypress = ReadKeyStatu();		
		if(IsSwitchInited==true && keystatus==SCAN_KEY_INIT && keypress==0){ // 按键释放
			return true;
		} 
	}  
	
	return false;
}


extern uint8_t Read_ZVD_Statu(void)
{
	uint8_t zvdval=0 ;
	
	zvdval = GPIO_ReadInputDataBit(CHECK_ZVD_GPIO_PIN);
	
	return zvdval; 
}

void CHECK_ZVD_PIN_INPUT_Handler(void)
{
    GPIO_INTConfig(CHECK_ZVD_GPIO_PIN, DISABLE);
    GPIO_MaskINTConfig(CHECK_ZVD_GPIO_PIN, ENABLE);
	
//    APP_PRINT_INFO0("Enter GPIO Interrupt");
    DBG_DIRECT("Enter GPIO Interrupt!");

    GPIO_ClearINTPendingBit(CHECK_ZVD_GPIO_PIN);
    GPIO_MaskINTConfig(CHECK_ZVD_GPIO_PIN, DISABLE);
    GPIO_INTConfig(CHECK_ZVD_GPIO_PIN, ENABLE);

}
