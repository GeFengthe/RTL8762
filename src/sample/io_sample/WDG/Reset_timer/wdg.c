/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     wdg.c
* @brief    This file provides demo code of WDG.
            Not feeding the dog before the wdg timer timeout,then reset system.
            Press KEY0 to feed dog, the timer restart.Then it does not reset system.
* @details
* @author   yuan
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "wdg.h"


/**
  * @brief  Initialize WDG.
  * @param  No parameter.
  * @return void
  */
void driver_wdg_init(void)
{
    WDG_ClockEnable();
    /* WDG timing time = ((77+1)/32000)*( 2^(11+1) - 1) , about 10S
     * Reset mode following:
     *      INTERRUPT_CPU: interrupt CPU
     *      RESET_ALL_EXCEPT_AON: reset all except aon
     *      RESET_CORE_DOMAIN: reset core domain
     *      RESET_ALL: reset all
     */
    WDG_Config(77, 11, RESET_ALL);
    WDG_Enable();
}

/**
  * @brief  Feeding dog.
  * @param  No parameter.
  * @return void
  */
void wdg_feed(void)
{
    WDG_Restart();
}


/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/


