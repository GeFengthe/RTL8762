#ifndef __SKYADC_HEAD__
#define __SKYADC_HEAD__

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>


extern void Sky_ADC_POWER_Init(void);
extern uint8_t HAL_ReadAmbient_Power(void);
extern void HAL_Set_Ambient_Power(uint8_t val);
extern void HAL_SkyAdc_Sample(uint16_t *bat_dat, uint16_t *lp_dat);
extern void HAL_Adc_Dlps_Control(bool isenter);

#endif
