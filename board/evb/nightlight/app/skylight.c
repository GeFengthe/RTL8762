

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <math.h>
#include "rtl876x.h"
#include "rtl876x_rcc.h"
#include "rtl876x_tim.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "platform_os.h"

#include "trace.h"
#include "skylight.h"
#include "app_skyiot_dlps.h"
#include "app_skyiot_server.h"


plt_timer_t LEDCtrl_timer = NULL;

// #define APP_DBG_PRINTF(fmt, ...)
#define APP_DBG_PRINTF   DBG_DIRECT

#if 1

#define PIN_INVALID       0xff
typedef struct
{
    uint8_t pin_num;
    uint8_t pin_func;
    TIM_TypeDef *tim_id;
    uint16_t duty_cycle;
	bool tmrenable;
} light_t;

static light_t light_pwm[TOTALPWMNUMBER] =
{
    {LED_FRONT, timer_pwm3, TIM3, PWM_DUTY_INIT, false},
    {LED_REAR, timer_pwm2, TIM2, PWM_DUTY_INIT, false},
};

static SkyLightManager *mLightManager=NULL;


/*
** PWM
*/
static void Pwm_Pin_Tmr_Config(const light_t *light)
{
    if (PIN_INVALID == light->pin_num)
    {
        return ;
    }

    Pad_Config(light->pin_num, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pinmux_Config(light->pin_num, light->pin_func);
    /* TIM */
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);
    TIM_InitStruct.TIM_PWM_En = PWM_ENABLE;
    /*<! PWM output freqency = 20M/(TIM_PWM_High_Count + TIM_PWM_Low_Count) */
    /*<! PWM duty cycle = TIM_PWM_High_Count/(TIM_PWM_High_Count + TIM_PWM_Low_Count) */
	
    TIM_InitStruct.TIM_PWM_High_Count = PWM_DUTY_INIT;
    TIM_InitStruct.TIM_PWM_Low_Count  = PWM_FREQUENCY;   

    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_SOURCE_DIV = TIM_CLOCK_DIVIDER_2;
    TIM_TimeBaseInit(light->tim_id, &TIM_InitStruct);
    /* Enable PWM output */
    // TIM_Cmd(light->tim_id, ENABLE);
    
}

static void pwm_enable_ctrl(light_t *light, bool isenable)
{
    if (PIN_INVALID == light->pin_num) {
        return ;
    }
	
	if(isenable == true){
	    /* ENABLE PWM output */
	    TIM_Cmd(light->tim_id, ENABLE);
		light->tmrenable = true;
	}else{
	    /* DISENABLE PWM output */
	    TIM_Cmd(light->tim_id, DISABLE);
		light->tmrenable = false;
	}
}

static void pwm_duty_cycle(const light_t *light)
{
    if (PIN_INVALID == light->pin_num) {
        return ;
    }
	
    TIM_PWMChangeFreqAndDuty(light->tim_id, light->duty_cycle, PWM_FREQUENCY-light->duty_cycle); 
}

void HAL_PwmForLight_Init(void)
{
    /* turn on timer clock */
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);
    *((volatile uint32_t *)0x40000360UL) &= ~(1 << 10);

	#ifdef FRONT_LED_PWM
	Pwm_Pin_Tmr_Config(&light_pwm[FRONT_LED_PWM]);  // 配置io为PWM
	#endif

	#ifdef REAR_LED_PWM
	Pwm_Pin_Tmr_Config(&light_pwm[REAR_LED_PWM]); 
	#endif
}

void HAL_PwmForLight_Deinit(void)
{
#ifdef FRONT_LED_PWM
    pwm_enable_ctrl(&light_pwm[FRONT_LED_PWM], false);
#endif
#ifdef REAR_LED_PWM
    pwm_enable_ctrl(&light_pwm[REAR_LED_PWM], false);
#endif

	RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, DISABLE);

}

static uint8_t HAL_UpdatePwmDuty(uint8_t lightindex, uint16_t val)
{
	if(lightindex!=FRONT_LED_PWM || lightindex!=REAR_LED_PWM){
		return 1;
	}
	
	if(val > 0){
		light_pwm[lightindex].duty_cycle = val;
		pwm_duty_cycle(&light_pwm[lightindex]);
    	pwm_enable_ctrl(&light_pwm[lightindex], true);
	}else{
    	pwm_enable_ctrl(&light_pwm[lightindex], false);
	}

	return 0;
}

