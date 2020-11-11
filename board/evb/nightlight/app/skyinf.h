#ifndef __SKYINF_HEAD__
#define __SKYINF_HEAD__

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>


extern bool infwakeupflag;


extern void HAL_INF_Dlps_Control(bool isenter);
extern uint8_t HAL_ReadInf_Statu(void);
extern uint8_t HAL_ReadInf_Power(void);
extern bool HAL_Inf_Init(void);
extern void HAL_OpenInf_Power(bool isallow);

#endif

