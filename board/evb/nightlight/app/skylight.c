

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
static uint16_t m_step_cnt = 0;

// #define APP_DBG_PRINTF(fmt, ...)
#define APP_DBG_PRINTF   DBG_DIRECT

#if 1
#define FRONT_LED_PWM		0
#define REAR_LED_PWM		1
#define TOTALPWMNUMBER     	2

#define PIN_INVALID       0xff
typedef struct
{
    uint8_t pin_num;
    uint8_t pin_func;
    TIM_TypeDef *tim_id;
    uint16_t duty_cycle;
} light_t;

static light_t light_pwm[TOTALPWMNUMBER] =
{
    {LED_FRONT, timer_pwm3, TIM3, PWM_DUTY_INIT},
    {LED_REAR, timer_pwm2, TIM2, PWM_DUTY_INIT},
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

    Pad_Config(light->pin_num, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    Pinmux_Config(light->pin_num, light->pin_func);
    /* TIM */
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);
    TIM_InitStruct.TIM_PWM_En = PWM_ENABLE;
    /*<! PWM output freqency = 20M/(TIM_PWM_High_Count + TIM_PWM_Low_Count) */
    /*<! PWM duty cycle = TIM_PWM_High_Count/(TIM_PWM_High_Count + TIM_PWM_Low_Count) */
    
    if(light->pin_num == LED_FRONT){
        switch((uint8_t)mLightManager->light_newmode)
        {
            case RELEASE_MODE:
            case REACT_MODE_M:
            case REACT_MODE_S:
            case REACT_MODE_A:
            case UNREACT_MODE_N:
            case UNREACT_MODE_S:
                TIM_InitStruct.TIM_PWM_High_Count = PWM_DUTY_INIT;
                TIM_InitStruct.TIM_PWM_Low_Count  = PWM_FREQUENCY;
                break;
            case UNREACT_MODE_M:
            case UNREACT_MODE_A:
                TIM_InitStruct.TIM_PWM_High_Count = PWM_FREQUENCY;
                TIM_InitStruct.TIM_PWM_Low_Count  = PWM_DUTY_INIT;
                break;
        }
    }else if(light->pin_num == LED_REAR){
        switch((uint8_t)mLightManager->light_newmode)
        {
            case RELEASE_MODE:
            case REACT_MODE_M:
            case REACT_MODE_S:
            case REACT_MODE_A:
            case UNREACT_MODE_N:
            case UNREACT_MODE_M:
                TIM_InitStruct.TIM_PWM_High_Count = PWM_DUTY_INIT;
                TIM_InitStruct.TIM_PWM_Low_Count  = PWM_FREQUENCY;
                break;
            case UNREACT_MODE_S:
            case UNREACT_MODE_A:
                TIM_InitStruct.TIM_PWM_High_Count = PWM_FREQUENCY;
                TIM_InitStruct.TIM_PWM_Low_Count  = PWM_DUTY_INIT;
                break;
        }
    }
    
	

    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_InitStruct.TIM_SOURCE_DIV = TIM_CLOCK_DIVIDER_2;
    TIM_TimeBaseInit(light->tim_id, &TIM_InitStruct);
    /* Enable PWM output */
    if((light->pin_num == LED_FRONT && (mLightManager->light_newmode == UNREACT_MODE_M || mLightManager->light_newmode == UNREACT_MODE_A)) ||
       (light->pin_num == LED_REAR && (mLightManager->light_newmode == UNREACT_MODE_S || mLightManager->light_newmode == UNREACT_MODE_A))){
        if(SkyBleMesh_Batt_Station() == BATT_NORMAL){
            TIM_Cmd(light->tim_id, ENABLE);
        }
    }
    
}

static void pwm_disable(const light_t *light)
{
    if (PIN_INVALID == light->pin_num)
    {
        return ;
    }
	
    /* DISENABLE PWM output */
    TIM_Cmd(light->tim_id, DISABLE);
}


static void pwm_enable(const light_t *light)
{
    if (PIN_INVALID == light->pin_num)
    {
        return ;
    }
	
    /* DISENABLE PWM output */
    TIM_Cmd(light->tim_id, ENABLE);
}