bool HAL_Lighting_Output_Statu(void)
{
	bool haveoutput = true;
	
	if(light_pwm[FRONT_LED_PWM].tmrenable == false && light_pwm[REAR_LED_PWM].tmrenable == false){
		haveoutput = false;
	}

	return haveoutput;
}


///////////////////////////////////////////////////////////////////////////
#define LIGHT_BRIGHTNESS_PRECENT    (100)
#define LIGHT_GRADUAL_TIMES         (20)


typedef struct {	
	LED_MODE_e    mode;
    uint32_t  blinkcnt;
	uint32_t  blinktime;
}LightMonitor;
static LightMonitor mLightMonitor={
	LED_MODE_UNKOWN,
		0,
		0,
};

void HAL_Lighting_Nightlight(uint8_t lightindex, uint16_t val)
{   
	uint16_t lightval;
	
	if(mLightManager==NULL){
		return;
	}
	
	if(val>100 || (lightindex!=FRONT_LED_PWM || lightindex!=REAR_LED_PWM)){
		return;
	}

	lightval = val * PWM_FREQUENCY / LIGHT_BRIGHTNESS_PRECENT;
	HAL_UpdatePwmDuty(lightindex, lightval);
}


void HAL_Lighting_OFF(void)
{
	HAL_Lighting_Nightlight(FRONT_LED_PWM, 0);
	HAL_Lighting_Nightlight(REAR_LED_PWM, 0);
}

