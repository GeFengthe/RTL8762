/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     qdec_demo.c
* @brief    qdec demo
* @details
* @author   yuan
* @date     2018-05-29
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_qdec.h"
#include "rtl876x_rcc.h"
#include "trace.h"

/* Defines -------------------------------------------------------------------*/
/* phase A */
#define QDEC_Y_PHA_PIN          P2_4
/* phase B */
#define QDEC_Y_PHB_PIN          P2_5

int16_t   Y_Axis_Count;
uint16_t  Y_Axis_Direction;

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_qdec_init(void)
{
    /* Qdecoder pad config */
    Pad_Config(QDEC_Y_PHA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(QDEC_Y_PHB_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    /* Qdecoder pinmux config */
    Pinmux_Config(QDEC_Y_PHA_PIN, qdec_phase_a_y);
    Pinmux_Config(QDEC_Y_PHB_PIN, qdec_phase_b_y);
}

/**
  * @brief  Initialize Qdecoder peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_qdec_init(void)
{
    QDEC_DeInit(QDEC);
    RCC_PeriphClockCmd(APBPeriph_QDEC, APBPeriph_QDEC_CLOCK, ENABLE);

    QDEC_InitTypeDef qdecInitStruct;
    QDEC_StructInit(&qdecInitStruct);
    qdecInitStruct.axisConfigY       = ENABLE;
    qdecInitStruct.debounceEnableY   = Debounce_Enable;
    QDEC_Init(QDEC, &qdecInitStruct);
    QDEC_INTConfig(QDEC, QDEC_Y_INT_NEW_DATA, ENABLE);
    QDEC_Cmd(QDEC, QDEC_AXIS_Y, ENABLE);

    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = qdecode_IRQn;
    nvic_init_struct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&nvic_init_struct);

}

/**
  * @brief  Demo code of qdec.
  * @param  No parameter.
  * @return Void
  */
void qdec_demo(void)
{
    board_qdec_init();
    driver_qdec_init();
}

/**
  * @brief  Qdecode interrupt handler function.
  * @param  No parameter.
  * @return Void
*/
void Qdecode_Handler(void)
{
    if (QDEC_GetFlagState(QDEC, QDEC_FLAG_NEW_CT_STATUS_Y) == SET)
    {
        DBG_DIRECT("Qdecode_Handler");
        DBG_DIRECT("QDEC_FLAG_NEW_CT_STATUS_Y");
        /* Mask qdec interrupt */
        QDEC_INTMask(QDEC, QDEC_Y_CT_INT_MASK, ENABLE);

        /* Read direction & count */
        Y_Axis_Direction = QDEC_GetAxisDirection(QDEC, QDEC_AXIS_Y);
        Y_Axis_Count = QDEC_GetAxisCount(QDEC, QDEC_AXIS_Y);

        /* clear qdec interrupt flags */
        QDEC_ClearINTPendingBit(QDEC, QDEC_CLR_NEW_CT_Y);
        /* Unmask qdec interrupt */
        QDEC_INTMask(QDEC, QDEC_Y_CT_INT_MASK, DISABLE);
    }
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
