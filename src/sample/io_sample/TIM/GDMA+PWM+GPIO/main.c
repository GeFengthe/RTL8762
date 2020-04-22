/**
*********************************************************************************************************
*               Copyright(c) 2019, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of gdma send data to gpio.
            This project uses GDMA+TIM+GPIO to implement the PWM function,
            and can dynamically change the duty cycle function of the PWM output.
* @details
* @author   yuan
* @date     2019-02-22
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_gdma.h"
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "rtl876x_tim.h"
#include "trace.h"

/* Defines ------------------------------------------------------------------*/
/** Set GPIO_OUT_REPEAT to 1 if repeat play is required.
  * If GPIO_OUT_REPEAT set to 1 output array GPIO_Ctl_AutoReload data, otherwise output array GPIO_Ctl_LLI.
  */
#define GPIO_OUT_REPEAT                 0

#define GPIO_OUTPUT_PIN_0               P2_2
#define GPIO_PIN_OUTPUT                 GPIO_GetPin(GPIO_OUTPUT_PIN_0)

#define PWM_OUT_PIN                     P2_3
#define PWM_TIMER_NUM                   TIM2
#define PWM_OUT_PIN_PINMUX              timer_pwm2

#define PWM_HIGH_COUNT                  (10*20-1)//5us
#define PWM_LOW_COUNT                   (10*20-1)//5us

#define GDMA_CHANNEL_NUM                2
#define GDMA_Channel                    GDMA_Channel2
#define GDMA_Channel_IRQn               GDMA0_Channel2_IRQn
#define GDMA_Channel_Handler            GDMA0_Channel2_Handler

#define GDMA_TRANSFER_SIZE              24
#define GDMA_LLI_SIZE                   4

/* Globales ------------------------------------------------------------------*/
uint32_t GPIO_Ctl_AutoReload[GDMA_TRANSFER_SIZE] = {0xffffffff, 0x0,
                                                    0xffffffff, 0xffffffff, 0x0,
                                                    0xffffffff, 0xffffffff, 0xffffffff, 0x0,
                                                    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x0,
                                                    0xffffffff, 0xffffffff, 0xffffffff, 0x0,
                                                    0xffffffff, 0xffffffff, 0x0,
                                                    0xffffffff, 0x0
                                                   };

uint32_t GPIO_Ctl_LLI[GDMA_LLI_SIZE][GDMA_TRANSFER_SIZE] =
{
    0xffffffff, 0x0, 0xffffffff, 0x0, 0xffffffff, 0x0, 0xffffffff, 0x0, 0xffffffff, 0x0, 0xffffffff, 0x0,
    0xffffffff, 0x0, 0xffffffff, 0x0, 0xffffffff, 0x0, 0xffffffff, 0x0, 0xffffffff, 0x0, 0xffffffff, 0x0,
    0xffffffff, 0xffffffff, 0x0, 0xffffffff, 0xffffffff, 0x0, 0xffffffff, 0xffffffff, 0x0, 0xffffffff, 0xffffffff, 0x0,
    0xffffffff, 0xffffffff, 0x0, 0xffffffff, 0xffffffff, 0x0, 0xffffffff, 0xffffffff, 0x0, 0xffffffff, 0xffffffff, 0x0,
    0xffffffff, 0x0, 0x0, 0xffffffff, 0x0, 0x0, 0xffffffff, 0x0, 0x0, 0xffffffff, 0x0, 0x0,
    0xffffffff, 0x0, 0x0, 0xffffffff, 0x0, 0x0, 0xffffffff, 0x0, 0x0, 0xffffffff, 0x0, 0x0,
    0xffffffff, 0xffffffff, 0x0, 0x0, 0xffffffff, 0xffffffff, 0x0, 0x0, 0xffffffff, 0xffffffff, 0x0, 0x0,
    0xffffffff, 0xffffffff, 0x0, 0x0, 0xffffffff, 0xffffffff, 0x0, 0x0, 0xffffffff, 0xffffffff, 0x0, 0x0,
};

GDMA_LLIDef GDMA_LLIStruct[GDMA_LLI_SIZE];


