

#ifndef _SKYLSWITCH_H_
#define _SKYLSWITCH_H_
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>


#define USE_SWITCH_FOR_SKYIOT  1

#define SKY_ONEWAYSWITCH_TYPE         1   // 一路
#define SKY_TWOWAYSWITCH_TYPE         2   // 两路
#define SKY_THREEWAYSWITCH_TYPE       3   // 三路

#define SKY_SWITCH_TYPE   SKY_ONEWAYSWITCH_TYPE  


#define LED_PRO_OFF 0   // 配网指示灯
#define LED_PRO_ON  1 

#define LEDTURNOFF    	0
#define LEDTURNON    	1
#define LEDBLINK    	2

#if (SKY_SWITCH_TYPE == SKY_ONEWAYSWITCH_TYPE)
#define BLEMESH_REPORT_FLAG_SWT1 (0x1)

#elif (SKY_SWITCH_TYPE == SKY_TWOWAYSWITCH_TYPE)
#define BLEMESH_REPORT_FLAG_SWT1 (0x1)
#define BLEMESH_REPORT_FLAG_SWT2 (0x2)

#elif (SKY_SWITCH_TYPE == SKY_THREEWAYSWITCH_TYPE)
#define BLEMESH_REPORT_FLAG_SWT1 (0x1)
#define BLEMESH_REPORT_FLAG_SWT2 (0x2)
#define BLEMESH_REPORT_FLAG_SWT3 (0x4)

#endif

enum SKY_SWITCH_INDEX_ENUM{
    SKYSWITC1_ENUM = 0x00,	
	#if (SKY_SWITCH_TYPE == SKY_TWOWAYSWITCH_TYPE || SKY_SWITCH_TYPE == SKY_THREEWAYSWITCH_TYPE)
    SKYSWITC2_ENUM ,
	#endif	
	#if (SKY_SWITCH_TYPE == SKY_THREEWAYSWITCH_TYPE)
    SKYSWITC3_ENUM ,
	#endif	
    SKYSWITC_NUMBERS ,	
};

typedef enum{
	KEY_MODE_INIT = 0x00,
	KEY_SHORTPRESS_MODE,    
	KEY_LONGPRESS_MODE,   
}KEY_PRESS_MODE_e;	

typedef struct {
	uint8_t status[SKYSWITC_NUMBERS];	
	uint8_t keyval;     // bit0:switch1 bit1:switch2   ; 1:press
	uint8_t keymode;    // KEY_PRESS_MODE_e
	
	// uint32_t report_flag;	
}SkySwitchManager;


extern bool HAL_Switch_Init(SkySwitchManager *manager);

extern void HAL_ProvisionLed_Control(uint8_t mode);
extern void HAL_ProvLed_Dlps_Control(uint8_t val, bool isenter);
extern void HAL_SwitchLed_Dlps_Control(uint8_t index, uint8_t val, bool isenter);
extern void HAL_SwitchKey_Dlps_Control(bool isenter);
extern void HAL_BlinkProLed_Enable(void);
extern void HAL_BlinkProLed_Disable(void);
extern bool HAL_BlinkProLed_Statu(void);

extern void HAL_SwitchLed_Control(uint8_t index, uint8_t mode);

extern void HAL_Switch_HandleTimer(void *timer);

extern bool HAL_Switch_Is_Relese(void);

extern uint8_t Read_ZVD_Statu(void);

#endif //



