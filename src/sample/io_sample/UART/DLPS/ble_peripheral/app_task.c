/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      app_task.c
   * @brief     Routines to create App task and handle events & messages
   * @author    jane
   * @date      2017-06-02
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <gap_le.h>

#include "app.h"
#include "app_task.h"

#include "io_uart.h"

/** @defgroup  PERIPH_APP_TASK Peripheral App Task
    * @brief This file handles the implementation of application task related functions.
    *
    * Create App task and handle events & messages
    * @{
    */
/*============================================================================*
 *                              Macros
 *============================================================================*/
#define APP_TASK_PRIORITY             1         //!< Task priorities
#define APP_TASK_STACK_SIZE           256 * 4   //!<  Task stack size
#define MAX_NUMBER_OF_GAP_MESSAGE     0x20      //!<  GAP message queue size
#define MAX_NUMBER_OF_IO_MESSAGE      0x20      //!<  IO message queue size
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
extern void driver_init(void);

void app_main_task(void *p_param);

/**
  * @brief      Send msg to app task.
  * @param[in]  p_handle: The handle to the message queue being peeked.
  * @return     The status of the message queue peek.
  * @retval     true:Message queue was peeked successfully.
  * @retval     false:Message queue was failed to peek.
  */
bool app_send_msg_to_apptask(T_IO_MSG *p_msg)
{
    uint8_t event = EVENT_IO_TO_APP;

    if (os_msg_send(io_queue_handle, p_msg, 0) == false)
    {
        APP_PRINT_ERROR0("[app_task] app_send_msg_to_apptask: send_io_msg_to_app fail");
        return false;
    }
    if (os_msg_send(evt_queue_handle, &event, 0) == false)
    {
        APP_PRINT_ERROR0("[app_task] app_send_msg_to_apptask: send_evt_msg_to_app fail");
        return false;
    }
    return true;
}

/**
 * @brief  Initialize App task
 * @return void
 */
void app_task_init()
{
    os_task_create(&app_task_handle, "app", app_main_task, 0, APP_TASK_STACK_SIZE, APP_TASK_PRIORITY);
}

/**
  * @brief      App task to handle events & messages
  * @param[in]  p_param: Parameters sending to the task
  * @return     void
  */
uint8_t String_Buf[100];
void app_main_task(void *p_param)
{
    uint8_t event;
    os_msg_queue_create(&io_queue_handle, MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
    os_msg_queue_create(&evt_queue_handle, MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint8_t));

    gap_start_bt_stack(evt_queue_handle, io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);

    driver_init();

    /* Send demo string */
    uint8_t demo_str_len = 0;
    char *demo_str = "### Welcome to use UART demo ###\r\n";
    demo_str_len = strlen(demo_str);
    memcpy(String_Buf, demo_str, demo_str_len);
    uart_senddata_continuous(UART, String_Buf, demo_str_len);

    while (true)
    {
        if (os_msg_recv(evt_queue_handle, &event, 0xFFFFFFFF) == true)
        {
            if (event == EVENT_IO_TO_APP)
            {
                T_IO_MSG io_msg;
                if (os_msg_recv(io_queue_handle, &io_msg, 0) == true)
                {
                    app_handle_io_msg(io_msg);
                }
            }
            else
            {
                gap_handle_msg(event);
            }
        }
    }
}

/** @} */ /* End of group PERIPH_APP_TASK */


