

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <math.h>
#include "skylight.h"


#define APP_DBG_PRINTF  MESH_APP_PRINT_DEBUG


#if 0
// PWM0--channel 0 -- P10 
// PWM1--channel 1 -- P11 
// PWM2--channel 2 -- P12 
// PWM3--channel 3 -- P13 
// PWM4--channel 4 -- P14 
// PWM5--channel 5 -- P15 

// represent channel and index
#if (SKY_LIGHT_TYPE == SKY_LIGHT_BELT_TYPE)
#define WARMWHITE_PWM      0   
#define COLDWHITE_PWM      1
// #define RED_PWM            2
// #define GREEN_PWM          3
// #define BLUE_PWM           4

#elif (SKY_LIGHT_TYPE == SKY_LIGHT_BULB_TYPE)
#define WARMWHITE_PWM      1   
#define COLDWHITE_PWM      0
// #define RED_PWM            2
// #define GREEN_PWM          3
// #define BLUE_PWM           4

#elif (SKY_LIGHT_TYPE == SKY_LIGHT_BULB_RGBWY_TYPE)
#define WARMWHITE_PWM      0   
#define COLDWHITE_PWM      1
#define RED_PWM            3
#define GREEN_PWM          4
#define BLUE_PWM           5

#endif

#define TOTALPWMNUMBER     6
PWM_DRV_DESC pwm[TOTALPWMNUMBER];


#define GAIN (10)
typedef struct
{
	uint8_t	r;
	uint8_t	g;
	uint8_t	b;
	uint8_t	cw;
	uint8_t	ww;
} Color_t;
static SkyLightManager *mLightManager=NULL;


/*
** PWM
*/
void HAL_PwmForLight_Init(void)
{
	rwip_prevent_sleep_set(BK_DRIVER_TIMER_ACTIVE);

	#ifdef WARMWHITE_PWM
	pwm[WARMWHITE_PWM].channel = WARMWHITE_PWM;	//  
	pwm[WARMWHITE_PWM].mode = (1 << 0) 	        // 
			  | (1 << 4);	            // select 16MHz
	pwm[WARMWHITE_PWM].pre_divid  = 1;
	pwm[WARMWHITE_PWM].end_value  = PWM_FREQUENCY;
	pwm[WARMWHITE_PWM].duty_cycle = PWM_DUTY_INIT ; 
	pwm[WARMWHITE_PWM].duty_step  = PWM_DUTY_STEP;
	pwm_init(&pwm[WARMWHITE_PWM]);
	#endif

	#ifdef COLDWHITE_PWM
	pwm[COLDWHITE_PWM].channel = COLDWHITE_PWM;	//  
	pwm[COLDWHITE_PWM].mode = (1 << 0) 	        // 
			  | (1 << 4);	            // select 16MHz
	pwm[COLDWHITE_PWM].pre_divid  = 1;
	pwm[COLDWHITE_PWM].end_value  = PWM_FREQUENCY;
	pwm[COLDWHITE_PWM].duty_cycle = PWM_DUTY_INIT ; 
	pwm[COLDWHITE_PWM].duty_step  = PWM_DUTY_STEP;
	pwm_init(&pwm[COLDWHITE_PWM]);
	#endif

	#ifdef RED_PWM
	pwm[RED_PWM].channel = RED_PWM;	//  
	pwm[RED_PWM].mode = (1 << 0) 	        // 
			  | (1 << 4);	            // select 16MHz
	pwm[RED_PWM].pre_divid  = 1;
	pwm[RED_PWM].end_value  = PWM_FREQUENCY;
	pwm[RED_PWM].duty_cycle = PWM_DUTY_INIT ; 
	pwm[RED_PWM].duty_step  = PWM_DUTY_STEP;
	pwm_init(&pwm[RED_PWM]);
	#endif

	#ifdef GREEN_PWM
	pwm[GREEN_PWM].channel = GREEN_PWM;	//  
	pwm[GREEN_PWM].mode = (1 << 0) 	        // 
			  | (1 << 4);	            // select 16MHz
	pwm[GREEN_PWM].pre_divid  = 1;
	pwm[GREEN_PWM].end_value  = PWM_FREQUENCY;
	pwm[GREEN_PWM].duty_cycle = PWM_DUTY_INIT ; 
	pwm[GREEN_PWM].duty_step  = PWM_DUTY_STEP;
	pwm_init(&pwm[GREEN_PWM]);
	#endif

	#ifdef BLUE_PWM
	pwm[BLUE_PWM].channel = BLUE_PWM;	//  
	pwm[BLUE_PWM].mode = (1 << 0) 	        // 
			  | (1 << 4);	            // select 16MHz
	pwm[BLUE_PWM].pre_divid  = 1;
	pwm[BLUE_PWM].end_value  = PWM_FREQUENCY;
	pwm[BLUE_PWM].duty_cycle = PWM_DUTY_INIT ; 
	pwm[BLUE_PWM].duty_step  = PWM_DUTY_STEP;
	pwm_init(&pwm[BLUE_PWM]);
	#endif

}

