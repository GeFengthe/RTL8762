/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     ir_driver_rx.c
* @brief    This file provides demo code of ir.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "ir_driver_rx.h"

#include "ir_rx_loop_queue.h"

#if (IR_FUN_EN && IR_FUN_LEARN_EN)

/* Globals ------------------------------------------------------------------*/
/* Function pointer used to send event to application from IR learn interrupt handler. */
pFn_IR_RX_Handler_CB_t pFn_IR_RX_Handler_CB = NULL;

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_ir_rx_init(void)
{
    Pad_Config(IR_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);

    Pinmux_Config(IR_RX_PIN, IRDA_RX);
}


/**
  * @brief  Initialize ir peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_ir_rx_init(void)
{
    /* Enable IR clock */
    RCC_PeriphClockCmd(APBPeriph_IR, APBPeriph_IR_CLOCK, DISABLE);
    RCC_PeriphClockCmd(APBPeriph_IR, APBPeriph_IR_CLOCK, ENABLE);

    /* Initialize IR */
    IR_InitTypeDef IR_InitStruct;
    IR_StructInit(&IR_InitStruct);
    /* IR carrier freqency is 38KHz */
    IR_InitStruct.IR_Freq               = IR_LEARN_FREQ;
    /* Duty ratio = 1/IR_DutyCycle */
    IR_InitStruct.IR_DutyCycle          = 2;
    /* IR receiveing mode */
    IR_InitStruct.IR_Mode               = IR_MODE_RX;
    IR_InitStruct.IR_RxStartMode        = IR_RX_AUTO_MODE;
    /* Configure RX FIFO threshold level to trigger IR_INT_RF_LEVEL interrupt */
    IR_InitStruct.IR_RxFIFOThrLevel     = IR_RX_FIFO_THR_LEVEL;
    /* Discard the lastest received dta if RX FIFO is full */
    IR_InitStruct.IR_RxFIFOFullCtrl     = IR_RX_FIFO_FULL_DISCARD_NEWEST;
    /* Configure trigger type */
    IR_InitStruct.IR_RxTriggerMode      = IR_RX_FALL_EDGE;
    /* If high to low or low to high transition time <= 50ns,Filter out it. */
    IR_InitStruct.IR_RxFilterTime       = IR_RX_FILTER_TIME_200ns;
    /* IR_RX_Count_Low_Level is counting low level */
    IR_InitStruct.IR_RxCntThrType       = IR_RX_Count_High_Level;
    /* Configure RX counter threshold.You can use it to decide to stop receiving IR data */
    IR_InitStruct.IR_RxCntThr           = IR_LEARN_NO_WAVEFORM_TIME_MAX;
    IR_Init(&IR_InitStruct);

    IR_Cmd(IR_MODE_RX, ENABLE);
    IR_ClearRxFIFO();

    IR_INTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, ENABLE);
    IR_MaskINTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, DISABLE);

    /* Configure NVIC */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = IR_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
  * @brief  Enables or disables the specified interrupt.
  * @param  NewState: new state of the specified interrupt.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None.
  */
void int_ir_rx_config(FunctionalState state)
{
    if (state != DISABLE)
    {
        /* Enable interrupt */
        IR_INTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, ENABLE);
    }
    else
    {
        /* Disable interrupt */
        IR_INTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, DISABLE);
    }
}

/**
  * @brief  Register callback function to send events from IR learn interrupt handle to application layer.
  * @param  pFunc: callback function.
  * @return void
  */
void ir_rx_handler_cb(pFn_IR_RX_Handler_CB_t pFunc)
{
    pFn_IR_RX_Handler_CB = pFunc;
}

/**
  * @brief  IR interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void IR_RX_Handler(void)
{
    uint16_t data_len = 0;
    uint8_t ir_learn_end_flag = false;

    /* Mask IR all interrupt */
    IR_MaskINTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, ENABLE);

    /* Received interrupt */
    if (IR_GetINTStatus(IR_INT_RF_LEVEL) == SET)
    {
        data_len = IR_GetRxDataLen();
        ir_loop_queue_data_in(&IR_RX_Queue, data_len);
        IR_ClearINTPendingBit(IR_INT_RF_LEVEL_CLR);
    }

    /* Stop to receive IR data */
    if (IR_GetINTStatus(IR_INT_RX_CNT_THR) == SET)
    {
        /* Read remaining data */
        data_len = IR_GetRxDataLen();
        ir_loop_queue_data_in(&IR_RX_Queue, data_len);
        IR_ClearINTPendingBit(IR_INT_RX_CNT_THR_CLR);

        /* Send ir learn end signal. */
        //Add application code here
        ir_learn_end_flag = true;
    }

    if (pFn_IR_RX_Handler_CB)
    {
        pFn_IR_RX_Handler_CB(ir_learn_end_flag);
    }

    /* Unmask IR all interrupt */
    IR_MaskINTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, DISABLE);

}

#endif

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/

