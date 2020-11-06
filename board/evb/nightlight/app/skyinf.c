
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"

#include "skyinf.h"

// #define APP_DBG_PRINTF(fmt, ...)
#define APP_DBG_PRINTF   DBG_DIRECT

#define INFSEN_POWER		        P2_6			            // Infrared Sensor Power(IO--O)
#define INFSEN_DETECT				P2_7			            // Infrared Sensor Detect(Intter)
#define INFSEN_POWER_PIN			GPIO_GetPin(INFSEN_POWER)
#define INFSEN_DETECT_PIN			GPIO_GetPin(INFSEN_DETECT)
#define INFPOWER_OPEN               ((BitAction)1)
#define INFPOWER_CLOSE              ((BitAction)0)

static SkyInfManager *mInfManager = NULL;

static void HAL_GpioForInf_Init(void)
{
    Pad_Config(INFSEN_POWER, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pinmux_Config(INFSEN_POWER, DWGPIO);	
    Pad_Config(INFSEN_DETECT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pinmux_Config(INFSEN_DETECT, DWGPIO);
    
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct;
    
    GPIO_StructInit(&GPIO_InitStruct);	
    GPIO_InitStruct.GPIO_Pin    = INFSEN_DETECT_PIN;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    
    GPIO_StructInit(&GPIO_InitStruct);	
    GPIO_InitStruct.GPIO_Pin    = INFSEN_POWER_PIN;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_WriteBit(INFSEN_POWER_PIN, INFPOWER_CLOSE);
}


uint8_t ReadInfStatu(void)
{
    uint8_t inf = 0;
	
	if(GPIO_ReadInputDataBit(INFSEN_DETECT_PIN) == 1){
		inf |= (1<<0);
	} 	
	return inf; 
}


void HAL_OpenInf_Power(bool isallow)
{
    if(isallow){
        GPIO_WriteBit(INFSEN_POWER_PIN, INFPOWER_OPEN);
    }else{
        GPIO_WriteBit(INFSEN_POWER_PIN, INFPOWER_CLOSE);
    }
}


bool HAL_Inf_Init(SkyInfManager *manager)
{
	if( manager == NULL){
		return false;
	}
		
	mInfManager = manager;
	HAL_GpioForInf_Init();
    mInfManager->inf_status_new = 0;
    return 0;
}


void HAL_INF_Dlps_Control(bool isenter)
{
    PAD_OUTPUT_VAL outval;
	
	uint8_t val = GPIO_ReadOutputDataBit(INFSEN_POWER_PIN);
	if(val){
		outval = PAD_OUT_HIGH;
	}else{
		outval = PAD_OUT_LOW;
	}
    
    if(isenter){
		Pad_Config(INFSEN_POWER, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, outval);
        Pad_Config(INFSEN_DETECT, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
        if(outval == PAD_OUT_HIGH){
//            APP_DBG_PRINTF("keep inf power\r\n");
			System_WakeUpPinEnable(INFSEN_DETECT, PAD_WAKEUP_POL_HIGH, 0);
        }
	}else{
		Pad_Config(INFSEN_POWER, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, outval);	
        Pad_Config(INFSEN_DETECT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
        if(System_WakeUpInterruptValue(INFSEN_DETECT) == 1){	
//            APP_DBG_PRINTF("inf\r\n");
            Pad_ClearWakeupINTPendingBit(INFSEN_DETECT);
            System_WakeUpPinDisable(INFSEN_DETECT);	
            mInfManager->inf_status_new = 1;
            mInfManager->inf_status_old = 0;
        }
	}
}