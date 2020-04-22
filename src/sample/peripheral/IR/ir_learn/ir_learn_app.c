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

/* Globals -------------------------------------------------------------------*/
#if IR_LEARN_MODE
typedef void *TimerHandle_t;

IR_LearnTypeDef IR_LearnPacket;
TimerHandle_t ir_learn_timer = NULL;

extern bool app_send_msg_to_apptask(T_IO_MSG *p_msg);
#define ir_send_msg_to_app app_send_msg_to_apptask

/**
  * @brief  initialization of pinmux settings and pad settings.
  * @param   No parameter.
  * @return  void
  */
void ir_learn_board_init(void)
{
    Pad_Config(IR_LEARN_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pinmux_Config(IR_LEARN_PIN, IRDA_RX);
}

/**
* @brief  Application code for IR learn key release handle.
* @param  uint32_t key_index_1;
*         uint32_t key_index_2.
* @return  void
*/
void ir_learn_exit(void)
{
    APP_PRINT_INFO0("[IR] ir mode exit.");

    /* Close IR learn */
    IR_Learn_DeInit();
}

/**
* @brief   Check if ir learn allow enter dlps.
* @param   void.
* @return  bool true   working;
*               false  not in working;
*/
bool ir_learn_check_dlps(void)
{
    return true;
}

/**
* @brief   ir learn msg handle from ir learn driver isr.
* @param   uint8_t isEnd .0 FALSE, 1 TRUE
* @return  bool   valid   true;
*                 invalid false;
*/
void ir_learn_msg_form_isr(uint8_t isEnd)
{
    //Add applcation code here which send msg from IR ISR to application layer to notify handling data
    T_IO_MSG bee_io_msg;
    bee_io_msg.type = IO_MSG_TYPE_IR;

    if (isEnd)
    {
        /*send ir learn data message to app*/
        bee_io_msg.subtype = IO_MSG_TYPE_IR_LEARN_STOP;
    }
    else
    {
        /*send ir learn data message to app*/
        bee_io_msg.subtype = IO_MSG_TYPE_IR_LEARN_DATA;
    }
    ir_send_msg_to_app(&bee_io_msg);
}

/**
 * @brief  ir learn timer callback.
 *@param   TimerHandle_t pxTimer.
 * @return     void.
 * @note       none.
**/
void ir_learn_timer_callback(TimerHandle_t pxTimer)
{
    APP_PRINT_INFO0("[IR] ir learn time out");
    ir_learn_exit();

    //add application code here!!!
}

/**
 * @brief  ir learn timer init.
 *@param   void.
 * @return     void.
 * @note       none.
**/
void ir_learn_init_timer(void)
{
    APP_PRINT_INFO0("[IR] init IR Learn timer");
    /*ir_learn_timer is used for ir learn*/
    if (false == os_timer_create(&ir_learn_timer, "ir_learn_timer",  1, \
                                 IR_LEARN_TIMEOUT, false, ir_learn_timer_callback))
    {
        APP_PRINT_ERROR0("[IR] timer creat failed!");
    }
}

/**
* @brief  Application code for IR learn module init.
* @param   No parameter.
* @return  void
*/
void ir_learn_module_init(void)
{
    if (true == os_timer_start(&ir_learn_timer))
    {
        APP_PRINT_INFO0("[IR] time out timer start success!");
    }
    else
    {
        APP_PRINT_ERROR0("[IR] time out timer start failed!");
    }

    ir_learn_board_init();
    /* Initialize IR learn data structure */
    memset(&IR_LearnPacket, 0, sizeof(IR_LearnTypeDef));
    DataTrans_RegisterIRLearnHandlerCB(ir_learn_msg_form_isr);
    IR_Learn_Init();
}

/**
* @brief  Application code for IR learn data process.
* @param  IR sub type msg.
* @return  bool.
*          ture   specific msg can be handle;
*          false  ir msg can not be handled.
*/
bool ir_learn_msg_proc(uint16_t msg_sub_type)
{
    bool ret = false;

    if (msg_sub_type == IO_MSG_TYPE_IR_LEARN_DATA)
    {
        IR_Learn_Status status = IR_LEARN_OK;
        status = IR_Learn_Decode(&IR_LearnPacket);

        if (status == IR_LEARN_EXCEED_SIZE)
        {
            APP_PRINT_INFO0("[IR] ERR: IR learn exceed size.");
            /* Close IR learn */
            IR_Learn_DeInit();
            ret = false;
        }
        else if (status == IR_LEARN_EXIT)
        {
            APP_PRINT_INFO0("[IR] ERR: IR learn stop time err.");
            /* Close IR learn */
            IR_Learn_DeInit();
            ret = false;
        }
    }
    else if (msg_sub_type == IO_MSG_TYPE_IR_LEARN_STOP)
    {
        /*pick up the last ir data*/
        IR_Learn_Decode(&IR_LearnPacket);
        /* Decode IR carrier freqency */
        IR_Learn_Freq(&IR_LearnPacket);
        /* Data reduction */
        IR_Learn_ConvertData(&IR_LearnPacket);
        ir_learn_exit();
        APP_PRINT_INFO1("[IR] IR learn stop msg, fre: %d.", (uint32_t)(IR_LearnPacket.freq * 1000));
        ret = true;

        // add application code here!!!
        //ir_learn_module_init();
    }
    return ret;
}

#endif /*end Micro IR_LEARN_MODE */

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