static void pwm_duty_cycle(const light_t *light)
{
    if (PIN_INVALID == light->pin_num)
    {
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
    pwm_disable(&light_pwm[FRONT_LED_PWM]);
#endif
#ifdef REAR_LED_PWM
    pwm_disable(&light_pwm[REAR_LED_PWM]);
#endif
}

static int8_t HAL_UpdatePwmDuty(uint16_t front_w,uint16_t rear_w)
{
#ifdef FRONT_LED_PWM
	if(front_w > 0){
		light_pwm[FRONT_LED_PWM].duty_cycle = front_w;
		pwm_duty_cycle(&light_pwm[FRONT_LED_PWM]);
	}
#endif
#ifdef REAR_LED_PWM
	if(rear_w > 0){
		light_pwm[REAR_LED_PWM].duty_cycle = rear_w;
		pwm_duty_cycle(&light_pwm[REAR_LED_PWM]);
	}
#endif
	return 0;
}


void HAL_Lighting_Nightlight(uint8_t front_w, uint8_t rear_w)
{   
	if(front_w > 100 || rear_w > 100){
		return;
	}
	
	if(front_w == 0 && rear_w == 0){
		HAL_PwmForLight_Deinit();
		return;
	}
	
	if(front_w == 0){
		pwm_disable(&light_pwm[FRONT_LED_PWM]);
	}else{
        pwm_enable(&light_pwm[FRONT_LED_PWM]);
    }
	
	if(rear_w == 0){
		pwm_disable(&light_pwm[REAR_LED_PWM]);
	}else{
        pwm_enable(&light_pwm[REAR_LED_PWM]);
    }
	
	HAL_UpdatePwmDuty(front_w*PWM_FREQUENCY/100, rear_w*PWM_FREQUENCY/100);
}

void HAL_Lighting_OFF(void)
{
	HAL_Lighting_Nightlight(0, 0);
}

void HAL_Lighting_ON(void)
{	
	if(mLightManager==NULL){
		APP_DBG_PRINTF("%s mLightManager=NULL !!!\r\n",__func__);	
		return;
	}
	
	HAL_Lighting_Nightlight(100, 100);
}

void HAL_Lighting_Default(void)
{	
	if(mLightManager==NULL){
		APP_DBG_PRINTF("%s mLightManager=NULL !!!\r\n",__func__);	
		return;
	}
	
#if USE_LIGHT_FOR_SKYIOT
	#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
	mLightManager->ctp = DEFAULT_SUNLIGHT_CTP_VALUE;
	mLightManager->bri = DEFAULT_LIGHT_BRI_VALUE;
	HAL_Lighting_Sunlight( mLightManager->ctp, mLightManager->bri );
	
	#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
	if(mLightManager->ctp){
		mLightManager->ctp = DEFAULT_SUNLIGHT_CTP_VALUE;
		mLightManager->bri = DEFAULT_LIGHT_BRI_VALUE;
		HAL_Lighting_Sunlight( mLightManager->ctp, mLightManager->bri );
	}else{
		mLightManager->bri = DEFAULT_LIGHT_BRI_VALUE;
		mLightManager->hue = DEFAULT_LIGHT_HUE_VALUE;
		mLightManager->sat = DEFAULT_LIGHT_SAT_VALUE;
		 HAL_Lighting_Color( mLightManager->hue, mLightManager->sat, mLightManager->bri );	  
	}
	
	#endif
#endif

	   
}


bool HAL_Lighting_Init(SkyLightManager *manager)
{
	if( manager == NULL){
		return false;
	}
		
	mLightManager = manager;
	HAL_PwmForLight_Init();
	if(mLightManager->light_newmode == UNREACT_MODE_M){
		mLightManager->front_led |= LED1_FLAG_STATUS_N;
	}else if(mLightManager->light_newmode == UNREACT_MODE_S){
		mLightManager->rear_led |= LED2_FLAG_STATUS_N;
	}else if(mLightManager->light_newmode == UNREACT_MODE_A){
		mLightManager->front_led |= LED1_FLAG_STATUS_N;
		mLightManager->rear_led |= LED2_FLAG_STATUS_N;
	}

	return true;	
}

void HAL_Light_Dlps_Control(bool isenter)
{
	if(isenter){
        if(SkyBleMesh_Batt_Station() == BATT_NORMAL){
            switch((uint8_t)mLightManager->light_newmode)
            {
                case RELEASE_MODE:
                case REACT_MODE_M:
                case REACT_MODE_S:
                case REACT_MODE_A:
                case UNREACT_MODE_N:
                    Pad_Config(LED_FRONT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
                    Pad_Config(LED_REAR, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
                    break;
                
                case UNREACT_MODE_M:
                    Pad_Config(LED_FRONT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
                    Pad_Config(LED_REAR, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
                    break;
                
                case UNREACT_MODE_S:
                    Pad_Config(LED_FRONT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
                    Pad_Config(LED_REAR, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
                    break;
                
                case UNREACT_MODE_A:
                    Pad_Config(LED_FRONT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
                    Pad_Config(LED_REAR, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
                    break;
            }
        }else{
            Pad_Config(LED_FRONT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
            Pad_Config(LED_REAR, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
        }
	}else{
        HAL_PwmForLight_Init();
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
	switch((uint8_t)mLightManager->led_mode)
	{
		case FAST_BLINK:
			if(mLightManager->led_timercnt > 0){
				if(m_step_cnt%4 == 1){
					if(mLightManager->led_timercnt%2 == 1){
						HAL_UpdatePwmDuty(0, PWM_FREQUENCY);
		                pwm_enable(&light_pwm[REAR_LED_PWM]);
					}else{
						// HAL_UpdatePwmDuty(0, 1);
		                pwm_disable(&light_pwm[REAR_LED_PWM]);
					}
					mLightManager->led_timercnt--;
				}
			}else{
				m_step_cnt = 0;
                if(mLightManager->light_newmode == UNREACT_MODE_N ||
                   mLightManager->light_newmode == UNREACT_MODE_M ||
                   mLightManager->light_newmode == UNREACT_MODE_S ||
                   mLightManager->light_newmode == UNREACT_MODE_A){
                    control_mode_switch(true);
                }
				mLightManager->led_mode = UNKOWN;
				Led_Relay_tmr_ctrl_dlps(true);
				plt_timer_delete(LEDCtrl_timer, 0);
			}
			break;
			
		case SLOW_BLINK:
			if(mLightManager->led_timercnt > 0){
				if(m_step_cnt%10 == 1){
					if(mLightManager->led_timercnt%2 == 1){
						HAL_UpdatePwmDuty(0, PWM_FREQUENCY);
						pwm_enable(&light_pwm[REAR_LED_PWM]);
					}else{
						// HAL_UpdatePwmDuty(0, 1);
		                pwm_disable(&light_pwm[REAR_LED_PWM]);
					}
					mLightManager->led_timercnt--;
				}
			}else{
				m_step_cnt = 0;
				mLightManager->led_mode = UNKOWN;
				Led_Relay_tmr_ctrl_dlps(true);
				plt_timer_delete(LEDCtrl_timer, 0);
			}
			break;
			
		case MODE_BLINK:
			if(mLightManager->led_timercnt > 0){
				if(m_step_cnt%4 == 1){
//                    APP_DBG_PRINTF("mLightManager->led_timercnt:%d\r\n", mLightManager->led_timercnt);
                    if(mLightManager->led_timercnt%2 == 1){
                        switch((uint8_t)mLightManager->light_newmode)
                        {
                            case REACT_MODE_M:
                            case UNREACT_MODE_M:
                                HAL_UpdatePwmDuty(PWM_FREQUENCY, 0);
				                pwm_enable(&light_pwm[FRONT_LED_PWM]);
				                pwm_disable(&light_pwm[REAR_LED_PWM]);
                                break;
                            
                            case REACT_MODE_S:
                            case UNREACT_MODE_S:
                                HAL_UpdatePwmDuty(0, PWM_FREQUENCY);
				                pwm_enable(&light_pwm[REAR_LED_PWM]);
				                pwm_disable(&light_pwm[FRONT_LED_PWM]);
                                break;
                            
                            case REACT_MODE_A:
                            case UNREACT_MODE_A:
                                HAL_UpdatePwmDuty(PWM_FREQUENCY, PWM_FREQUENCY);
				                pwm_enable(&light_pwm[FRONT_LED_PWM]);
				                pwm_enable(&light_pwm[REAR_LED_PWM]);
                                break;
                            default:
                                break;
                        }
                    }else{
                        // HAL_UpdatePwmDuty(1, 1);
		                pwm_disable(&light_pwm[FRONT_LED_PWM]);
		                pwm_disable(&light_pwm[REAR_LED_PWM]);
                    }
					mLightManager->led_timercnt--;
				}
			}else{
				m_step_cnt = 0;

				if((mLightManager->light_newmode == UNREACT_MODE_N\
                   || mLightManager->light_newmode == UNREACT_MODE_M\
                   || mLightManager->light_newmode == UNREACT_MODE_S\
                   || mLightManager->light_newmode == UNREACT_MODE_A)){		    // 常亮模式，直接控制
                    mLightManager->led_mode = UNKOWN;
                    Led_Relay_tmr_ctrl_dlps(true);
                    plt_timer_delete(LEDCtrl_timer, 0);
                    
				}else if((mLightManager->light_newmode == REACT_MODE_M\
                       || mLightManager->light_newmode == REACT_MODE_S\
                       || mLightManager->light_newmode == REACT_MODE_A)){	    // 感应模式，led_timercnt为亮灯时长（time*50）
					mLightManager->led_mode = SHORT_BRIGHT;
					mLightManager->led_timercnt = 20*5; // 50*5;
//                    APP_DBG_PRINTF("mLightManager->led_timercnt:%d\r\n", mLightManager->led_timercnt);
				}
			}
			break;
		
		case SHORT_BRIGHT:
			if(mLightManager->led_timercnt > 0){
                mLightManager->led_timercnt--;
            }else{
                m_step_cnt = 0;
                pwm_disable(&light_pwm[FRONT_LED_PWM]);
                pwm_disable(&light_pwm[REAR_LED_PWM]);
				mLightManager->led_mode = UNKOWN;
				Led_Relay_tmr_ctrl_dlps(true);
				plt_timer_delete(LEDCtrl_timer, 0);
            }
			break;
		
		case LONG_BRIGHT:
            m_step_cnt = 0;
            mLightManager->led_mode = UNKOWN;
            Led_Relay_tmr_ctrl_dlps(true);
            plt_timer_delete(LEDCtrl_timer, 0);
            break;
	}
	m_step_cnt++;
}


void control_mode_switch(bool blink)
{
    if(blink){
        switch((uint8_t)mLightManager->light_newmode)
        {
            case UNREACT_MODE_N:
                pwm_disable(&light_pwm[FRONT_LED_PWM]);
                pwm_disable(&light_pwm[REAR_LED_PWM]);
                break;
            
            case REACT_MODE_M:
            case UNREACT_MODE_M:
                pwm_enable(&light_pwm[FRONT_LED_PWM]);
                pwm_disable(&light_pwm[REAR_LED_PWM]);
                HAL_UpdatePwmDuty(PWM_FREQUENCY, 0);
                break;
            
            case REACT_MODE_S:
            case UNREACT_MODE_S:
                pwm_disable(&light_pwm[FRONT_LED_PWM]);
                pwm_enable(&light_pwm[REAR_LED_PWM]);
                HAL_UpdatePwmDuty(0, PWM_FREQUENCY);
                break;
            
            case REACT_MODE_A:        
            case UNREACT_MODE_A:
                pwm_enable(&light_pwm[FRONT_LED_PWM]);
                pwm_enable(&light_pwm[REAR_LED_PWM]);
                HAL_UpdatePwmDuty(PWM_FREQUENCY, PWM_FREQUENCY);
                break;
            
            default:
                break;
        }    
    }else{
        if(mLightManager->light_newmode == mLightManager->light_oldmode && 
           ((mLightManager->light_newmode == UNREACT_MODE_N\
           || mLightManager->light_newmode == UNREACT_MODE_M\
           || mLightManager->light_newmode == UNREACT_MODE_S\
           || mLightManager->light_newmode == UNREACT_MODE_A))){
            return;
        }
        
        switch((uint8_t)mLightManager->light_newmode)
        {
            case REACT_MODE_M:
                pwm_enable(&light_pwm[FRONT_LED_PWM]);
                pwm_disable(&light_pwm[REAR_LED_PWM]);
                for(int i=1;i<=20;i++){
                    HAL_UpdatePwmDuty(PWM_FREQUENCY*i/20, 0);
                    os_delay(2);
                }
                break;
                
            case REACT_MODE_S:
                pwm_enable(&light_pwm[REAR_LED_PWM]);
                pwm_disable(&light_pwm[FRONT_LED_PWM]);
                for(int i=1;i<=20;i++){
                    HAL_UpdatePwmDuty(0, PWM_FREQUENCY*i/20);
                    os_delay(2);
                }
                break;
           
           case REACT_MODE_A:  
                pwm_enable(&light_pwm[FRONT_LED_PWM]);
                pwm_enable(&light_pwm[REAR_LED_PWM]);
                for(int i=1;i<=20;i++){
                    HAL_UpdatePwmDuty(PWM_FREQUENCY*i/20, PWM_FREQUENCY*i/20);
                    os_delay(2);
                }
                break;
        
            case UNREACT_MODE_N:
                if(mLightManager->led_timercnt > 0\
                  || mLightManager->light_oldmode == UNREACT_MODE_M\
                  || mLightManager->light_oldmode == UNREACT_MODE_S\
                  || mLightManager->light_oldmode == UNREACT_MODE_A){
                    if(mLightManager->led_timercnt > 0){
                        m_step_cnt = 0;
                        mLightManager->led_timercnt = 0;
                        mLightManager->led_mode = UNKOWN;
                        Led_Relay_tmr_ctrl_dlps(true);
                        plt_timer_delete(LEDCtrl_timer, 0);
                    }
                    
                    if(mLightManager->light_oldmode == REACT_MODE_M ||
                       mLightManager->light_oldmode == UNREACT_MODE_M)
                    {
                        pwm_enable(&light_pwm[FRONT_LED_PWM]);
                        for(int i=1;i<=20;i++){
                            HAL_UpdatePwmDuty(PWM_FREQUENCY*(20-i)/20, 0);
                            os_delay(2);
                        }
                        pwm_disable(&light_pwm[FRONT_LED_PWM]);
                    }
                    else if(mLightManager->light_oldmode == REACT_MODE_S ||
                            mLightManager->light_oldmode == UNREACT_MODE_S)
                    {
                        pwm_enable(&light_pwm[REAR_LED_PWM]);
                        for(int i=1;i<=20;i++){
                            HAL_UpdatePwmDuty(0, PWM_FREQUENCY*(20-i)/20);
                            os_delay(2);
                        }
                        pwm_disable(&light_pwm[REAR_LED_PWM]);
                    }
                    else if(mLightManager->light_oldmode == REACT_MODE_A ||
                            mLightManager->light_oldmode == UNREACT_MODE_A)
                    {
                        pwm_enable(&light_pwm[FRONT_LED_PWM]);
                        pwm_enable(&light_pwm[REAR_LED_PWM]);
                        for(int i=1;i<=20;i++){
                            HAL_UpdatePwmDuty(PWM_FREQUENCY*(20-i)/20, PWM_FREQUENCY*(20-i)/20);
                            os_delay(2);
                        }
                        pwm_disable(&light_pwm[FRONT_LED_PWM]);
                        pwm_disable(&light_pwm[REAR_LED_PWM]);
                    }
                }
                break;
            
            
            
            case UNREACT_MODE_M:
                if(mLightManager->led_timercnt > 0\
                  || mLightManager->light_oldmode == UNREACT_MODE_M\
                  || mLightManager->light_oldmode == UNREACT_MODE_S\
                  || mLightManager->light_oldmode == UNREACT_MODE_A){
                    if(mLightManager->led_timercnt > 0){
                        m_step_cnt = 0;
                        mLightManager->led_timercnt = 0;
                        mLightManager->led_mode = UNKOWN;
                        Led_Relay_tmr_ctrl_dlps(true);
                        plt_timer_delete(LEDCtrl_timer, 0);
                    }
                    
                    if(mLightManager->light_oldmode == REACT_MODE_M){
                        return;
                    }
                    else if(mLightManager->light_oldmode == REACT_MODE_S ||
                            mLightManager->light_oldmode == UNREACT_MODE_S)
                    {
                        pwm_enable(&light_pwm[FRONT_LED_PWM]);
                        pwm_enable(&light_pwm[REAR_LED_PWM]);
                        for(int i=1;i<=20;i++){
                            HAL_UpdatePwmDuty(PWM_FREQUENCY*i/20, PWM_FREQUENCY*(20-i)/20);
                            os_delay(2);
                        }
                        pwm_disable(&light_pwm[REAR_LED_PWM]);
                    }
                    else if(mLightManager->light_oldmode == REACT_MODE_A ||
                            mLightManager->light_oldmode == UNREACT_MODE_A)
                    {
                        pwm_enable(&light_pwm[REAR_LED_PWM]);
                        for(int i=1;i<=20;i++){
                            HAL_UpdatePwmDuty(0, PWM_FREQUENCY*(20-i)/20);
                            os_delay(2);
                        }
                        pwm_disable(&light_pwm[REAR_LED_PWM]);
                    }
                }else{
                    pwm_enable(&light_pwm[FRONT_LED_PWM]);
                    for(int i=1;i<=20;i++){
                        HAL_UpdatePwmDuty(PWM_FREQUENCY*i/20, 0);
                        os_delay(2);
                    }
                }
                break;
            
            
            
            case UNREACT_MODE_S:
               if(mLightManager->led_timercnt > 0\
                  || mLightManager->light_oldmode == UNREACT_MODE_M\
                  || mLightManager->light_oldmode == UNREACT_MODE_S\
                  || mLightManager->light_oldmode == UNREACT_MODE_A){
                    if(mLightManager->led_timercnt > 0){
                        m_step_cnt = 0;
                        mLightManager->led_timercnt = 0;
                        mLightManager->led_mode = UNKOWN;
                        Led_Relay_tmr_ctrl_dlps(true);
                        plt_timer_delete(LEDCtrl_timer, 0);
                    }
                    
                    if(mLightManager->light_oldmode == REACT_MODE_S){
                        return;
                    }
                    else if(mLightManager->light_oldmode == REACT_MODE_M ||
                            mLightManager->light_oldmode == UNREACT_MODE_M)
                    {
                        pwm_enable(&light_pwm[FRONT_LED_PWM]);
                        pwm_enable(&light_pwm[REAR_LED_PWM]);
                        for(int i=1;i<=20;i++){
                            HAL_UpdatePwmDuty(PWM_FREQUENCY*(20-i)/20, PWM_FREQUENCY*i/20);
                            os_delay(2);
                        }
                        pwm_disable(&light_pwm[FRONT_LED_PWM]);
                    }
                    else if(mLightManager->light_oldmode == REACT_MODE_A ||
                            mLightManager->light_oldmode == UNREACT_MODE_A)
                    {
                        pwm_enable(&light_pwm[FRONT_LED_PWM]);
                        for(int i=1;i<=20;i++){
                            HAL_UpdatePwmDuty(PWM_FREQUENCY*(20-i)/20, 0);
                            os_delay(2);
                        }
                        pwm_disable(&light_pwm[FRONT_LED_PWM]);
                    }
                }else{
                    pwm_enable(&light_pwm[REAR_LED_PWM]);
                    for(int i=1;i<=20;i++){
                        HAL_UpdatePwmDuty(0, PWM_FREQUENCY*i/20);
                        os_delay(2);
                    }
                }
                break;
            
            
            
            case UNREACT_MODE_A:
               if(mLightManager->led_timercnt > 0\
                  || mLightManager->light_oldmode == UNREACT_MODE_M\
                  || mLightManager->light_oldmode == UNREACT_MODE_S\
                  || mLightManager->light_oldmode == UNREACT_MODE_A){
                    if(mLightManager->led_timercnt > 0){
                        m_step_cnt = 0;
                        mLightManager->led_timercnt = 0;
                        mLightManager->led_mode = UNKOWN;
                        Led_Relay_tmr_ctrl_dlps(true);
                        plt_timer_delete(LEDCtrl_timer, 0);
                    }
                    
                    if(mLightManager->light_oldmode == REACT_MODE_A){
                        return;
                    }
                    else if(mLightManager->light_oldmode == REACT_MODE_M ||
                            mLightManager->light_oldmode == UNREACT_MODE_M)
                    {
                        pwm_enable(&light_pwm[REAR_LED_PWM]);
                        for(int i=1;i<=20;i++){
                            HAL_UpdatePwmDuty(0, PWM_FREQUENCY*i/20);
                            os_delay(2);
                        }
                    }
                    else if(mLightManager->light_oldmode == REACT_MODE_S ||
                            mLightManager->light_oldmode == UNREACT_MODE_S)
                    {
                        pwm_enable(&light_pwm[FRONT_LED_PWM]);
                        for(int i=1;i<=20;i++){
                            HAL_UpdatePwmDuty(PWM_FREQUENCY*i/20, 0);
                            os_delay(2);
                        }
                    }
                }else{
                    pwm_enable(&light_pwm[FRONT_LED_PWM]);
                    pwm_enable(&light_pwm[REAR_LED_PWM]);
                    for(int i=1;i<=20;i++){
                        HAL_UpdatePwmDuty(PWM_FREQUENCY*i/20, PWM_FREQUENCY*i/20);
                        os_delay(2);
                    }
                }
                break;
            
            default:
                break;
        }    
        
    }
    APP_DBG_PRINTF("TIM_GetPWMOutputStatus:%d, %d", TIM_GetPWMOutputStatus(TIM2), TIM_GetPWMOutputStatus(TIM3));// 读取当前定时器的PWM电平（高低）
}


static void enable_pwm(void)
{
    switch((uint8_t)mLightManager->light_newmode)
    {
        case UNREACT_MODE_N:
            pwm_disable(&light_pwm[FRONT_LED_PWM]);
            pwm_disable(&light_pwm[REAR_LED_PWM]);
            break;
        
        case REACT_MODE_M:
        case UNREACT_MODE_M:
            pwm_enable(&light_pwm[FRONT_LED_PWM]);
            pwm_disable(&light_pwm[REAR_LED_PWM]);
            break;
        
        case REACT_MODE_S:
        case UNREACT_MODE_S:
            pwm_disable(&light_pwm[FRONT_LED_PWM]);
            pwm_enable(&light_pwm[REAR_LED_PWM]);
            break;
        
        case REACT_MODE_A:        
        case UNREACT_MODE_A:
            pwm_enable(&light_pwm[FRONT_LED_PWM]);
            pwm_enable(&light_pwm[REAR_LED_PWM]);
            break;
        
        default:
            break;
    }  
    
    if(mLightManager->led_mode == FAST_BLINK ||
       mLightManager->led_mode == SLOW_BLINK){
        pwm_enable(&light_pwm[REAR_LED_PWM]);
    }
}


void Start_LED_Timer(void)
{
   // enable_pwm();
	if(LEDCtrl_timer == NULL){
		LEDCtrl_timer = plt_timer_create("ledtest", 50, true, 0, SkyLed_Timeout_cb); 
		if(LEDCtrl_timer != NULL){
//			APP_DBG_PRINTF("singal led mode:%d\r\n", mLightManager->led_mode);
			Led_Relay_tmr_ctrl_dlps(false);
			plt_timer_start(LEDCtrl_timer, 0);
		}
	}else{
        m_step_cnt = 0;
    }
}


void Delete_LED_Timer(void)
{
    plt_timer_delete(LEDCtrl_timer, 0);
    Led_Relay_tmr_ctrl_dlps(true);
}


#endif
