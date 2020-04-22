/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      app_task.c
* @brief     direct test mode task.
* @details
* @author    jeff
* @date      2016-12-01
* @version   v1.0
*
*/

#include <os_task.h>
#include <os_msg.h>
#include <gap.h>
#include "dtm_app.h"
#include <app_msg.h>

/** @defgroup  DTM_APP_TASK Direct Test Demo App Task
  * @brief This file create message queue and application task related functions.
  *
  * Create App task and handle events & messages
  * @{
  */

/*============================================================================*
 *                              Macros
 *============================================================================*/

#define APP_TASK_PRIORITY             1         //!< Task priorities
#define APP_TASK_STACK_SIZE           256 * 4   //!< Task stack size
#define MAX_NUMBER_OF_GAP_MESSAGE     0x20      //!< GAP message queue size
#define MAX_NUMBER_OF_IO_MESSAGE      0x20      //!< IO message queue size

#define MAX_NUMBER_OF_EVENT_MESSAGE   (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE)    //!< Event message queue size

/*============================================================================*
 *                              Variables
 *============================================================================*/

void *app_task_handle;   //!< APP Task handle
void *evt_queue_handle;  //!< Event queue handle
void *io_queue_handle;   //!< IO queue handle

/*============================================================================*
 *                              Functions
 *============================================================================*/

void dtm_task(void *pvParameters);
/**
 * @brief  Initialize App task
 * @return void
 */
void application_task_init()
{
    os_task_create(&app_task_handle, "dtm_task", dtm_task, 0, 512 * 4, 1);
}

/**
 * @brief        App task to handle events & messages
 * @param[in]    p_param Parameters sending to the task
 * @return       void
 */
void dtm_task(void *p_param)
{
    uint8_t event;
    os_msg_queue_create(&io_queue_handle, MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
    os_msg_queue_create(&evt_queue_handle, MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint8_t));

    gap_start_bt_stack(evt_queue_handle, io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);

    dtm_uart_init();

    while (true)
    {
        if (os_msg_recv(evt_queue_handle, &event, 0xffffffff) == true)
        {
            if (event == EVENT_IO_TO_APP)
            {
                T_IO_MSG io_msg;
                if (os_msg_recv(io_queue_handle, &io_msg, 0) == true)
                {
                    //dtm need not to handle io message
                }
            }
            else
            {
                gap_handle_msg(event);
            }
        }
    }
}

/** @} */ /* End of group DTM_APP_TASK */

