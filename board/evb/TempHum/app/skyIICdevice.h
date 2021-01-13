#ifndef __SKYIICDEVICE_HEAD__
#define __SKYIICDEVICE_HEAD__

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>
#include <string.h>


extern void RTL8762_IIC_Init(void);
extern void BL55072A_Init(uint8_t* data, uint8_t len);
extern void BL55072A_DisplayOn(void);
void SkyIot_Lcd_Display(uint32_t humidity, int temperature, uint8_t rssi, uint8_t battery);

extern void SHTC3_Init(void);
extern uint8_t SHTC3_Read_Temp_Hum(int *gettemp, uint32_t *gethum);

extern void displaymain(void);
extern void Single_blink(void);
extern void HAL_Sky_I2C_Dlps(bool allowenter);

#endif
