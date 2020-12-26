

#ifndef _SKYLIGHT_H_
#define _SKYLIGHT_H_
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>

#if 1




//endvalue = 20MHZ 
//frequency = 20MHZ / PWM_FREQUENCY = 4KHZ


#define LED_OPEN					1
#define LED_CLOSE					0

typedef enum{
	LED_MODE_UNKOWN = 0,
	LED_MODE_FAST_BLINK,   // 配网成功
	LED_MODE_SLOW_BLINK,   // 进入配网
	LED_MODE_NORMAL_BRIGHT, //正常模式
}LED_MODE_e;

#define LED_SLOW_BLINK_PERIOD 500
#define LED_FAST_BLINK_PERIOD 200

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
#if USE_DOOR_FOR_SKYIOT
    uint8_t alm;
    uint8_t stu;
    uint8_t bat;
#endif	
}SkyLightManager;



bool HAL_Lighting_Output_Statu(void);
bool HAL_Lighting_Influence_End(void);

bool HAL_Lighting_Init(SkyLightManager *manager);
void HAL_Light_Dlps_Control(bool isenter);
void Start_LED_Timer(void);
void Delete_LED_Timer(void);
void SkyLed_LightEffective_CTL(bool blink, uint32_t mode, uint16_t blinkcnt);
extern void HAL_Lighting_OFF(void);
extern void HAL_Lighting_ON( void );
extern void HAL_Light_Init(void);
extern void HAL_LightToggle(void);
extern void SkyLed_Timeout_cb_handel(void *timer);
extern void SkyLed_Ctrl(LED_MODE_e mode,uint8_t cnt);


#endif

#endif //



