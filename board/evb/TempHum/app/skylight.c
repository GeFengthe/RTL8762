

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
#include "app_task.h"


plt_timer_t LEDCtrl_timer = NULL;

// #define APP_DBG_PRINTF(fmt, ...)
//static SkyLightManager *mLightManager=NULL;

#if 1

#define PIN_INVALID       0xff

#define LED_WHITE					P4_1			// LED_1(PWM) ��
#define LED_YELLOW					P0_6			// LED_2(PWM) ��

#define LEDG_Pin            GPIO_GetPin(LED_WHITE)
#define LEDR_Pin            GPIO_GetPin(LED_YELLOW)
#define LEDPOWER_CLOSE      ((BitAction)0)
#define LEDPOWER_OPEN       ((BitAction)1)



uint32_t time;
uint8_t lightcount = 0;
typedef struct {
    LED_MODE_e mode;
    uint32_t linktime;
    uint32_t linkcnt;
}LightMonitor;

static LightMonitor mLightMonitor={
    LED_MODE_UNKOWN,
    0,
    0   
};
LED_MODE_e led_mode;



void HAL_Light_Init(void)
{
    Pad_Config(LED_WHITE,PAD_PINMUX_MODE,PAD_IS_PWRON,PAD_PULL_UP,PAD_OUT_ENABLE,PAD_OUT_LOW);
    Pinmux_Config(LED_WHITE,DWGPIO);

    
    Pad_Config(LED_YELLOW,PAD_PINMUX_MODE,PAD_IS_PWRON,PAD_PULL_UP,PAD_OUT_ENABLE,PAD_OUT_LOW);
    Pinmux_Config(LED_YELLOW,DWGPIO);
    
    RCC_PeriphClockCmd(APBPeriph_GPIO,APBPeriph_GPIO_CLOCK,ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStruct;
    
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = LEDR_Pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = LEDG_Pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd = DISABLE;
    GPIO_Init(&GPIO_InitStruct);

    GPIO_WriteBit(LEDR_Pin,LEDPOWER_CLOSE);
    GPIO_WriteBit(LEDG_Pin,LEDPOWER_CLOSE);
}


void HAL_Lighting_OFF(void)
{
    GPIO_WriteBit(LEDR_Pin,LEDPOWER_CLOSE);
}
void HAL_Lighting_ON( void )
{
   GPIO_WriteBit(LEDR_Pin,LEDPOWER_OPEN);
}
/*
** PWM
*/
void SkyLed_Ctrl(LED_MODE_e mode,uint8_t cnt)
{
    mLightMonitor.mode = mode;
    mLightMonitor.linkcnt = cnt;
    Start_LED_Timer();
}




void SkyLed_Timeout_cb_handel(void *timer)
{
    switch(mLightMonitor.mode)
    {
        case LED_MODE_SLOW_BLINK:
        {
            mLightMonitor.linktime +=100;
            if(mLightMonitor.linktime >=LED_SLOW_BLINK_PERIOD)
            {
                if((mLightMonitor.linkcnt &0x01) == 1)
                {
                    GPIO_WriteBit(LEDG_Pin,LEDPOWER_CLOSE);
                }else{
                    GPIO_WriteBit(LEDG_Pin,LEDPOWER_OPEN);
                }
                mLightMonitor.linktime = 0;
                mLightMonitor.linkcnt--;
                if(mLightMonitor.linkcnt==0)
                {
                    mLightMonitor.mode = LED_MODE_UNKOWN;
                    Delete_LED_Timer();
                }
                DBG_DIRECT("----------m_linkcnt=%d---------\r\n",mLightMonitor.linkcnt);
            }
            break;
        }
        case LED_MODE_FAST_BLINK:
        {
            mLightMonitor.linktime +=100;
            if(mLightMonitor.linktime >=LED_FAST_BLINK_PERIOD)
            {
                if((mLightMonitor.linkcnt & 0x01) == 1)
                {
                    GPIO_WriteBit(LEDG_Pin,LEDPOWER_CLOSE);
                }else{
                    GPIO_WriteBit(LEDG_Pin,LEDPOWER_OPEN);
                }
                mLightMonitor.linktime = 0;
                mLightMonitor.linkcnt--;
                if(mLightMonitor.linkcnt==0)
                {
                    mLightMonitor.mode = LED_MODE_UNKOWN;
                    Delete_LED_Timer();
                }
                DBG_DIRECT("----------m_linkcnt=%d---------\r\n",mLightMonitor.linkcnt);
            }
            break;
        }
        case LED_MODE_NORMAL_BRIGHT:
            if(mLightMonitor.linkcnt>0)
            {
                GPIO_WriteBit(LEDG_Pin,~(GPIO_ReadOutputDataBit(LEDG_Pin)));
                mLightMonitor.linkcnt--;
            }else{
                GPIO_WriteBit(LEDG_Pin,~(GPIO_ReadOutputDataBit(LEDG_Pin)));
                mLightMonitor.mode = LED_MODE_UNKOWN;
                Delete_LED_Timer();
            }
            break;
        default:
            break;
    }
}
void SkyLed_Timeout_cb(void * timer)
{      
    T_IO_MSG msg;
    msg.type = IO_MSG_TYPE_TIMER;
    msg.subtype = test_light_TIMEOUT;
    app_send_msg_to_apptask(&msg);
}

void Start_LED_Timer(void)
{
	if(LEDCtrl_timer == NULL){
		LEDCtrl_timer = plt_timer_create("ledtest", 100, true, 0, SkyLed_Timeout_cb); 
		if(LEDCtrl_timer != NULL){
			Led_Relay_tmr_ctrl_dlps(false);
			plt_timer_start(LEDCtrl_timer, 0);
		}
	}
}

void Delete_LED_Timer(void)
{
	if(LEDCtrl_timer){
   		plt_timer_delete(LEDCtrl_timer, 0);
		LEDCtrl_timer = NULL;
	}
    Led_Relay_tmr_ctrl_dlps(true);
}

void HAL_Light_Dlps_Control(bool isenter)
{
    if(isenter)
    {
        Pad_Config(LED_WHITE, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
        Pad_Config(LED_YELLOW, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    }else{
        Pad_Config(LED_WHITE, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
        Pad_Config(LED_YELLOW, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    }
}


#endif
