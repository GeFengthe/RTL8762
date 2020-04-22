/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file
* @brief    Provide some debug APIs.
* @details
* @author
* @date
* @version  v0.1
*********************************************************************************************************
*/
#include "debug_platform.h"
#include "os_timer.h"
#include "rtl876x.h"
#include "trace.h"

/* Type definitions. */
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       short
#define portSTACK_TYPE  uint32_t
#define portBASE_TYPE   long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;


typedef uint32_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL

/*
 *To use volatile list structure members then add the following line to
 * FreeRTOSConfig.h (without the quotes):
 * "#define configLIST_VOLATILE volatile"
 */
#ifndef configLIST_VOLATILE
#define configLIST_VOLATILE
#endif /* configSUPPORT_CROSS_MODULE_OPTIMISATION */

/*
 * Definition of the only type of object that a list can contain.
 */
struct xLIST_ITEM
{
    //listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE         /*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
    configLIST_VOLATILE TickType_t
    xItemValue;          /*< The value being listed.  In most cases this is used to sort the list in descending order. */
    struct xLIST_ITEM *configLIST_VOLATILE
        pxNext;      /*< Pointer to the next ListItem_t in the list. */
    struct xLIST_ITEM *configLIST_VOLATILE
        pxPrevious;  /*< Pointer to the previous ListItem_t in the list. */
    void *pvOwner;                                      /*< Pointer to the object (normally a TCB) that contains the list item.  There is therefore a two way link between the object containing the list item and the list item itself. */
    void *configLIST_VOLATILE
    pvContainer;              /*< Pointer to the list in which this list item is placed (if any). */
    //listSECOND_LIST_ITEM_INTEGRITY_CHECK_VALUE            /*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
};
typedef struct xLIST_ITEM
    ListItem_t;                   /* For some reason lint wants this as two separate definitions. */

struct xMINI_LIST_ITEM
{
    //listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE         /*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
    configLIST_VOLATILE TickType_t xItemValue;
    struct xLIST_ITEM *configLIST_VOLATILE pxNext;
    struct xLIST_ITEM *configLIST_VOLATILE pxPrevious;
};
typedef struct xMINI_LIST_ITEM MiniListItem_t;

/*
 * Definition of the type of queue used by the scheduler.
 */
typedef struct xLIST
{
    //listFIRST_LIST_INTEGRITY_CHECK_VALUE              /*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
    configLIST_VOLATILE UBaseType_t uxNumberOfItems;
    ListItem_t *configLIST_VOLATILE
    pxIndex;            /*< Used to walk through the list.  Points to the last item returned by a call to listGET_OWNER_OF_NEXT_ENTRY (). */
    MiniListItem_t
    xListEnd;                            /*< List item that contains the maximum possible item value meaning it is always at the end of the list and is therefore used as a marker. */
    //listSECOND_LIST_INTEGRITY_CHECK_VALUE             /*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
} List_t;

/*
 * Definition of the queue used by the scheduler.
 * Items are queued by copy, not reference.  See the following link for the
 * rationale: http://www.freertos.org/Embedded-RTOS-Queues.html
 */
typedef struct QueueDefinition
{
    int8_t *pcHead;                 /*< Points to the beginning of the queue storage area. */
    int8_t *pcTail;                 /*< Points to the byte at the end of the queue storage area.  Once more byte is allocated than necessary to store the queue items, this is used as a marker. */
    int8_t *pcWriteTo;              /*< Points to the free next place in the storage area. */

    union                           /* Use of a union is an exception to the coding standard to ensure two mutually exclusive structure members don't appear simultaneously (wasting RAM). */
    {
        int8_t *pcReadFrom;         /*< Points to the last place that a queued item was read from when the structure is used as a queue. */
        UBaseType_t
        uxRecursiveCallCount;/*< Maintains a count of the number of times a recursive mutex has been recursively 'taken' when the structure is used as a mutex. */
    } u;

    List_t xTasksWaitingToSend;     /*< List of tasks that are blocked waiting to post onto this queue.  Stored in priority order. */
    List_t xTasksWaitingToReceive;  /*< List of tasks that are blocked waiting to read from this queue.  Stored in priority order. */

    volatile UBaseType_t uxMessagesWaiting;/*< The number of items currently in the queue. */
//    UBaseType_t
//    uxLength;           /*< The length of the queue defined as the number of items it will hold, not the number of bytes. */
//    UBaseType_t uxItemSize;         /*< The size of each items that the queue will hold. */

//    volatile BaseType_t
//    xRxLock;    /*< Stores the number of items received from the queue (removed from the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */
//    volatile BaseType_t
//    xTxLock;    /*< Stores the number of items transmitted to the queue (added to the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */

//#if ( configUSE_TRACE_FACILITY == 1 )
//    UBaseType_t uxQueueNumber;
//    uint8_t ucQueueType;
//#endif

//#if ( configUSE_QUEUE_SETS == 1 )
//    struct QueueDefinition *pxQueueSetContainer;
//#endif

} xQUEUE;

/* The old xQUEUE name is maintained above then typedefed to the new Queue_t
name below to enable the use of older kernel aware debuggers. */
typedef xQUEUE Queue_t;


extern void *app_task_handle;   //!< APP Task handle
extern void *evt_queue_handle;  //!< Event queue handle

void *xTimerDLPSDebug; //debug dlps timer handle