void HAL_PwmForLight_Deinit(void)
{
#ifdef WARMWHITE_PWM
    pwm_disable(WARMWHITE_PWM);
#endif
#ifdef COLDWHITE_PWM
    pwm_disable(COLDWHITE_PWM);
#endif
#ifdef RED_PWM
    pwm_disable(RED_PWM);
#endif
#ifdef GREEN_PWM
    pwm_disable(GREEN_PWM);
#endif
#ifdef BLUE_PWM
    pwm_disable(BLUE_PWM);
#endif

	rwip_prevent_sleep_clear(BK_DRIVER_TIMER_ACTIVE);
}

static int8_t HAL_UpdatePwmDuty(uint16_t r, uint16_t g, uint16_t b,uint16_t cw,uint16_t ww)
{
	// APP_DBG_PRINTF("HAL_UpdatePwmDuty ww=%d cw=%d\r\n",ww,cw);

	#ifdef WARMWHITE_PWM
	pwm[WARMWHITE_PWM].duty_cycle = ww;
	pwm_duty_cycle(&pwm[WARMWHITE_PWM]);
	#endif
	#ifdef COLDWHITE_PWM
	pwm[COLDWHITE_PWM].duty_cycle = cw;
	pwm_duty_cycle(&pwm[COLDWHITE_PWM]);
	#endif
	#ifdef RED_PWM
	pwm[RED_PWM].duty_cycle = r;
	pwm_duty_cycle(&pwm[RED_PWM]);
	#endif
	#ifdef GREEN_PWM
	pwm[GREEN_PWM].duty_cycle = g;
	pwm_duty_cycle(&pwm[GREEN_PWM]);
	#endif
	#ifdef BLUE_PWM
	pwm[BLUE_PWM].duty_cycle = b;
	pwm_duty_cycle(&pwm[BLUE_PWM]);
	#endif

	return 0;
}





/*
** 
*/
static Color_t HAL_HSVtoRGB(unsigned int h, unsigned int s, unsigned int v)
{
    // convert from HSV/HSB to RGB color
    // R,G,B from 0-255, H from 0-360, S,V from 0-100
    // ref http://colorizer.org/

    // The hue (H) of a color refers to which pure color it resembles
    // The saturation (S) of a color describes how white the color is
    // The value (V) of a color, also called its lightness, describes how dark the color is
    Color_t color;
    int tmp = 0;

    float RGB_min, RGB_max;
    RGB_max = v*2.55f;
    RGB_min = RGB_max*(100 - s)/ 100.0f;

	tmp = h / 60;
	int difs = h % 60; // factorial part of h

	// RGB adjustment amount by hue 
	float RGB_Adj = (RGB_max - RGB_min)*difs / 60.0f;

	switch (tmp) {
	case 0:
		color.r = (int)(RGB_max + 0.5f);
		color.g = (int)((RGB_min + RGB_Adj) + 0.5f);
		color.b = (int)(RGB_min + 0.5f);
		break;
	case 1:
		color.r = (int)((RGB_max - RGB_Adj) + 0.5f);
		color.g = (int)(RGB_max + 0.5f);
		color.b = (int)(RGB_min + 0.5f);
		break;
	case 2:
		color.r = (int)(RGB_min + 0.5f);
		color.g = (int)(RGB_max + 0.5f);
		color.b = (int)((RGB_min + RGB_Adj) + 0.5f);
		break;
	case 3:
		color.r = (int)(RGB_min + 0.5f);
		color.g = (int)((RGB_max - RGB_Adj) + 0.5f);
		color.b = (int)(RGB_max + 0.5f);
		break;
	case 4:
		color.r = (int)((RGB_min + RGB_Adj) + 0.5f);
		color.g = (int)(RGB_min + 0.5f);
		color.b = (int)(RGB_max + 0.5f);
		break;
	default:		// case 5:
		color.r = (int)(RGB_max + 0.5f);
		color.g = (int)(RGB_min + 0.5f);
		color.b = (int)((RGB_max - RGB_Adj) + 0.5f);
		break;
	}        
	return color;
}

