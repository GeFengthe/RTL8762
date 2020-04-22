/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      gpio_app.c
   * @brief     This file handles BLE scatternet application routines.
   * @author    ken_mei
   * @date      2018-04-12
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <trace.h>
#include <string.h>
#include <app_msg.h>
#include <gpio_app.h>
#include <os_timer.h>
#include <os_sched.h>
#include "rtl876x_gpio.h"
#include "board.h"

/** @addtogroup  GPIO_APP
    * @{
    */

/*============================================================================*
 *                              Variables
 *============================================================================*/


/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg)
{
    uint16_t msg_type = io_msg.type;

    switch (msg_type)
    {
    case IO_MSG_TYPE_BT_STATUS:
        {
            //app_handle_gap_msg(&io_msg);
        }
        break;
    case IO_MSG_TYPE_GPIO:
        if (io_msg.subtype == MSG_KEY_PRESS)
        {
            APP_PRINT_INFO0("KEY_PRESS");
            GPIO_SetBits(GPIO_GetPin(Output_Pin));
            os_delay(10);
            GPIO_ResetBits(GPIO_GetPin(Output_Pin));
        }
        else if (io_msg.subtype == MSG_KEY_RELEASE)
        {
            APP_PRINT_INFO0("KEY_RELEASE");
            GPIO_SetBits(GPIO_GetPin(Output_Pin));
            os_delay(10);
            GPIO_ResetBits(GPIO_GetPin(Output_Pin));
        }
        else
        {
        }
        break;
    default:
        break;
    }
}

/** @} */ /* End of group GPIO_APP */
