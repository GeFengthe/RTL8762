

#ifndef _SKYLIGHT_H_
#define _SKYLIGHT_H_
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>

#if 0
#define USE_LIGHT_FOR_SKYIOT  1


#define SKY_LIGHT_BELT_TYPE         1   // �ƴ�
#define SKY_LIGHT_BULB_TYPE         2   // ����
#define SKY_LIGHT_BULB_RGBWY_TYPE   3   // RGBWY����

#define SKY_LIGHT_TYPE   SKY_LIGHT_BULB_RGBWY_TYPE  





//PWMʱ��ѡ��Ϊ16Mʱ������(endvalue)���㷽��Ϊ��
//endvalue = 16MHZ /Ŀ��ֵ��
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
	//����״̬�ֶ�
	#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
	uint8_t status; /* 0: off, 1: on */
	int32_t bri;	/* �չ�ģʽ���� */		
	int32_t ctp;	/* �չ�ģʽɫ�� */
	
	#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
	uint8_t status; /* 0: off, 1: on */
	int32_t bri;	/* ���� */		
	int32_t ctp;	/* �չ�ģʽɫ�� */
	int32_t hue;	/* ɫ�� */		
	int32_t sat;	/* ���Ͷ� */
	
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



