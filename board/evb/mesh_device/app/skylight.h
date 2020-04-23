

#ifndef _SKYLIGHT_H_
#define _SKYLIGHT_H_
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>

#if 0
#define USE_LIGHT_FOR_SKYIOT  1


#define SKY_LIGHT_BELT_TYPE         1   // 灯带
#define SKY_LIGHT_BULB_TYPE         2   // 灯泡
#define SKY_LIGHT_BULB_RGBWY_TYPE   3   // RGBWY灯泡

#define SKY_LIGHT_TYPE   SKY_LIGHT_BULB_RGBWY_TYPE  





//PWM时钟选择为16M时，周期(endvalue)计算方法为，
//endvalue = 16MHZ /目标值，
//frequency = 16MHZ / PWM_FREQUENCY = 6274.5HZ
#define 	PWM_FREQUENCY    	2552
#define 	PWM_DUTY_INIT		0
#define 	PWM_DUTY_STEP 	    10     // no use  

#define DEFAULT_LIGHT_BRI_VALUE    (255)
#define DEFAULT_SUNLIGHT_CTP_VALUE (3000)
#define DEFAULT_LIGHT_HUE_VALUE    (170)
#define DEFAULT_LIGHT_SAT_VALUE    (255)

#define BLEMESH_REPORT_FLAG_SWT (0x1)
#define BLEMESH_REPORT_FLAG_BRI (0x2)
#define BLEMESH_REPORT_FLAG_CTP (0x4)
#define BLEMESH_REPORT_FLAG_HUE (0x8)
#define BLEMESH_REPORT_FLAG_SAT (0x10)

	

typedef struct {
	//灯泡状态字段
	#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
	uint8_t status; /* 0: off, 1: on */
	int32_t bri;	/* 日光模式亮度 */		
	int32_t ctp;	/* 日光模式色温 */
	
	#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
	uint8_t status; /* 0: off, 1: on */
	int32_t bri;	/* 亮度 */		
	int32_t ctp;	/* 日光模式色温 */
	int32_t hue;	/* 色度 */		
	int32_t sat;	/* 饱和度 */
	
	#endif
	
	// uint32_t report_flag;	
}SkyLightManager;





void HAL_Lighting_Color(int hue, int sat, int bri);
void HAL_Lighting_Sunlight(int ctp, int ctp_bri);
void HAL_Lighting_OFF(void);
void HAL_Lighting_ON( void );
void HAL_Lighting_Default(void);

bool HAL_Lighting_Init(SkyLightManager *manager);

#endif

#endif //