void HAL_Lighting_Color(int hue, int sat, int bri)
{
	Color_t color;	
	int tmp,h,s,b;		
	
	tmp   = hue*360;	
	tmp   = tmp/254;	
	h   = (int)(tmp-0.5f);	

	tmp = sat*100;
	tmp = tmp/255;
	s   = (int)(tmp+0.5f);
	if(s < 20){
		s = 20;
	}
	
	tmp   = bri*100;
	tmp   = tmp/255;
	b   = (int)(tmp+0.5f);  //brightness from 10 to 100; real value from 25 to 254
	if(b < 10){
		b = 10;
	}
	
	color = HAL_HSVtoRGB(h, s ,b);
	// APP_DBG_PRINTF("%s:color.r*GAIN = %d, color.g*GAIN = %d, color.b*GAIN = %d\r\n ",__FUNCTION__,color.r*GAIN,color.g*GAIN,color.b*GAIN);
	HAL_UpdatePwmDuty(color.r*GAIN, color.g*GAIN, color.b*GAIN, 0, 0);
	return;
}

void HAL_Lighting_Sunlight(int ctp, int ctp_bri)
{   
    int cw,ww, sctp,sbri;
	float nGAIN,rate;

	
	sctp = ctp;
	sbri = ctp_bri;

	if( sbri < 10 ){
		nGAIN = ((float)10)*GAIN/255;
	}else{
		nGAIN = ((float)sbri)*GAIN/255;
	}


	if(sctp <= 3000){
          ww = (int)(255 * nGAIN);
          cw = 0;
	}else if(sctp >= 6300){
          cw = (int)(255 * nGAIN);
          ww = 0;
	} else {
          rate = (sctp-3000)/3300.0;
          ww = (int)(255 * (1.0-rate) * nGAIN);
          cw = (int)(255 * rate * nGAIN);
	}
 
	HAL_UpdatePwmDuty(0, 0, 0, cw, ww);
	
}

void HAL_Lighting_OFF(void)
{
	HAL_UpdatePwmDuty(0 ,0 ,0 ,0 ,0);
}

void HAL_Lighting_ON(void)
{	
	if(mLightManager==NULL){
		APP_DBG_PRINTF("%s mLightManager=NULL !!!\r\n",__func__);	
		return;
	}
		
#if USE_LIGHT_FOR_SKYIOT
	#if ((SKY_LIGHT_TYPE==SKY_LIGHT_BELT_TYPE)||(SKY_LIGHT_TYPE==SKY_LIGHT_BULB_TYPE))
  	HAL_Lighting_Sunlight( mLightManager->ctp, mLightManager->bri );
	
	#elif (SKY_LIGHT_TYPE==SKY_LIGHT_BULB_RGBWY_TYPE)
	if(mLightManager->ctp){
  		HAL_Lighting_Sunlight( mLightManager->ctp, mLightManager->bri ); 
	}else{
		HAL_Lighting_Color( mLightManager->hue, mLightManager->sat, mLightManager->bri );	 
	}
	
	#endif
#endif

	
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
	// HAL_Lighting_OFF();

	return true;	
}


#endif
