/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      main.c
* @brief     Entry of application
* @details
* @author    jeff
* @date      2016-12-01
* @version   v1.0
*
*/

#include "app_section.h"
#include "flash_device.h"
#include "gap_le.h"
#include "os_sched.h"
#include "app_task.h"
#include "dtm_app.h"

/** @defgroup  DTM_MAIN Direct Test Mode Main
  * @brief Main file to initialize parameters and register application message callback.
  * @{
  */

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
  * @brief  register gap message callback
  * @return void
  */
void app_le_gap_init(void)
{
    le_register_app_cb(app_gap_callback);
}

/**
  * @brief    Entry of APP code
  * @return   int (To avoid compile warning)
  */
int main(void)
{
    le_gap_init(0);
    gap_lib_init();
    app_le_gap_init();
    application_task_init();
    os_sched_start();

    return 0;
}

/** @} */ /* End of group DTM_MAIN */
