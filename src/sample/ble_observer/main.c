/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      main.c
   * @brief     Source file for BLE observer project, mainly used for initialize modules
   * @author    jane
   * @date      2017-06-12
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <stdlib.h>
#include <os_sched.h>
#include <string.h>
#include <app_task.h>
#include <trace.h>
#include <gap_le.h>
#include <gap_scan.h>
#include <observer_app.h>
#if F_BT_DLPS_EN
#include <dlps.h>
#include <rtl876x_io_dlps.h>
#endif


/** @defgroup  OB_DEMO_MAIN Observer Main
    * @brief Main file to initialize hardware and BT stack and start task scheduling
    * @{
    */

/*============================================================================*
 *                              Constants
 *============================================================================*/

/** @brief Default scan interval (units of 0.625ms, 0x10=2.5ms) */
#define DEFAULT_SCAN_INTERVAL     0x10
/** @brief Default scan window (units of 0.625ms, 0x10=2.5ms) */
#define DEFAULT_SCAN_WINDOW       0x10

/*============================================================================*
 *                              Functions
 *============================================================================*/
/**
  * @brief  Initialize scan related parameters
  * @return void
  */
void app_le_gap_init(void)
{
    /* Scan parameters */
    uint8_t  scan_mode = GAP_SCAN_MODE_PASSIVE;
    uint16_t scan_interval = DEFAULT_SCAN_INTERVAL;
    uint16_t scan_window = DEFAULT_SCAN_WINDOW;
    uint8_t  scan_filter_policy = GAP_SCAN_FILTER_ANY;
    uint8_t  scan_filter_duplicate = GAP_SCAN_FILTER_DUPLICATE_ENABLE;

    /* Set Scan parameters */
    le_scan_set_param(GAP_PARAM_SCAN_MODE, sizeof(scan_mode), &scan_mode);
    le_scan_set_param(GAP_PARAM_SCAN_INTERVAL, sizeof(scan_interval), &scan_interval);
    le_scan_set_param(GAP_PARAM_SCAN_WINDOW, sizeof(scan_window), &scan_window);
    le_scan_set_param(GAP_PARAM_SCAN_FILTER_POLICY, sizeof(scan_filter_policy),
                      &scan_filter_policy);
    le_scan_set_param(GAP_PARAM_SCAN_FILTER_DUPLICATES, sizeof(scan_filter_duplicate),
                      &scan_filter_duplicate);

    /* register gap message callback */
    le_register_app_cb(app_gap_callback);
}

/**
 * @brief    Contains the initialization of pinmux settings and pad settings
 * @note     All the pinmux settings and pad settings shall be initiated in this function,
 *           but if legacy driver is used, the initialization of pinmux setting and pad setting
 *           should be peformed with the IO initializing.
 * @return   void
 */
void board_init(void)
{

}

/**
 * @brief    Contains the initialization of peripherals
 * @note     Both new architecture driver and legacy driver initialization method can be used
 * @return   void
 */
void driver_init(void)
{

}

/**
 * @brief    Contains the power mode settings
 * @return   void
 */
void io_dlps_enter_cb(void)
{

}

void io_dlps_exit_cb(void)
{

}

void pwr_mgr_init(void)
{
#if F_BT_DLPS_EN
    DLPS_IORegUserDlpsEnterCb(io_dlps_enter_cb);
    DLPS_IORegUserDlpsExitCb(io_dlps_exit_cb);
    DLPS_IORegister();
    lps_mode_set(LPM_DLPS_MODE);
#endif
}

/**
 * @brief    Contains the initialization of all tasks
 * @note     There is only one task in BLE Observer APP, thus only one APP task is init here
 * @return   void
 */
void task_init(void)
{
    app_task_init();
}

/**
 * @brief    Entry of APP code
 * @return   int (To avoid compile warning)
 */
int main(void)
{
    extern uint32_t random_seed_value;
    srand(random_seed_value);
    board_init();
    le_gap_init(0);
    gap_lib_init();
    app_le_gap_init();
    pwr_mgr_init();
    task_init();
    os_sched_start();

    return 0;
}
/** @} */ /* End of group OB_DEMO_MAIN */


