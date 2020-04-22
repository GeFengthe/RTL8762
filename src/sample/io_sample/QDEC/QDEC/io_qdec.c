/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     io_qdec.c
* @brief    qdec demo
* @details
* @author   yuan
* @date     2019-01-15
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "io_qdec.h"

#include "trace.h"

#include "app_task.h"

/* Globals -------------------------------------------------------------------*/
QDEC_Data_TypeDef   Y_Axis_Data;

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

    QDEC_InitTypeDef QDEC_InitStruct;
    QDEC_StructInit(&QDEC_InitStruct);
    QDEC_InitStruct.axisConfigY       = ENABLE;
    QDEC_InitStruct.debounceEnableY   = Debounce_Enable;
    QDEC_Init(QDEC, &QDEC_InitStruct);
    QDEC_INTConfig(QDEC, QDEC_Y_INT_NEW_DATA, ENABLE);

    /** To debug, enable QDEC when the GAP stack is ready.
      * In app.c->app_handle_dev_state_evt()
      */
//    QDEC_Cmd(QDEC, QDEC_AXIS_Y, ENABLE);

    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = qdecode_IRQn;
    nvic_init_struct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&nvic_init_struct);

}

/**
  * @brief  Handle adc data function.
  * @param  No parameter.
  * @return void
  */
void io_handle_qdec_msg(T_IO_MSG *io_qdec_msg)
{
    QDEC_Data_TypeDef *p_buf = io_qdec_msg->u.buf;
    APP_PRINT_INFO2("[io_qdec]io_handle_qdec_msg: Y_Axis_Direction = %d,Y_Axis_Count = %d,",
                    p_buf->AxisDirection, p_buf->AxisCount);
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
        /* Mask qdec interrupt */
        QDEC_INTMask(QDEC, QDEC_Y_CT_INT_MASK, ENABLE);

        /* Read direction & count */
        Y_Axis_Data.AxisDirection = QDEC_GetAxisDirection(QDEC, QDEC_AXIS_Y);
        Y_Axis_Data.AxisCount = QDEC_GetAxisCount(QDEC, QDEC_AXIS_Y);

        T_IO_MSG int_qdec_msg;

        int_qdec_msg.type = IO_MSG_TYPE_QDECODE;
        int_qdec_msg.u.buf = (void *)&Y_Axis_Data;
        if (false == app_send_msg_to_apptask(&int_qdec_msg))
        {
            APP_PRINT_ERROR0("[io_qdec]Qdecode_Handler: Send int_qdec_msg failed!");
            //Add user code here!
            /* Clear qdec interrupt flags */
            QDEC_ClearINTPendingBit(QDEC, QDEC_CLR_NEW_CT_Y);
            return;
        }
        /* Clear qdec interrupt flags */
        QDEC_ClearINTPendingBit(QDEC, QDEC_CLR_NEW_CT_Y);
        /* Unmask qdec interrupt */
        QDEC_INTMask(QDEC, QDEC_Y_CT_INT_MASK, DISABLE);
    }
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
