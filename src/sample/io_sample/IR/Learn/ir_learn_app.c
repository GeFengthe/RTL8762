/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file         ir_learn_app.c
* @brief        This file provides application layer application code for how to use IR learin driver.
* @details
* @author       elliot chen
* @date         2018-7-24
* @version      v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "ir_learn_app.h"


#if (IR_FUN_EN && IR_FUN_LEARN_EN)
#include "os_timer.h"

/* Globals -------------------------------------------------------------------*/
typedef void *TimerHandle_t;

TimerHandle_t IR_Learn_Timer = 0;

IR_Learn_TypeDef IR_Learn_Packet;

extern bool app_send_msg_to_apptask(T_IO_MSG *p_msg);
#define ir_send_msg_to_apptask app_send_msg_to_apptask

/**
  * @brief  ir learn msg handle from ir learn driver isr.
  * @param  vIsEnd:0 FALSE, 1 TRUE
  * @return void
  */
static void ir_learn_send_msg_from_isr(uint8_t vIsEnd)
{
    //Add applcation code here which send msg from IR ISR to application layer to notify handling data
    T_IO_MSG bee_io_msg;
    bee_io_msg.type = IO_MSG_TYPE_IR;

    if (vIsEnd)
    {
        /* send ir learn data message to app */
        bee_io_msg.subtype = IO_MSG_TYPE_IR_LEARN_STOP;
    }
    else
    {
        /* send ir learn data message to app */
        bee_io_msg.subtype = IO_MSG_TYPE_IR_LEARN_DATA;
    }
    ir_send_msg_to_apptask(&bee_io_msg);
}

/**
  * @brief  Stop ir learn function.
  * @param  None
  * @return void
  */
static void ir_learn_exit(void)
{
    APP_PRINT_INFO0("[ir_learn_app]ir_learn_exit");

    /* Close IR learn */
    ir_learn_deinit();
}

/**
  * @brief  IR learn timer callback.
  * @param  TimerHandle_t pxTimer.
  * @return void.
  * @note   None.
  */
static void ir_learn_timer_callback(TimerHandle_t pxTimer)
{
    APP_PRINT_INFO0("[ir_learn_app]ir_learn_timer_callback: ir learn time out");
    ir_learn_exit();

    //Add application code here!!!
}

/**
 * @brief  ir learn timer init.
 *@param   void.
 * @return     void.
 * @note       none.
**/
void ir_learn_timer_init(void)
{
    APP_PRINT_INFO0("[ir_learn_app]ir_learn_timer_init");
    /* IR_Learn_Timer is used for ir learn */
    if (false == os_timer_create(&IR_Learn_Timer, "ir_learn_timer",  1, \
                                 IR_LEARN_TIMEOUT, false, ir_learn_timer_callback))
    {
        APP_PRINT_ERROR0("[ir_learn_app]ir_learn_timer_init: creat IR_Learn_Timer fail!");
    }
}

/**
* @brief  Application code for IR learn module init.
* @param   No parameter.
* @return  void
*/
void ir_learn_module_init(void)
{
    APP_PRINT_INFO0("[ir_learn_app]ir_learn_module_init");
    if (true == os_timer_start(&IR_Learn_Timer))
    {
        APP_PRINT_INFO0("[ir_learn_app]ir_learn_module_init: IR_Learn_Timer start success!");
    }
    else
    {
        APP_PRINT_ERROR0("[ir_learn_app]ir_learn_module_init: IR_Learn_Timer start failed!");
    }

    board_ir_learn_init();
    /* Initialize IR learn data structure */
    memset(&IR_Learn_Packet, 0, sizeof(IR_Learn_TypeDef));
    ir_trans_rx_handler_cb(ir_learn_send_msg_from_isr);
    ir_learn_init();
}

/**
  * @brief   Check if ir learn allow enter dlps.
  * @param   None
  * @return  bool: true   working;
  *                false  idle;
  */
bool ir_learn_check_dlps(void)
{
    return true;
}

/**
  * @brief  Application code for IR learn data process.
  * @param  IR sub type msg.
  * @return bool.
  *         ture: specific msg can be handle;
  *         false: ir msg can not be handled.
  */
bool ir_learn_handle_msg(T_IO_MSG *io_ir_msg)
{
    bool ret = false;
    uint16_t sub_type = io_ir_msg->subtype;

    if (sub_type == IO_MSG_TYPE_IR_LEARN_DATA)
    {
        IR_Learn_Status status = IR_LEARN_OK;
        status = ir_learn_decode(&IR_Learn_Packet);

        if (status == IR_LEARN_EXCEED_SIZE)
        {
            APP_PRINT_WARN0("[ir_learn_app]ir_learn_handle_msg: IR learn exceed size!");
            /* Close IR learn */
            ir_learn_deinit();
            ret = false;
        }
        else if (status == IR_LEARN_EXIT)
        {
            APP_PRINT_WARN0("[ir_learn_app]ir_learn_handle_msg: IR learn stop time error!");
            /* Close IR learn */
            ir_learn_deinit();
            ret = false;
        }
    }
    else if (sub_type == IO_MSG_TYPE_IR_LEARN_STOP)
    {
        /* Pick up the last ir data*/
        ir_learn_decode(&IR_Learn_Packet);
        /* Decode IR carrier freqency */
        ir_learn_freq(&IR_Learn_Packet);
        /* Data reduction */
        ir_learn_data_convert(&IR_Learn_Packet);
        /* Print decode result */
#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
        APP_PRINT_INFO1("[ir_learn_app]ir_learn_handle_msg:IO_MSG_TYPE_IR_LEARN_STOP, duty_cycle: %d/1000.",
                        (uint32_t)(IR_Learn_Packet.duty_cycle * 1000));
#endif
        APP_PRINT_INFO1("[ir_learn_app]ir_learn_handle_msg:IO_MSG_TYPE_IR_LEARN_STOP, frequence: %d.",
                        (uint32_t)(IR_Learn_Packet.freq * 1000));
        APP_PRINT_INFO1("[ir_learn_app]ir_learn_handle_msg:IO_MSG_TYPE_IR_LEARN_STOP, learn data lenth: %d.",
                        IR_Learn_Packet.buf_index);
        for (uint32_t i = 0; i < IR_Learn_Packet.buf_index; i++)
        {
            APP_PRINT_INFO2("[ir_learn_app]ir_learn_handle_msg:IO_MSG_TYPE_IR_LEARN_STOP,learn data%d: 0x%x.",
                            i, IR_Learn_Packet.ir_buf[i]);
        }
        ir_learn_exit();
        ret = true;

        // Add application code here!!!
        ir_learn_module_init();
    }
    return ret;
}

#endif /* IR_FUN_EN && IR_FUN_LEARN_EN */

/******************* (C) COPYRIGHT 2019 Realtek Semiconductor Corporation *****END OF FILE****/
