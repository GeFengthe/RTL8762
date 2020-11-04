#ifndef __SKYADC_HEAD__
#define __SKYADC_HEAD__

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>


extern void Sky_ADC_POWER_Init(void);
extern void HAL_SkyAdc_Sample(uint16_t *bat_dat, uint16_t *lp_dat);
extern void HAL_Adc_Dlps_Control(bool isenter);

#endif
