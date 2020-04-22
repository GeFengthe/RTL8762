/**
*****************************************************************************************
*     Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      link_mgr.c
   * @brief     Link manager functions.
   * @author    berni
   * @date      2018-04-27
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2018 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <string.h>
#include "link_mgr.h"

/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @} */
/** @addtogroup  BT5_CENTRAL_SCAN_MGR
    * @{
    */
T_DEV_INFO dev_list[APP_MAX_DEVICE_INFO];
uint8_t dev_list_count = 0;
#if APP_RECOMBINE_ADV_DATA
T_EXT_ADV_DATA *ext_adv_data = NULL;
uint16_t fail_event_type;
uint8_t fail_bd_addr[GAP_BD_ADDR_LEN] = {0};
#endif
/** @} */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @addtogroup  BT5_CENTRAL_SCAN_MGR
    * @{
    */
/**
 * @brief   Add device information to device list.
 *
 * @param[in] bd_addr Peer device address.
 * @param[in] bd_type Peer device address type.
 * @retval true Success.
 * @retval false Failed, device list is full.
 */
bool link_mgr_add_device(uint8_t *bd_addr, uint8_t bd_type)
{
    /* If result count not at max */
    if (dev_list_count < APP_MAX_DEVICE_INFO)
    {
        uint8_t i;
        /* Check if device is already in device list*/
        for (i = 0; i < dev_list_count; i++)
        {
            if (memcmp(bd_addr, dev_list[i].bd_addr, GAP_BD_ADDR_LEN) == 0)
            {
                return true;
            }
        }

        /*Add addr to device list list*/
        memcpy(dev_list[dev_list_count].bd_addr, bd_addr, GAP_BD_ADDR_LEN);
        dev_list[dev_list_count].bd_type = bd_type;

        /*Increment device list count*/
        dev_list_count++;
    }
    else
    {
        return false;
    }
    return true;
}

/**
 * @brief Clear device list.
 * @retval void
 */
void link_mgr_clear_device_list(void)
{
    dev_list_count = 0;
}
/** @} */
