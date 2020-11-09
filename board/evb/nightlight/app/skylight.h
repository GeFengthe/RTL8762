

#ifndef _SKYLIGHT_H_
#define _SKYLIGHT_H_
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>

#if 1
#define USE_LIGHT_FOR_SKYIOT  1


#define SKY_LIGHT_BELT_TYPE         1   
#define SKY_LIGHT_BULB_TYPE         2   
#define SKY_LIGHT_BULB_RGBWY_TYPE   3   
#define SKY_LIGHT_NIGHT_TYPE		4

#define SKY_LIGHT_TYPE   SKY_LIGHT_NIGHT_TYPE  


#define LED_FRONT					P2_3			// LED_1(PWM)
#define LED_REAR					P2_2			// LED_2(PWM)

//endvalue = 20MHZ 
//frequency = 20MHZ / PWM_FREQUENCY = 4KHZ
#define PWM_FREQUENCY    			(5000)
#define PWM_DUTY_INIT				0


#define LIGHT_DEFAULT_TIME			20
#define LED_OPEN					1
#define LED_CLOSE					0


#define LED1_FLAG_STATUS_N			(0x01)	// 主灯当前状态
#define LED1_FLAG_COTROL			(0x02)	// 主灯控制标志
#define LED1_FLAG_STATUS_C          (0x04)	// 主灯控制状态
#define LED2_FLAG_STATUS_N			(0x01)	// 副灯当前状态
#define LED2_FLAG_COTROL			(0x02) 	// 副灯控制标志
#define LED2_FLAG_STATUS_C 			(0x04)	// 副灯控制状态

	
#define BLEMESH_REPORT_FLAG_SW1 	(0x01)
#define BLEMESH_REPORT_FLAG_SW2 	(0x02)
#define BLEMESH_REPORT_FLAG_BAT 	(0x04)
#define BLEMESH_REPORT_FLAG_INF0	(0x08)
#define BLEMESH_REPORT_FLAG_AMB		(0x10)
#define BLEMESH_REPORT_FLAG_MOD		(0x20)
#define BLEMESH_REPORT_FLAG_TIM		(0x40)
#define BLEMESH_REPORT_FLAG_INF1	(0x80)


typedef enum{
	NLIGHT_MANUAL_MOD = 0,    // 手动模式
	NLIGHT_REACT_LED1_MOD,    // 主灯
	NLIGHT_REACT_LED2_MOD,    // 副灯
	NLIGHT_REACT_LEDALL_MOD,  // 主副灯
}NLIGHT_MODE_e;
	

#define LED_BRIGHT_TMR_PERIOD   (50)  // ms
#define LED_FAST_BLINK_PERIOD   (200)  // ms
#define LED_SLOW_BLINK_PERIOD   (500)  // ms
#define LED_MODE_BLINK_PERIOD   (200)  // ms
#define LED_DELAY_BRIGHT_TIME   (5000)  // ms
typedef enum{
	LED_MODE_UNKOWN,
	LED_MODE_FAST_BLINK,   // 配网成功
	LED_MODE_SLOW_BLINK,   // 进入配网
	LED_MODE_MODE_BLINK,   // 本地切换至感应模式
	LED_MODE_DELAY_BRIGHT, // 延长亮灯时间
	LED_MODE_NORMAL_BRIGHT,
}LED_MODE_e;


typedef struct {
	//灯泡状态字段
#if  (SKY_LIGHT_TYPE==SKY_LIGHT_NIGHT_TYPE)
	uint8_t  statu[ 2 ];  // TOTALPWMNUMBER
	// uint8_t  manset[ 2 ];  // TOTALPWMNUMBER
	uint8_t  batt;
	uint8_t  inf;      // 人感
	uint8_t  amb;      // 环境光
	uint16_t bri_time; // 感应亮灯时长
	NLIGHT_MODE_e mode;	
#endif	
}SkyLightManager;




void HAL_Lighting_OFF(void);
void HAL_Lighting_ON( void );
bool HAL_Lighting_Init(SkyLightManager *manager);
void HAL_Light_Dlps_Control(bool isenter);
void Start_LED_Timer(void);
void Delete_LED_Timer(void);
void SkyLed_LightEffective_CTL(bool blink, LED_MODE_e blinkmode, uint16_t blinkcnt);

#endif

#endif //



