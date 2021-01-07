#ifndef __SKYDISPLAY_HEAD__
#define __SKYDISPLAY_HEAD__

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>
#include <string.h>


extern void BL55072A_IIC_Init(void);
extern void BL55072A_Init(uint8_t* data, uint8_t len);
extern void BL55072A_DisplayOn(void);
extern void SkyIot_Lcd_Display(uint32_t humidity, int temperature, uint8_t rssi, uint8_t battery);

extern void displaymain(void);

#endif