/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_gpio_init(void)
{
    Pad_Config(GPIO_OUTPUT_PIN_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pinmux_Config(GPIO_OUTPUT_PIN_0, DWGPIO);
}

void board_pwm_init(void)
{
    Pad_Config(PWM_OUT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    Pinmux_Config(PWM_OUT_PIN, PWM_OUT_PIN_PINMUX);
}

/**
  * @brief  Initialize gpio peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_gpio_init(void)
{
    /* Initialize gpio peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin    = GPIO_PIN_OUTPUT;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd  = DISABLE;
    GPIO_InitStruct.GPIO_ControlMode = GPIO_HARDWARE_MODE;
    GPIO_Init(&GPIO_InitStruct);

}
/**
  * @brief  Initialize tim peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_pwm_init(void)
{
    /* Initialize tim peripheral */
    RCC_PeriphClockCmd(APBPeriph_TIMER, APBPeriph_TIMER_CLOCK, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_StructInit(&TIM_InitStruct);

    TIM_InitStruct.TIM_PWM_En = PWM_ENABLE;
    TIM_InitStruct.TIM_PWM_High_Count   = PWM_HIGH_COUNT;
    TIM_InitStruct.TIM_PWM_Low_Count    = PWM_LOW_COUNT;
    TIM_InitStruct.TIM_Mode = TIM_Mode_UserDefine;
    TIM_TimeBaseInit(PWM_TIMER_NUM, &TIM_InitStruct);

}

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_gdma_multiblock_init(void)
{
    /* Set gdma with GDMA_Handshake_TIM2, msize=1, transfer width = 32 */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);

    GDMA_InitStruct.GDMA_ChannelNum          = GDMA_CHANNEL_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = GDMA_TRANSFER_SIZE;//determine total transfer size
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)GPIO_Ctl_AutoReload;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(0x40011200);//vendor gpio reg address
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_TIM2;
    GDMA_InitStruct.GDMA_Multi_Block_En     = 1;
    GDMA_InitStruct.GDMA_Multi_Block_Struct = (uint32_t)GDMA_LLIStruct;
#if (GPIO_OUT_REPEAT == 1)
    GDMA_InitStruct.GDMA_Multi_Block_Mode   = LLI_WITH_AUTO_RELOAD_SAR;
#else
    GDMA_InitStruct.GDMA_Multi_Block_Mode   = LLI_TRANSFER;
#endif
    for (int i = 0; i < GDMA_LLI_SIZE; i++)
    {
        GDMA_LLIStruct[i].SAR = (uint32_t)(&(GPIO_Ctl_LLI[i]));
        GDMA_LLIStruct[i].DAR = (uint32_t)(0x40011200);
        if (i == (GDMA_LLI_SIZE - 1))
        {
            GDMA_LLIStruct[i].LLP = 0;  //link back to beginning
            /* Configure low 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct.GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct.GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct.GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct.GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct.GDMA_DIR << 20);
            /* Configure high 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_HIGH = GDMA_InitStruct.GDMA_BufferSize;
        }
        else
        {
            GDMA_LLIStruct[i].LLP = (uint32_t)&GDMA_LLIStruct[i + 1];
            /* Configure low 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct.GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct.GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct.GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct.GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct.GDMA_DIR << 20)
                                        | (GDMA_InitStruct.GDMA_Multi_Block_Mode & LLP_SELECTED_BIT);//BIT(28) | BIT(27)
            /* Configure high 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_HIGH = GDMA_InitStruct.GDMA_BufferSize;
        }
    }

    GDMA_Init(GDMA_Channel, &GDMA_InitStruct);

    /* GDMA irq config */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);

    GDMA_INTConfig(GDMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    GDMA_Cmd(GDMA_CHANNEL_NUM, ENABLE);

    /* Set timer toggle */
    TIM_Cmd(PWM_TIMER_NUM, ENABLE);
}

/**
  * @brief  Demo code of operation about pwm + gpio + gdma.
  * @param  No parameter.
  * @return void
*/
void gdma_pwm_gpio_demo(void)
{
    board_gpio_init();
//    board_pwm_init();
    driver_gpio_init();
    driver_pwm_init();
    driver_gdma_multiblock_init();
}

/**
  * @brief  Entry of app code
  * @return int (To avoid compile warning)
  */
int main(void)
{
    gdma_pwm_gpio_demo();

    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();

    }
}

/**
  * @brief  GDMA0 channel interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void GDMA_Channel_Handler(void)
{
    GDMA_ClearINTPendingBit(GDMA_CHANNEL_NUM, GDMA_INT_Transfer);
}

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/
