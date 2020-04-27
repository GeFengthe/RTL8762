

#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <math.h>
#include "rtl876x_gpio.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "skyswrelay.h"


#define APP_DBG_PRINTF(fmt, ...)


#define SW_RELAY1_GPIO           P0_1
#define SW_RELAY1_GPIO_PIN       GPIO_GetPin(SW_RELAY1_GPIO)

static SkySwRelayManager *mSwRelayManager=NULL;



/*
** GPIO
*/
static void HAL_GpioForSwRelay_Init(void)
{	
	/* Configure pad and pinmux firstly! */
	Pad_Config(SW_RELAY1_GPIO, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, SWITCH_RELAY_ON); // PAD_OUT_HIGH);
    Pinmux_Config(SW_RELAY1_GPIO, DWGPIO);
		
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin    = SW_RELAY1_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
}

void HAL_SwRelay_Control(uint8_t mode)
{
	switch(mode){
		case SWITCH_RELAY_OFF:{
			GPIO_WriteBit(SW_RELAY1_GPIO_PIN, (BitAction)(SWITCH_RELAY_OFF));
		break;
		}
		case SWITCH_RELAY_ON:{
			GPIO_WriteBit(SW_RELAY1_GPIO_PIN, (BitAction)(SWITCH_RELAY_ON));
		break;
		}
		case SWITCH_RELAY_BLINK:{
			APP_DBG_PRINTF("%s proval:%d\r\n",__func__, GPIO_ReadOutputDataBit(SW_RELAY1_GPIO_PIN))
			if(GPIO_ReadOutputDataBit(SW_RELAY1_GPIO_PIN)==1){				
				GPIO_WriteBit(SW_RELAY1_GPIO_PIN, (BitAction)(SWITCH_RELAY_OFF));
			} else{				
				GPIO_WriteBit(SW_RELAY1_GPIO_PIN, (BitAction)(SWITCH_RELAY_ON));
			}
		break;
		}
	}
}


bool HAL_SwRelay_Init(SkySwRelayManager *manager)
{
	if( manager == NULL){
		return false;
	}
		
	mSwRelayManager = manager;
	HAL_GpioForSwRelay_Init();

	return true;	
}