void HAL_Lighting_ON(void)
{		
	HAL_Lighting_Nightlight(FRONT_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
	HAL_Lighting_Nightlight(REAR_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
}

void HAL_Gradual_Nightlight(bool firstoff, bool front, bool rear)
{   
	uint8_t times = 0;
	uint16_t lightval;
	
	if(mLightManager==NULL){
		return;
	}

	if(firstoff == true){
		times = 1;
		HAL_Lighting_OFF();
		os_delay(2);
	}
	for(;times<=LIGHT_GRADUAL_TIMES;times++){
		lightval = PWM_FREQUENCY * times * (LIGHT_BRIGHTNESS_PRECENT / LIGHT_GRADUAL_TIMES);
		if(front==true){
			HAL_Lighting_Nightlight(FRONT_LED_PWM, lightval);
		}
		if(rear==true){
			HAL_Lighting_Nightlight(REAR_LED_PWM, lightval);
		}
		os_delay(2);
	}
}

bool HAL_Lighting_Init(SkyLightManager *manager)
{
	if( manager == NULL){
		return false;
	}
		
	mLightManager = manager;
	HAL_PwmForLight_Init();

	return true;	
}

void HAL_Light_Dlps_Control(bool isenter)
{
	if(isenter){
        if(SkyBleMesh_Batt_Station() == BATT_NORMAL){	
			HAL_PwmForLight_Deinit(); // qlj 新增
			
	        switch(mLightManager->mode){
	            case NLIGHT_MANUAL_MOD:{
					if( mLightManager->statu[FRONT_LED_PWM] == 1 ){
                    	Pad_Config(LED_FRONT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
					}else{
                    	Pad_Config(LED_FRONT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
					}
					
					if( mLightManager->statu[REAR_LED_PWM] == 1 ){
                    	Pad_Config(LED_REAR, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
					}else{
                    	Pad_Config(LED_REAR, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
					}	
	                break;
	            }			
	            case NLIGHT_REACT_LED1_MOD:  // 感应模式，全灭
	            case NLIGHT_REACT_LED2_MOD:
	            case NLIGHT_REACT_LEDALL_MOD:{
                    Pad_Config(LED_FRONT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
                    Pad_Config(LED_REAR,  PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);	
	                break;
	            }
				default:{
					break;
				}
	        }		
        }else{
        	// 电池预警，全灭
            Pad_Config(LED_FRONT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
            Pad_Config(LED_REAR,  PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
        }
		
	}else{	
	#if 1   // qlj 新增
        Pad_Config(LED_FRONT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
        Pad_Config(LED_REAR,  PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);		
		RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);
	    pwm_enable_ctrl(&light_pwm[FRONT_LED_PWM], true);
	    pwm_enable_ctrl(&light_pwm[REAR_LED_PWM], true);
	#else
		// qlj 增加 带参初始化
	
        // HAL_PwmForLight_Init();
	#endif
	}
}

#include "app_task.h"
static void SkyLed_Timeout_cb(void *timer)
{
    T_IO_MSG msg;
    msg.type = IO_MSG_TYPE_TIMER;
    msg.subtype = test_light_TIMEOUT;
    app_send_msg_to_apptask(&msg);
}
void SkyLed_Timeout_cb_handel(void *timer)
{
	if(mLightMonitor.blinkcnt){
		switch(mLightMonitor.mode){	
			case LED_MODE_FAST_BLINK:{				
				mLightMonitor.blinktime += LED_BRIGHT_TMR_PERIOD;
				if(mLightMonitor.blinktime >= LED_FAST_BLINK_PERIOD){
					if((mLightMonitor.blinkcnt&0x01) == 1){
						HAL_Lighting_Nightlight(FRONT_LED_PWM,  0);
						HAL_Lighting_Nightlight(REAR_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
					}else{
						HAL_Lighting_Nightlight(FRONT_LED_PWM, 0);
						HAL_Lighting_Nightlight(REAR_LED_PWM,  0);
					}
					mLightMonitor.blinkcnt--;
					mLightMonitor.blinktime = 0;
				}
			break;
			}
			case LED_MODE_SLOW_BLINK:{
				mLightMonitor.blinktime += LED_BRIGHT_TMR_PERIOD;
				if(mLightMonitor.blinktime >= LED_SLOW_BLINK_PERIOD){
					if((mLightMonitor.blinkcnt&0x01) == 1){
						HAL_Lighting_Nightlight(FRONT_LED_PWM,  0);
						HAL_Lighting_Nightlight(REAR_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
					}else{
						HAL_Lighting_Nightlight(FRONT_LED_PWM, 0);
						HAL_Lighting_Nightlight(REAR_LED_PWM,  0);
					}
					mLightMonitor.blinkcnt--;
					mLightMonitor.blinktime = 0;
				}
			break;
			}
			case LED_MODE_MODE_BLINK:{
				mLightMonitor.blinktime += LED_BRIGHT_TMR_PERIOD;
				if(mLightMonitor.blinktime >= LED_SLOW_BLINK_PERIOD){
					if((mLightMonitor.blinkcnt&0x01) == 1){
						if((mLightManager->mode == NLIGHT_MANUAL_MOD &&  mLightManager->statu[FRONT_LED_PWM] == 1)
						   || mLightManager->mode == NLIGHT_REACT_LED1_MOD){
						   // operat LED1
							HAL_Lighting_Nightlight(FRONT_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
							HAL_Lighting_Nightlight(REAR_LED_PWM, 0);
						}else if((mLightManager->mode == NLIGHT_MANUAL_MOD &&  mLightManager->statu[REAR_LED_PWM] == 1)
						   || mLightManager->mode == NLIGHT_REACT_LED2_MOD){
						   // operat LED2
							HAL_Lighting_Nightlight(FRONT_LED_PWM,  0);
							HAL_Lighting_Nightlight(REAR_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
						}else if((mLightManager->mode == NLIGHT_MANUAL_MOD && mLightManager->statu[FRONT_LED_PWM] == 1 && mLightManager->statu[REAR_LED_PWM] == 1)
						   || mLightManager->mode == NLIGHT_REACT_LEDALL_MOD){
						   // operat LED2\LED1
							HAL_Lighting_Nightlight(FRONT_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
							HAL_Lighting_Nightlight(REAR_LED_PWM,  LIGHT_BRIGHTNESS_PRECENT);
						}
					}else{
						HAL_Lighting_Nightlight(FRONT_LED_PWM, 0);
						HAL_Lighting_Nightlight(REAR_LED_PWM,  0);
					}
					mLightMonitor.blinkcnt--;
					mLightMonitor.blinktime = 0;
				}
			break;
			}
			case LED_MODE_DELAY_BRIGHT:{				
				mLightMonitor.blinktime += LED_BRIGHT_TMR_PERIOD;
				if(mLightMonitor.blinktime >= LED_BRIGHT_TMR_PERIOD){
					HAL_Lighting_OFF();
					mLightMonitor.blinkcnt--;
					mLightMonitor.blinktime = 0;
				}

			break;
			}
			case LED_MODE_NORMAL_BRIGHT:{

			break;
			}
			default:{
			break;
			}
		}
	}else{
			
			switch(mLightMonitor.mode){ 
				case LED_MODE_FAST_BLINK:{
					if(mLightManager->mode == NLIGHT_MANUAL_MOD){ // 手动模式，灯状态恢复
						if(mLightManager->statu[FRONT_LED_PWM] == 1){
							HAL_Lighting_Nightlight(FRONT_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
						}
						if(mLightManager->statu[REAR_LED_PWM] == 1){
							HAL_Lighting_Nightlight(REAR_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
						}
					}
					mLightMonitor.mode = LED_MODE_UNKOWN;
				break;
				}
				case LED_MODE_SLOW_BLINK:{
					HAL_Lighting_OFF(); // 关闭，等待重启
					mLightMonitor.mode = LED_MODE_UNKOWN;
				break;
				}
				case LED_MODE_MODE_BLINK:{
					if(mLightManager->mode == NLIGHT_MANUAL_MOD){ // 手动模式，灯状态恢复
						if(mLightManager->statu[FRONT_LED_PWM] == 1){
							HAL_Lighting_Nightlight(FRONT_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
						}
						if(mLightManager->statu[REAR_LED_PWM] == 1){
							HAL_Lighting_Nightlight(REAR_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
						}
					}else if(mLightManager->mode == NLIGHT_REACT_LED1_MOD
						  || mLightManager->mode == NLIGHT_REACT_LED2_MOD
						  || mLightManager->mode == NLIGHT_REACT_LEDALL_MOD){ // 感应模式亮灯延时	LED_DELAY_BRIGHT_TIME		转换到cnt上				
						mLightMonitor.blinkcnt = LED_DELAY_BRIGHT_TIME/LED_BRIGHT_TMR_PERIOD;
						mLightMonitor.blinktime = 0;
						mLightMonitor.mode = LED_MODE_DELAY_BRIGHT;
					}
				break;
				}
				case LED_MODE_DELAY_BRIGHT:
				case LED_MODE_NORMAL_BRIGHT:{						
					mLightMonitor.mode = LED_MODE_UNKOWN;
				break;
				}
				default:{
				break;
				}
			}
	
	}
	if(mLightMonitor.mode == LED_MODE_UNKOWN && mLightMonitor.blinkcnt==0){		
		Delete_LED_Timer();
	}
		
}

void Start_LED_Timer(void)
{
	if(LEDCtrl_timer == NULL){
		LEDCtrl_timer = plt_timer_create("ledtest", 50, true, 0, SkyLed_Timeout_cb); 
		if(LEDCtrl_timer != NULL){
			Led_Relay_tmr_ctrl_dlps(false);
			plt_timer_start(LEDCtrl_timer, 0);
		}
	}
	mLightMonitor.blinktime = 0;
}

void Delete_LED_Timer(void)
{
	if(LEDCtrl_timer){
   		plt_timer_delete(LEDCtrl_timer, 0);
		LEDCtrl_timer = NULL;
	}
    Led_Relay_tmr_ctrl_dlps(true);
}


void SkyLed_LightEffective_CTL(bool blink, LED_MODE_e blinkmode, uint16_t blinkcnt)
{
    if(blink){
		mLightMonitor.mode      = blinkmode;
		mLightMonitor.blinkcnt  = blinkcnt;
		mLightMonitor.blinktime = 0;
		Start_LED_Timer();
    }else{        
        switch(mLightManager->mode){
            case NLIGHT_MANUAL_MOD:{
				if( mLightManager->statu[FRONT_LED_PWM] == 1 ){
					HAL_Lighting_Nightlight(FRONT_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
				}else{
					HAL_Lighting_Nightlight(FRONT_LED_PWM,  0);
				}
				
				if( mLightManager->statu[REAR_LED_PWM] == 1 ){
					HAL_Lighting_Nightlight(REAR_LED_PWM, LIGHT_BRIGHTNESS_PRECENT);
				}else{
					HAL_Lighting_Nightlight(REAR_LED_PWM,  0);
				}				
                break;
            }			
            case NLIGHT_REACT_LED1_MOD:{
				HAL_Gradual_Nightlight(true, true, false);			
                break;
            }	
            case NLIGHT_REACT_LED2_MOD:{
				HAL_Gradual_Nightlight(true, false, true);		
                break;
            }	
            case NLIGHT_REACT_LEDALL_MOD:{
				HAL_Gradual_Nightlight(true, true, true);			
                break;
            }
			default:{
				break;
			}
        }
           
    }
   // APP_DBG_PRINTF("TIM_GetPWMOutputStatus:%d, %d", TIM_GetPWMOutputStatus(TIM2), TIM_GetPWMOutputStatus(TIM3)); // 读取当前定时器的PWM电平（高低）
}


#endif
