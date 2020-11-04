#ifndef __SKYINF_HEAD__
#define __SKYINF_HEAD__

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>


typedef struct{
    uint8_t inf_status_old;
	uint8_t inf_status_new;
}SkyInfManager; 


extern void HAL_INF_Dlps_Control(bool isenter);
extern uint8_t ReadInfStatu(void);
extern bool HAL_Inf_Init(SkyInfManager *manager);
extern void HAL_OpenInf_Power(bool isallow);

#endif

