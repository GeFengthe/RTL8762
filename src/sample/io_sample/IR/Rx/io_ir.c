/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     io_ir.c
* @brief    This file provides demo code of ir.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "io_ir.h"

#include "app_task.h"


/* Globals ------------------------------------------------------------------*/
IR_Data_TypeDef IR_Rx_Data;
/* Number of data which has been sent */
uint16_t IR_RX_Count = 0;

/**
  * @brief  Initialization ir global data.
  * @param  No parameter.
  * @return void
  */
void global_data_ir_init(void)
{
    /* Initialize adc k value! */
    APP_PRINT_INFO0("[io_ir]global_data_ir_init");
    memset(&IR_Rx_Data, 0, sizeof(IR_Rx_Data));
    IR_RX_Count = 0;
}

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_ir_init(void)
{
    Pad_Config(IR_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);

    Pinmux_Config(IR_RX_PIN, IRDA_RX);
}


/**
  * @brief  Initialize ir peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_ir_init(void)
{
    /* Enable IR clock */
    RCC_PeriphClockCmd(APBPeriph_IR, APBPeriph_IR_CLOCK, ENABLE);

    /* Initialize IR */
    IR_InitTypeDef IR_InitStruct;
    IR_StructInit(&IR_InitStruct);

    IR_InitStruct.IR_Freq               = 38;/* IR carrier freqency is 38KHz */
    IR_InitStruct.IR_Mode               = IR_MODE_RX;/* IR receiveing mode */
    IR_InitStruct.IR_RxStartMode        = IR_RX_AUTO_MODE;
    IR_InitStruct.IR_RxFIFOThrLevel     =
        IR_RX_FIFO_THR_LEVEL; /* Configure RX FIFO threshold level to trigger IR_INT_RF_LEVEL interrupt */
    IR_InitStruct.IR_RxFIFOFullCtrl     =
        IR_RX_FIFO_FULL_DISCARD_NEWEST;/* Discard the lastest received dta if RX FIFO is full */
    IR_InitStruct.IR_RxFilterTime       =
        IR_RX_FILTER_TIME_50ns;/* If high to low or low to high transition time <= 50ns,Filter out it. */
    IR_InitStruct.IR_RxTriggerMode      = IR_RX_FALL_EDGE;/* Configure trigger type */
    IR_InitStruct.IR_RxCntThrType       =
        IR_RX_Count_High_Level;/* IR_RX_Count_High_Level is counting high level */
    IR_InitStruct.IR_RxCntThr           =
        0x1F40;/* Configure RX counter threshold.You can use it to decide to stop receiving IR data */
    IR_Init(&IR_InitStruct);
    IR_Cmd(IR_MODE_RX, ENABLE);
    IR_ClearRxFIFO();

    /* Configure NVIC */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = IR_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

}

/**
  * @brief  Demo code of ir send data.
  * @param  No parameter.
  * @return Void
*/
void ir_demo(void)
{
    /* Enable IR threshold interrupt. when RX FIFO offset >= threshold value, trigger interrupt*/
    /* Enable IR counter threshold interrupt to stop receiving data */
    IR_INTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, ENABLE);
    IR_MaskINTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, DISABLE);
}

void io_handle_ir_msg(T_IO_MSG *io_ir_msg)
{
    IR_Data_TypeDef *p_buf = io_ir_msg->u.buf;
    for (uint16_t i = 0; i < p_buf->DataLen; i++)
    {
        APP_PRINT_INFO2("[io_ir]io_handle_ir_msg: IR RX data[%d] = 0x%x", i, p_buf->DataBuf[i]);
    }
    memset(&IR_Rx_Data, 0, sizeof(IR_Rx_Data));
    IR_RX_Count = 0;
}

/**
  * @brief  IR interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void IR_Handler(void)
{
    uint16_t len = 0;

    /* Mask IR all interrupt */
    IR_MaskINTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, ENABLE);

    /* Receive by interrupt */
    if (IR_GetINTStatus(IR_INT_RF_LEVEL) == SET)
    {
        len = IR_GetRxDataLen();
        IR_ReceiveBuf(IR_Rx_Data.DataBuf + IR_RX_Count, len);
        IR_Rx_Data.DataLen += len;
        IR_RX_Count += len;

        IR_ClearINTPendingBit(IR_INT_RF_LEVEL_CLR);
    }

    /* Stop to receive IR data */
    if (IR_GetINTStatus(IR_INT_RX_CNT_THR) == SET)
    {
        /* Read remaining data */
        len = IR_GetRxDataLen();
        IR_ReceiveBuf(IR_Rx_Data.DataBuf + IR_RX_Count, len);
        IR_Rx_Data.DataLen += len;
        IR_RX_Count += len;

        T_IO_MSG int_ir_msg;
        int_ir_msg.type = IO_MSG_TYPE_IR;
        int_ir_msg.subtype = 0;
        int_ir_msg.u.buf = (void *)(&IR_Rx_Data);
        if (false == app_send_msg_to_apptask(&int_ir_msg))
        {
            APP_PRINT_ERROR0("[io_ir]IR_INT_RX_CNT_THR: Send int_ir_msg failed!");
            //Add user code here!
            IR_ClearINTPendingBit(IR_INT_RX_CNT_THR_CLR);
            return;
        }
        IR_ClearINTPendingBit(IR_INT_RX_CNT_THR_CLR);
    }

    /* Unmask IR all interrupt */
    IR_MaskINTConfig(IR_INT_RF_LEVEL | IR_INT_RX_CNT_THR, DISABLE);
}

