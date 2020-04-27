

#ifndef _SKYSWRELAY_H_
#define _SKYSWRELAY_H_
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>


#define USE_SWRELAY_FOR_SKYIOT  1

#define BLEMESH_REPORT_FLAG_SWT (0x1)


#define SWITCH_RELAY_OFF    0   // ¼ÌµçÆ÷IO
#define SWITCH_RELAY_ON     1 	
#define SWITCH_RELAY_BLINK  2 	

typedef struct {
	// ¼ÌµçÆ÷×´Ì¬×Ö¶Î
	uint8_t status; /* 0: off, 1: on */
}SkySwRelayManager;



void HAL_SwRelay_Control(uint8_t mode);
bool HAL_SwRelay_Init(SkySwRelayManager *manager);


#endif //



