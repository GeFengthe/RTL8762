/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     mouse_qdecoder.c
* @brief
* @details
* @author   parker
* @date     2018-04-28
* @version  v1.0
*********************************************************************************************************
*/
#include <board.h>
#include <string.h>
#include <trace.h>
#include <app_msg.h>
#include <app_task.h>
#include <qdec.h>
#include <os_timer.h>
#include <rtl876x.h>
#include <rtl876x_qdec.h>
#include <rtl876x_rcc.h>
#include <rtl876x_nvic.h>
#include <rtl876x_gpio.h>
#include <rtl876x_pinmux.h>



qdec_ctx_t qdec_ctx;
uint8_t qdecoder_a_status = 0;
uint8_t qdecoder_b_status = 0;
uint8_t pre_a_status = 0;
uint8_t pre_b_status = 0;


void app_handle_io_msg(T_IO_MSG io_msg)
{
    uint16_t msg_type = io_msg.type;

    switch (msg_type)
    {
    case IO_MSG_TYPE_QDECODE:
        {
            handle_qdecoder_event(io_msg);
        }
        break;
    case IO_MSG_TYPE_GPIO:
        break;
    default:
        break;
    }
}
/**
* @brief   qdec_ctx_clear
* @return  void
*/
void qdec_ctx_clear(void)
{
    memset(&qdec_ctx, 0, sizeof(qdec_ctx_t));
}

/**
* @brief   qdecoder pinmux config
* @return  void
*/
void qdecoder_pinmux_config(void)
{
    Pinmux_Config(QDEC_A_Y, qdec_phase_a_y);
    Pinmux_Config(QDEC_B_Y, qdec_phase_b_y);
}

/**
* @brief   qdecoder pad config
* @return  void
*/
void qdecoder_pad_config(void)
{
    Pad_Config(QDEC_A_Y, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(QDEC_B_Y, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
}

/**
  * @brief  read qdecoder init phase
  * @param   No parameter.
  * @return  void
  */
void qdecoder_init_status_read(void)
{
    Pinmux_Config(QDEC_A_Y, DWGPIO);
    Pinmux_Config(QDEC_B_Y, DWGPIO);
    Pad_Config(QDEC_A_Y, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(QDEC_B_Y, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);

    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
    GPIO_InitTypeDef MouseButton_Param;
    MouseButton_Param.GPIO_Pin  = GPIO_GetPin(QDEC_A_Y) | GPIO_GetPin(QDEC_B_Y);
    MouseButton_Param.GPIO_Mode = GPIO_Mode_IN;
    MouseButton_Param.GPIO_ITCmd = DISABLE;
    GPIO_Init(&MouseButton_Param);
    qdecoder_a_status = GPIO_ReadInputDataBit(GPIO_GetPin(QDEC_A_Y));
    qdecoder_b_status = GPIO_ReadInputDataBit(GPIO_GetPin(QDEC_B_Y));

    qdecoder_pinmux_config();
}

/**
  * @brief  Initialize Qdecoder peripheral.
  * @param   No parameter.
  * @return  void
  */
void driver_qdec_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_QDEC, APBPeriph_QDEC_CLOCK, ENABLE);

    QDEC_InitTypeDef qdecInitStruct;
    QDEC_StructInit(&qdecInitStruct);
    qdecInitStruct.axisConfigY       = ENABLE;
    qdecInitStruct.debounceEnableY   = Debounce_Enable;
    qdecInitStruct.initPhaseY = (qdecoder_a_status << 1) | qdecoder_b_status;
    qdecInitStruct.counterScaleY = CounterScale_1_Phase;
    QDEC_Init(QDEC, &qdecInitStruct);
    QDEC_INTConfig(QDEC, QDEC_Y_INT_NEW_DATA, ENABLE);
    QDEC_INTMask(QDEC, QDEC_Y_CT_INT_MASK, DISABLE);
    QDEC_Cmd(QDEC, QDEC_AXIS_Y, ENABLE);

    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = qdecode_IRQn;
    nvic_init_struct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&nvic_init_struct);

    qdec_ctx_clear();
}
/**
* @brief  Qdecode interrupt handler function.
* @param   No parameter.
* @return  void
*/
void Qdecode_Handler(void)
{
    if (QDEC_GetFlagState(QDEC, QDEC_FLAG_NEW_CT_STATUS_Y) == SET)
    {
        /* Mask qdec interrupt */
        QDEC_INTMask(QDEC, QDEC_Y_CT_INT_MASK, ENABLE);
        /* Read direction & count */
        qdec_ctx.dir = QDEC_GetAxisDirection(QDEC, QDEC_AXIS_Y);
        qdec_ctx.cur_ct = QDEC_GetAxisCount(QDEC, QDEC_AXIS_Y);

        T_IO_MSG qdecoder_msg;
        qdecoder_msg.type = IO_MSG_TYPE_QDECODE;
        qdecoder_msg.subtype = qdec_ctx.dir;
        qdecoder_msg.u.param = 1;
        app_send_msg_to_apptask(&qdecoder_msg);
        qdec_ctx.pre_ct = qdec_ctx.cur_ct;

        /* clear qdec interrupt flags */
        QDEC_ClearINTPendingBit(QDEC, QDEC_CLR_NEW_CT_Y);
        /* Unmask qdec interrupt */
        QDEC_INTMask(QDEC, QDEC_Y_CT_INT_MASK, DISABLE);
    }
}

/**
* @brief   send qdecoder data
* @return  void
*/
void handle_qdecoder_event(T_IO_MSG qdecoder_msg)
{
    uint16_t direction = qdecoder_msg.subtype;
    int16_t delta = (uint16_t)qdecoder_msg.u.param;

    /* direction:1--up; 0-- down */
    if (!direction)
    {
        delta = -delta;
    }

//    APP_PRINT_INFO1("Qdecoder delta value: %d\n\n", delta);
    DBG_DIRECT("%d", delta);
}