bool vTaskSwitchContext_imp(void)
{
    Queue_t *pxEventQueue = (Queue_t *)evt_queue_handle;
    Queue_t *pxTimerQueue = *(Queue_t **)0x00200e58; //xTimerQueue

    if (pxEventQueue) //after app event queue created
    {
        if (pxEventQueue->uxMessagesWaiting >= 10 ||
            pxTimerQueue->uxMessagesWaiting >= 4) /*app queue and timer queue threshold */
        {
            DBG_DIRECT("Debug");
            DBG_DIRECT("Event:%d  Timer:%d", pxEventQueue->uxMessagesWaiting, pxTimerQueue->uxMessagesWaiting);

            uint32_t *pxTopOfTimerStack =  *(uint32_t **)(*(uint32_t *)0x00200e5c); //timer, xTimerTaskHandle
            uint32_t *pxTopOfLowerStack =  *(uint32_t **)(*(uint32_t *)
                                                          0x0020176c); //lowerstack, low_task_handle
            uint32_t *pxTopOfUpperStack =  *(uint32_t **)(*(uint32_t *)0x00201494);//upperstack, bte.task_handle
            uint32_t *pxTopOfAPPStack =    *(uint32_t **)app_task_handle;/*app task handle*/

            uint32_t lr_val_timer = *(pxTopOfTimerStack + 8); //get LR value after entering interrupt
            uint32_t lr_val_lower = *(pxTopOfLowerStack + 8); //get LR value after entering interrupt
            uint32_t lr_val_upper = *(pxTopOfUpperStack + 8); //get LR value after entering interrupt
            uint32_t lr_val_app = *(pxTopOfAPPStack + 8);     //get LR value after entering interrupt

            //need check the value of the stack pointer has been adjusted or not

            if (lr_val_timer & 0x10)   //bit4 is not 0, not use fpu
            {
                pxTopOfTimerStack += 9;
            }
            else
            {
                pxTopOfTimerStack += 25;
            }

            if (lr_val_lower & 0x10)   //bit4 is not 0, not use fpu
            {
                pxTopOfLowerStack += 9;
            }
            else
            {
                pxTopOfLowerStack += 25;
            }

            if (lr_val_upper & 0x10)   //bit4 is not 0, not use fpu
            {
                pxTopOfUpperStack += 9;
            }
            else
            {
                pxTopOfUpperStack += 25;
            }

            if (lr_val_app & 0x10)   //bit4 is not 0, not use fpu
            {
                pxTopOfAPPStack += 9;
            }
            else
            {
                pxTopOfAPPStack += 25;
            }

            DBG_DIRECT("Timer:LR=0x%x PC=0x%x", *(pxTopOfTimerStack + 5), *(pxTopOfTimerStack + 6));
            DBG_DIRECT("Lower:LR=0x%x PC=0x%x", *(pxTopOfLowerStack + 5), *(pxTopOfLowerStack + 6));
            DBG_DIRECT("Upper:LR=0x%x PC=0x%x", *(pxTopOfUpperStack + 5), *(pxTopOfUpperStack + 6));
            DBG_DIRECT("App:LR=0x%x PC=0x%x", *(pxTopOfAPPStack + 5), *(pxTopOfAPPStack + 6));
        }
    }

    return false;
}

void DebugTaskHang(void)
{
    *((uint32_t *)0x00201238) = (uint32_t)vTaskSwitchContext_imp; //patch_vTaskSwitchContext
}

/**
  * @brief  DLPS debug function
  * @param
  * @retval
  */
void DLPSDebugHelpFunc(void)
{
    //print the address of the function which disallow to enter dlps
    extern uint32_t *DlpsRefuseReason;
    extern uint8_t DlpsErrorCode;
    APP_PRINT_INFO2("DLPS disallow function: 0x%x, error code 0x%x", DlpsRefuseReason, DlpsErrorCode);

    //the detail BT config information will be print in Rom code

    //tell how long has not been in DLPS mode
    static uint32_t wakeCountLast = 0;
    static uint32_t totalwakeuptimeLast = 0;
    static uint32_t totaltimeLast = 0;
    static uint32_t activeTime = 0;  //measure in seconds

    uint32_t wakeupCount = *(uint32_t *)0x00201b1c;  //sleep_mode_param.lps_wakeup_count
    uint32_t totalwakeuptime = *(uint32_t *)0x00201b20; //sleep_mode_param.lps_wakeup_time
    uint32_t totaltime = *(uint32_t *)0x00201b24; //sleep_mode_param.lps_total_time

    if (wakeCountLast == wakeupCount &&
        totalwakeuptimeLast == totalwakeuptime &&
        totaltimeLast == totaltime
       )
    {
        activeTime += 10;
        APP_PRINT_INFO1("NOT in DLPS for %d seconds", activeTime);
    }
    else
    {
        activeTime = 0;
    }
    wakeCountLast = wakeupCount;
    totalwakeuptimeLast = totalwakeuptime;
    totaltimeLast = totaltime;

    //User code can be added here
}

void vTimerDLPSDebugCallback(void *pxTimer)
{
    DLPSDebugHelpFunc();
}

/**
  * @brief  DLPS debug timer
  * @note   create timer after jump app main
  * @retval
  */
void dlps_debug_swtimer_init(void)
{
    bool retval = os_timer_create(&xTimerDLPSDebug, "xTimerDLPSDebug",  1, \
                                  10000/*10s*/, true, vTimerDLPSDebugCallback);

    if (retval)
    {
        os_timer_start(&xTimerDLPSDebug);
    }
}

