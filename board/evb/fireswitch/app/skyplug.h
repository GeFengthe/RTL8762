

#ifndef _SKYLSWITCH_H_
#define _SKYLSWITCH_H_
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>


#define USE_PLUG_FOR_SKYIOT  1



#define LEDTURNOFF    	1
#define LEDTURNON    	0
#define LEDBLINK    	2

#define RELAY_OFF    	0
#define RELAY_ON    	1

#define BLEMESH_REPORT_FLAG_SWT (0x1)

enum SKY_SWITCH_INDEX_ENUM{
    SKYSWITC1_ENUM = 0x00,	
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


bool HAL_Switch_Init(SkySwitchManager *manager);

void HAL_ProvisionLed_Control(uint8_t mode);
void HAL_BlinkProLed_Enable(void);
void HAL_BlinkProLed_Disable(void);
bool HAL_BlinkProLed_Statu(void);

void HAL_SwitchLed_Control(uint8_t index, uint8_t mode);
void HAL_SwitchRelay_Control(uint8_t index, uint8_t mode);

void HAL_Switch_HandleTimer(void *timer);


#endif //



