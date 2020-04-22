/****************************************************************************************************//**
 * @file     debug_platform.h
 *
 * @brief    Provide some debug APIs.
 *
 * @version  v0.1
 * @date     2018-04-16
 *
 * @note
 *******************************************************************************************************/
#ifndef _DEBUG_PLATFORM_H_
#define _DEBUG_PLATFORM_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void DebugTaskHang(void);
void DLPSDebugHelpFunc(void);
//create debug timer after jump app main
void dlps_debug_swtimer_init(void);


#ifdef __cplusplus
}
#endif

#endif /*_APPLICATION_H_*/

/******************* (C) COPYRIGHT 2015 Realtek Semiconductor Corporation *****END OF FILE****/
