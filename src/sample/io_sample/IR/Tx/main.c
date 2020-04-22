/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of ir tx.
* @details
* @author   yuan
* @date     2018-12-07
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_ir.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"

#include "trace.h"

#define IR_DATA_SIZE_MAX            75

#define IR_TX_PIN                   P2_5
#define IR_TX_FIFO_THR_LEVEL        2

/**
  * @brief  IR data structure definition
  */
typedef struct
{
    /* Unit of carrierFreq is KHz */
    uint8_t     CarrierFreq;
    uint32_t    DataBuf[IR_DATA_SIZE_MAX];
    uint16_t    DataLen;
} IR_Data_TypeDef;

/* Globals ------------------------------------------------------------------*/
IR_Data_TypeDef IR_TxData;
/* Number of data which has been sent */
uint8_t IR_TX_Count = 0;

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_ir_init(void)
{
    Pad_Config(IR_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);

    Pinmux_Config(IR_TX_PIN, IRDA_TX);
}


/**
  * @brief  Initialize ir peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_ir_init(uint32_t vFreq)
{
    /* Enable ir clock */
    RCC_PeriphClockCmd(APBPeriph_IR, APBPeriph_IR_CLOCK, ENABLE);

    /* Initialize ir */
    IR_InitTypeDef IR_InitStruct;
    IR_StructInit(&IR_InitStruct);
    IR_InitStruct.IR_Freq           = vFreq;
    IR_InitStruct.IR_DutyCycle      = 2; /* !< 1/2 duty cycle */
    IR_InitStruct.IR_Mode           = IR_MODE_TX;
    IR_InitStruct.IR_TxInverse      = IR_TX_DATA_NORMAL;
    IR_InitStruct.IR_TxFIFOThrLevel = IR_TX_FIFO_THR_LEVEL;
    IR_Init(&IR_InitStruct);
    IR_Cmd(IR_MODE_TX, ENABLE);

    /* Configure nvic */
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
    /* Data to send */
    IR_TxData.CarrierFreq = 38;
    IR_TxData.DataLen = 67 + 1; //2+64+1;
    IR_TxData.DataBuf[0] =  0x80000000 | 0x156; //342 about 9ms
    IR_TxData.DataBuf[1] =  0x00000000 | 0xAB; //171 about 4.5ms
    for (uint16_t i = 2; i < IR_TxData.DataLen - 1;)
    {
        IR_TxData.DataBuf[i] =  0x80000000 | 0x15; //21  about 560us
        IR_TxData.DataBuf[i + 1] =  0x00000000 | 0x15; //21  about 565us
        i += 2;
    }
    IR_TxData.DataBuf[30] =  0x80000000 | 0x15; //21  about 560us
    IR_TxData.DataBuf[31] =  0x00000000 | 0x40; //64  about 1690us
    IR_TxData.DataBuf[62] =  0x80000000 | 0x15; //21  about 560us
    IR_TxData.DataBuf[63] =  0x00000000 | 0x40; //64  about 1690us
    IR_TxData.DataBuf[64] =  0x80000000 | 0x15; //21  about 560us
    IR_TxData.DataBuf[65] =  0x00000000 | 0x40; //64  about 1690us
    IR_TxData.DataBuf[66] =  0x80000000 | 0x15; //21  about 560us
    IR_TxData.DataBuf[IR_TxData.DataLen - 1] =  0x00000000 | 0x15;

    board_ir_init();
    driver_ir_init(IR_TxData.CarrierFreq);

    /* Start to send data. */
    IR_SendBuf(IR_TxData.DataBuf, IR_TX_FIFO_SIZE, DISABLE);
    /* Record number which has been sent */
    IR_TX_Count = IR_TX_FIFO_SIZE;

    /* Enable IR threshold interrupt. when TX FIFO offset <= threshold value, trigger interrupt*/
    IR_INTConfig(IR_INT_TF_LEVEL, ENABLE);
}

/**
  * @brief    Entry of app code
  * @return   int (To avoid compile warning)
  */
int main(void)
{
    __enable_irq();

    ir_demo();

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
  * @brief  IR interrupt handler function.
  * @param  No parameter.
  * @return void
  */
void IR_Handler(void)
{
    /* Mask IR interrupt */
    IR_MaskINTConfig(IR_INT_TF_LEVEL, ENABLE);

    /* Continue to send by interrupt */
    if (IR_GetINTStatus(IR_INT_TF_LEVEL) == SET)
    {
        /* The remaining data is larger than the TX FIFO length */
        if ((IR_TxData.DataLen - IR_TX_Count) >= IR_TX_FIFO_SIZE)
        {
            IR_SendBuf(IR_TxData.DataBuf + IR_TX_Count, (IR_TX_FIFO_SIZE - IR_TX_FIFO_THR_LEVEL), DISABLE);
            IR_TX_Count += (IR_TX_FIFO_SIZE - IR_TX_FIFO_THR_LEVEL);

            /* Clear threshold interrupt */
            IR_ClearINTPendingBit(IR_INT_TF_LEVEL_CLR);
        }
        else if ((IR_TxData.DataLen - IR_TX_Count) > 0)
        {
            /* The remaining data is less than the TX FIFO length */

            /*  Configure TX threshold level to zero and trigger interrupt when TX FIFO is empty */
            IR_SetTxThreshold(0);
            IR_SendBuf(IR_TxData.DataBuf + IR_TX_Count, IR_TxData.DataLen - IR_TX_Count, DISABLE);
            IR_TX_Count += (IR_TxData.DataLen - IR_TX_Count);

            /* Clear threshold interrupt */
            IR_ClearINTPendingBit(IR_INT_TF_LEVEL_CLR);
        }
        else
        {
            /* Tx completed */
            /* Disable IR tx empty interrupt */
            IR_INTConfig(IR_INT_TF_LEVEL, DISABLE);
            IR_TX_Count = 0;

            /* Clear threshold interrupt */
            IR_ClearINTPendingBit(IR_INT_TF_LEVEL_CLR);
        }
    }

    /* Unmask IR interrupt */
    IR_MaskINTConfig(IR_INT_TF_LEVEL, DISABLE);
}

