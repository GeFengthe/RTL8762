

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
    UNKOWN_MODE = 0,    // UNKOWN_MODE
	RELEASE_MODE = 1,	// The first release
	REACT_MODE_M = 2,	// Master led
	REACT_MODE_S = 3,	// Slaver led
	REACT_MODE_A = 4,	// All led
	UNREACT_MODE_N = 5,	// no led(unreact)
	UNREACT_MODE_M = 6,	// Master led
	UNREACT_MODE_S = 7,	// Slaver led
	UNREACT_MODE_A = 8,	// All led
}LIGHT_MODE_e;
	
typedef enum{
	UNKOWN,
	FAST_BLINK,
	SLOW_BLINK,
	MODE_BLINK,
	SHORT_BRIGHT,
	LONG_BRIGHT,
}LED_MODE_e;

typedef struct {
	//灯泡状态字段
#if  (SKY_LIGHT_TYPE==SKY_LIGHT_NIGHT_TYPE)
	uint8_t  front_led;
//	uint8_t  frontled_c;// ctroal
//	uint8_t  frontled_cs;
//	uint8_t  frontled_ns;
	uint8_t  rear_led;
    uint8_t  change_flag;
	LED_MODE_e    led_mode;
	LIGHT_MODE_e  light_oldmode;
	LIGHT_MODE_e  light_newmode;
    uint32_t  led_timercnt;
	uint32_t  led_time;
#endif	
}SkyLightManager;




void HAL_Lighting_OFF(void);
void HAL_Lighting_ON( void );
bool HAL_Lighting_Init(SkyLightManager *manager);
void HAL_Light_Dlps_Control(bool isenter);
void Start_LED_Timer(void);
void control_mode_switch(bool blink);
void Delete_LED_Timer(void);

#endif

#endif //



