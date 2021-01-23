
#include <trace.h>
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_adc.h"
#include "bee2_adc_lib.h"
#include "rtl876x_gpio.h"

#include "skyadc.h"
// 转换表，对应bitmap
#define ADC_SCHEDULE_0             	0
#define ADC_SCHEDULE_1            	1
#define ADC_SCHEDULE_2           	2
#define ADC_SCHEDULE_3           	3
#define ADC_SCHEDULE_4           	4
#define ADC_SCHEDULE_5           	5
#define ADC_SCHEDULE_6             	6
#define ADC_SCHEDULE_7             	7
#define ADC_SCHEDULE_8           	8
#define ADC_SCHEDULE_9          	9
#define ADC_SCHEDULE_10       		10
#define ADC_SCHEDULE_11           	11
#define ADC_SCHEDULE_12            	12
#define ADC_SCHEDULE_13            	13
#define ADC_SCHEDULE_14           	14
#define ADC_SCHEDULE_15       		15
// 通道
#define ADC_CHANNEL_4              	4
#define ADC_CHANNEL_5         		5

#define ALS_POWER_PIN 				GPIO_GetPin(ALS_POWER)
#define ADCPOWER_OPEN               ((BitAction)1)
#define ADCPOWER_CLOSE              ((BitAction)0)

#define BATT_GRADE_NUMBER_3              2500
#define BATT_GRADE_NUMBER_2              2200
#define BATT_GRADE_NUMBER_1              2000
#define BATT_GRADE_NUMBER_0              1800

static void Driver_ADC_init()
{
    ADC_DeInit(ADC);
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    ADC_InitTypeDef adcInitStruct;
    ADC_StructInit(&adcInitStruct);
	for(int i=0;i<16;i++)
	{
		adcInitStruct.schIndex[ADC_SCHEDULE_0+i]  = INTERNAL_VBAT_MODE;
	}
    adcInitStruct.bitmap              = 0xFFFF;
	adcInitStruct.adcSamplePeriod     = 255;
    ADC_Init(ADC, &adcInitStruct);
    ADC_INTConfig(ADC, ADC_INT_ONE_SHOT_DONE, ENABLE);
    ADC_SchTableConfig(ADC,ADC_SCHEDULE_0,INTERNAL_VBAT_MODE);
	ADC_CalibrationInit();
}




void Sky_ADC_POWER_Init(void)
{
    Driver_ADC_init();
}



static uint16_t calc_avg(uint16_t *arr, uint8_t index, uint8_t arr_len)
{
	if(arr_len <= 2)
		return 0;
    
    uint32_t avg = arr[0];
    uint16_t max = arr[0];
    uint16_t min = arr[0];
	
	for (int i = 1; i < arr_len; i++)
    {
        if (arr[i] > max)
        {
            max = arr[i];
        }
        if (arr[i] < min)
        {
            min = arr[i];
        }
        avg += arr[i];
    }
    avg = (avg - max - min) / (arr_len-2);

	return avg;
}


void HAL_SkyAdc_Sample(uint16_t *bat_dat)
{
	uint16_t sample_data[16];
    uint16_t vol[16];
	uint16_t batt_data[16];
	
	for(int i=0;i<16;i++)
	{
		ADC_ErrorStatus ErrorStatus;
		ADC_Cmd(ADC, ADC_One_Shot_Mode, ENABLE);
		while(ADC_GetIntFlagStatus(ADC, ADC_INT_ONE_SHOT_DONE) == RESET);
		ADC_ClearINTPendingBit(ADC, ADC_INT_ONE_SHOT_DONE);
		for (int j=0;j<16;j++)
        {
            sample_data[j] = ADC_ReadByScheduleIndex(ADC, ADC_SCHEDULE_0+j);
            vol[j] = ADC_GetVoltage(DIVIDE_SINGLE_MODE, sample_data[j], &ErrorStatus);
        }
        batt_data[i] = calc_avg(vol, 0, 16);
	}
	*bat_dat = calc_avg(batt_data, 0, 16);
}


