

#ifndef _SKYLIGHT_H_
#define _SKYLIGHT_H_
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>

#if 1
#define USE_LIGHT_FOR_SKYIOT  1


#define SKY_LIGHT_NIGHT_TYPE		1
#define SKY_LIGHT_TYPE   SKY_LIGHT_NIGHT_TYPE  


#define LED_FRONT					P2_3			// LED_1(PWM)
#define LED_REAR					P2_2			// LED_2(PWM)

//endvalue = 20MHZ 
//frequency = 20MHZ / PWM_FREQUENCY = 4KHZ
#define PWM_FREQUENCY    			(5000)
#define PWM_DUTY_INIT				0


#define LED_OPEN					1
#define LED_CLOSE					0


// 移到 app_skyiot_server.h	
#define BLEMESH_REPORT_FLAG_SW1 	(0x01)
#define BLEMESH_REPORT_FLAG_SW2 	(0x02)
#define BLEMESH_REPORT_FLAG_BAT 	(0x04)
#define BLEMESH_REPORT_FLAG_INF 	(0x08)
#define BLEMESH_REPORT_FLAG_AMB		(0x10)
#define BLEMESH_REPORT_FLAG_MOD		(0x20)
#define BLEMESH_REPORT_FLAG_TIM		(0x40)


#define FRONT_LED_PWM		0
#define REAR_LED_PWM		1
#define TOTALPWMNUMBER     	2

#define SKY_LED1_STATUS		0
#define SKY_LED2_STATUS		1



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
	LED_MODE_UNKOWN = 0,
	LED_MODE_FAST_BLINK,   // 配网成功
	LED_MODE_SLOW_BLINK,   // 进入配网
	LED_MODE_MODE_BLINK,   // 本地切换至感应模式
	LED_MODE_DELAY_BRIGHT, // 延长亮灯时间
	LED_MODE_NORMAL_BRIGHT,
}LED_MODE_e;


#define LIGHT_DEFAULT_TIME			20
#define SKYIOT_INF_HAVE_BODY     (0)
#define SKYIOT_INF_NO_BODY       (1)
#define SKYIOT_AMBIENT_DARK      (0)
#define SKYIOT_AMBIENT_BRIGHT    (1)
#define SKYIOT_AMBIENT_LIMITVOL	 (160)		    // 20lux limit val

typedef enum{
	SKYIOT_INVALID_RELEASE = 0xFF,
	SKYIOT_FIRST_RELEASE   = 0x5A,         // 出厂标志
	SKYIOT_ACTIVE_RELEASE  = 0xA5,         // 出厂已激活
}SKYIOT_RELEASE_e;



typedef struct {
	//灯泡状态字段
#if  (SKY_LIGHT_TYPE==SKY_LIGHT_NIGHT_TYPE)
	uint8_t  statu[ 2 ];  // TOTALPWMNUMBER
	// uint8_t  manset[ 2 ];  // TOTALPWMNUMBER
	uint8_t  batt;
	uint8_t  inf;      // 人感   0:有人 1:无人
	uint8_t  amb;      // 环境光 0:环境暗时感应           1:环境亮时感应
	uint32_t bri_time; // 感应亮灯时长
	NLIGHT_MODE_e mode;	

	uint8_t  ambstatu; // 当前环境光状态 0:当前环境暗              1:当前环境亮
#endif	
}SkyLightManager;



bool HAL_Lighting_Output_Statu(void);
bool HAL_Lighting_Influence_End(void);
void HAL_Lighting_OFF(void);
void HAL_Lighting_ON( void );
bool HAL_Lighting_Init(SkyLightManager *manager);
void HAL_Light_Dlps_Control(bool isenter);
void Start_LED_Timer(void);
void Delete_LED_Timer(void);
void SkyLed_LightEffective_CTL(bool blink, uint32_t mode, uint16_t blinkcnt);


#endif

#endif //



