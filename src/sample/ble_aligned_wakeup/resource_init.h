#include <rtl876x_rcc.h>
#include <rtl876x_nvic.h>
#include <rtl876x_adc.h>
#include <bee2_adc_lib.h>
#include <rtl876x_rtc.h>

/* Timer timing config */
#define ALIGNED_WAKEUP         1

#define TIMING_TIME     100000    //uint: us
#define TIMER_PERIOD    ((TIMING_TIME)*40-1)

extern void driver_rtc_init(void);
extern void driver_timer_init(void);
//extern void Driver_ADC_init(void);
extern void *xTimerADC;
extern void KEY_Handler(void);
extern void driver_gpio_init(void);
