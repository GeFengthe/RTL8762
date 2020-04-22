/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file
* @brief
* @details
* @author
* @date
* @version  v0.1
*********************************************************************************************************
*/
#include <os_mem.h>
#include "list_newtimer.h"
#include "rtl876x_nvic.h"
#include "rtl876x_rtc.h"
#include "rtl876x_pinmux.h"

/**-------RTC -----*/
#define RTC_PRESCALER_VALUE             0
#define RTC_COMP_INDEX                  1
#define RTC_INT_CMP_1                   RTC_INT_CMP1
#define TICK_VALUE                      (32)//1ms /*Interal 32k accuracy*/

#define MAX_NUMBER_OF_NEWTIMER_MESSAGE     2
#define NEWTIMER_TASK_STACK_SIZE           1024*1
#define NEWTIMER_TASK_PRIORITY                  3
#define DLPS_THRESHOLD                          8*32
#define MAX_RTC_Counter               ( uint32_t )0xffffffUL

#define tmrCOMMAND_START_DONT_TRACE             ( ( BaseType_t ) 0 )
#define tmrCOMMAND_START                        ( ( BaseType_t ) 1 )
#define tmrCOMMAND_RESET                        ( ( BaseType_t ) 2 )
#define tmrCOMMAND_STOP                         ( ( BaseType_t ) 3 )
#define tmrCOMMAND_CHANGE_PERIOD                ( ( BaseType_t ) 4 )
#define tmrCOMMAND_DELETE                       ( ( BaseType_t ) 5 )
#define tmrFIRST_FROM_ISR_COMMAND               ( ( BaseType_t ) 6 )
#define tmrCOMMAND_START_FROM_ISR               ( ( BaseType_t ) 6 )
#define tmrCOMMAND_RESET_FROM_ISR               ( ( BaseType_t ) 7 )
#define tmrCOMMAND_STOP_FROM_ISR                ( ( BaseType_t ) 8 )
#define tmrCOMMAND_CHANGE_PERIOD_FROM_ISR       ( ( BaseType_t ) 9 )


#define pdFALSE   false
#define pdTRUE    true

typedef void *TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t xTimer);

extern TickType_t newTickCount;
extern void *newTimerQueue;
extern uint32_t rtlEnterCritical(void);
extern void rtlExitCritical(uint32_t ulSavedInterruptStatus);
extern void *pvPortMalloc(RAM_TYPE ramType, size_t xSize);
extern void vPortFree(void *pv);

uint8_t CheckDiff(uint32_t now, uint32_t next, uint32_t *diff);
bool DlpsEnterCheck(void);
void *newTimerGetTimerID(const TimerHandle_t xTimer);
BaseType_t newTimerIsTimerActive(TimerHandle_t xTimer);

BaseType_t newTimerCreateTimerTask(void);

TimerHandle_t newxTimerCreate(const char *const pcTimerName,  TickType_t xTimerPeriodInTicks,
                              const UBaseType_t uxAutoReload, void *const pvTimerID,
                              TimerCallbackFunction_t
                              pxCallbackFunction);   /*lint !e971 Unqualified char types are allowed for strings and single characters only. */

BaseType_t newTimerGenericCommand(TimerHandle_t xTimer, const BaseType_t xCommandID,
                                  const TickType_t xOptionalValue);

static void newProcessExpiredTimer(const TickType_t xNextExpireTime, const TickType_t xTimeNow);

static void newtimer_main_task(void *pvParameters);

static void newProcessTimerOrBlockTask(const TickType_t xNextExpireTime, BaseType_t xListWasEmpty);

static TickType_t newGetNextExpireTime(BaseType_t *const pxListWasEmpty);

static TickType_t newSampleTimeNow(BaseType_t *const pxTimerListsWereSwitched);

static void newProcessReceivedCommands(void);

void RTC_INIT(void);

#define newxTimerStart( xTimer ) newTimerGenericCommand( ( xTimer ), tmrCOMMAND_START, (RTC_GetCounter()) )  //可以像os_timer_start 包一层，xTicksToWait会有问题
#define newxTimerStop( xTimer ) newTimerGenericCommand( ( xTimer ), tmrCOMMAND_STOP, 0U )     //同上
#define newxTimerDelete( xTimer ) newTimerGenericCommand( ( xTimer ), tmrCOMMAND_DELETE, 0U )     //同上
#define newxTimerChangePeriod( xTimer, xNewPeriod ) newTimerGenericCommand( ( xTimer ), tmrCOMMAND_CHANGE_PERIOD, ( xNewPeriod ) )   //同上


