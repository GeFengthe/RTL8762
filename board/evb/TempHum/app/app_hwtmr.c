#include <string.h>
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_tim.h"

#include "app_hwtmr.h"
#include "soft_wdt.h"


#define TIMER_NUM       TIM7
#define TIMER_IRQN      TIMER7_IRQ
#define TIMER_PERIOD    (500*4000-1)  // 50ms


/**
  * @brief  Initialize tim peripheral.
  * @param  No parameter.
  * @return void
*/
extern void Hal_Timer_init(void)
{
	// TIM_DeInit();
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);

    TIM_InitStruct.TIM_PWM_En = PWM_DISABLE;
    TIM_InitStruct.TIM_Period = TIMER_PERIOD;
    TIM_InitStruct.TIM_Mode   = TIM_Mode_UserDefine;
    TIM_TimeBaseInit(TIMER_NUM, &TIM_InitStruct);

    /*  Enable TIMER IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = TIMER_IRQN;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    TIM_ClearINT(TIMER_NUM);
    TIM_INTConfig(TIMER_NUM, ENABLE);
    TIM_Cmd(TIMER_NUM, ENABLE);
}


/**
  * @brief  TIM7 interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void Timer7_Handler(void)
{
    TIM_ClearINT(TIMER_NUM);
    TIM_Cmd(TIMER_NUM, DISABLE);
	
    //Add User code here
	#if USE_SOFT_WATCHDOG
	SoftWdtISR();
	#endif
	
    TIM_Cmd(TIMER_NUM, ENABLE);
}

