/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of qdecoder.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "rtl876x_nvic.h"
#include "board.h"
#include "qdec.h"
#include "app_task.h"
#include "trace.h"
#include "os_sched.h"

/**
 * @brief    Contains the initialization of pinmux settings and pad settings
 * @note     All the pinmux settings and pad settings shall be initiated in this function,
 *           but if legacy driver is used, the initialization of pinmux setting and pad setting
 *           should be peformed with the IO initializing.
 * @return   void
 */
void board_init(void)
{
    qdecoder_pinmux_config();
    qdecoder_pad_config();
}

/**
  * @brief  Initialize GPIO peripheral.
  * @param   No parameter.
  * @return  void
  */

void driver_init(void)
{
    qdecoder_init_status_read();
    driver_qdec_init();
}

int main(void)
{
    board_init();
    app_task_init();
    os_sched_start();

}
