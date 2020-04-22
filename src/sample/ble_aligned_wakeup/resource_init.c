#include <resource_init.h>
#include <trace.h>
#include <os_timer.h>
#include <aligned_wakeup_app.h>
#include <app_msg.h>
#include <app_task.h>
#include <bee2_adc_lib.h>
#include <rtl876x_tim.h>
#include <simple_ble_service.h>
#include <board.h>
#include <rtl876x_gpio.h>
#include <rtl876x_nvic.h>
#include <rtl876x_pinmux.h>

/** @brief Prescaler value.
  * 12 bits prescaler for COUNTER frequency (32768/(PRESCALER+1)).
  * If use internal 32KHz, (32000/(PRESCALER+1)).
  * Must be written when RTC 24-bit counter is stopped.
  */
#define RTC_PRESCALER_VALUE     (32-1)//f = 10Hz
/* RTC has four comparators.0~3 */
#define RTC_COMP_INDEX          1
#define RTC_INT_CMP_NUM         RTC_INT_CMP1
#define RTC_COMP_VALUE          (100)
#define ADC_SCHEDULE_0          0


void *xTimerADC;

void driver_gpio_init(void)
{
    //GPIO_DeInit();
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
    GPIO_InitTypeDef Gpio_Struct;
    GPIO_StructInit(&Gpio_Struct);

    Gpio_Struct.GPIO_Pin = GPIO_KEY;
    Gpio_Struct.GPIO_Mode = GPIO_Mode_IN;
    Gpio_Struct.GPIO_ITCmd = ENABLE;
    Gpio_Struct.GPIO_ITTrigger = GPIO_INT_Trigger_EDGE;
    Gpio_Struct.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;
    Gpio_Struct.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
    Gpio_Struct.GPIO_DebounceTime = 20;
    GPIO_Init(&Gpio_Struct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GPIO_KEY_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    GPIO_MaskINTConfig(GPIO_KEY, DISABLE);
    GPIO_INTConfig(GPIO_KEY, ENABLE);


    //
}

#if defined(ALIGNED_WAKEUP)
/**
 * @brief    Contains the initialization of peripherals
 * @note     Both new architecture driver and legacy driver initialization method can be used
 * @return   void
 */
void driver_rtc_init(void)
{
    //RTC_DeInit();
    RTC_SetPrescaler(RTC_PRESCALER_VALUE);

    RTC_SetComp(RTC_COMP_INDEX, RTC_COMP_VALUE);
    RTC_MaskINTConfig(RTC_INT_CMP_NUM, ENABLE);
    RTC_CompINTConfig(RTC_INT_CMP_NUM, DISABLE);

    /* Config RTC interrupt */
//    NVIC_InitTypeDef NVIC_InitStruct;
//    NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
//    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
//    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStruct);

}

/**
  * @brief  Initialize tim peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_timer_init(void)
{
    //TIM_DeInit();
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);

    TIM_InitStruct.TIM_PWM_En = PWM_DISABLE;
    TIM_InitStruct.TIM_Period = TIMER_PERIOD ;
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_TimeBaseInit(TIM6, &TIM_InitStruct);

    /*  Enable TIMER IRQ  */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIMER6_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    TIM_ClearINT(TIM6);
    TIM_INTConfig(TIM6, ENABLE);
    //TIM_Cmd(TIM6, ENABLE);
}
#endif
//void Driver_ADC_init(void)
//{
//    int i;
//    ADC_DeInit(ADC);
//    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

//    ADC_InitTypeDef adcInitStruct;
//    ADC_StructInit(&adcInitStruct);

//    for (i = 0; i < 16; i++)
//    {
//        adcInitStruct.schIndex[ADC_SCHEDULE_0 + i]         = INTERNAL_VBAT_MODE;
//    }

//    //(1<<ADC_SCHEDULE_0) | (1<<ADC_SCHEDULE_1).........|(1<<ADC_SCHEDULE_15);
//    adcInitStruct.bitmap              = 0x1;
//    adcInitStruct.adcSamplePeriod     = 127;             //10M/(127+1)=78.125K
//    adcInitStruct.adcConvertTimePeriod = ADC_CONVERT_TIME_1100NS;
//    adcInitStruct.timerTriggerEn      = ENABLE;
//    ADC_Init(ADC, &adcInitStruct);
//    ADC_INTConfig(ADC, ADC_INT_ONE_SHOT_DONE, ENABLE);

//    NVIC_InitTypeDef NVIC_InitStruct;
//    NVIC_InitStruct.NVIC_IRQChannel = ADC_IRQn;
//    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
//    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStruct);
//}

//void ADC_Handler(void)
//{
//    uint16_t sample_data;
//    uint16_t vol;
////    uint16_t res = 0;
//    if (ADC_GetIntFlagStatus(ADC, ADC_INT_ONE_SHOT_DONE) == SET)
//    {
//        ADC_ErrorStatus ErrorStatus;
//        ADC_ClearINTPendingBit(ADC, ADC_INT_ONE_SHOT_DONE);
//        sample_data = ADC_ReadByScheduleIndex(ADC, ADC_SCHEDULE_0);
//        vol =  ADC_GetVoltage(DIVIDE_SINGLE_MODE, sample_data, &ErrorStatus);
//        APP_PRINT_INFO1("Voltage : %d", vol);
//    }
//}

void vTimerADCCallback(void *pxTimer)
{
    //allowedEnterDlps = false;
    APP_PRINT_INFO0("sw timer timeout");
    val[1]++;
    simp_ble_service_send_v3_notify(connect_id, simp_srv_id, val, 2);
}

void sw_timer_init(void)
{
    bool retval ;

    retval = os_timer_create(&xTimerADC, "xTimerADC",  1, \
                             50/*100ms*/, true, vTimerADCCallback);
    if (!retval)
    {
        APP_PRINT_INFO1("xTimerADC retval is %d", retval);
    }
}

/**
  * @brief  TIM6 interrupt handler function.
  * @param  No parameter.
  * @return void
*/
#if defined(ALIGNED_WAKEUP)
void Timer6_Handler(void)
{
    TIM_ClearINT(TIM6);
    TIM_Cmd(TIM6, DISABLE);
    val[1]++;
    simp_ble_service_send_v3_notify(connect_id, simp_srv_id, val, 2);
    //Add user code here
    previous_val = RTC_GetCounter();
    if (change_period)
    {
        TIM_ChangePeriod(TIM6, TIMER_PERIOD);
        change_period = false;
    }
    TIM_Cmd(TIM6, ENABLE);
}
#endif


void KEY_Handler(void)
{
    GPIO_MaskINTConfig(GPIO_KEY, ENABLE);
    APP_PRINT_INFO0("Enter GPIO Interrupt");
    uint8_t keystatus =  GPIO_ReadInputDataBit(GPIO_KEY);

    if (keystatus == 0)
    {
        GPIO->INTPOLARITY |= GPIO_KEY;
        allowedEnterDlps = false;
    }

    else
    {
        GPIO->INTPOLARITY &= ~GPIO_KEY;
        allowedEnterDlps = true;
    }

    GPIO_ClearINTPendingBit(GPIO_KEY);
    GPIO_MaskINTConfig(GPIO_KEY, DISABLE);
}
