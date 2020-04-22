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
#include <string.h>
#include <os_sched.h>
#include <os_msg.h>
#include <os_task.h>
#include "newtimers.h"
#include "trace.h"


typedef struct tmrTimerControl
{
    const char
    *pcTimerName;       /*<< Text name.  This is not used by the kernel, it is included simply to make debugging easier. */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    ListItem_t
    xTimerListItem;     /*<< Standard linked list item as used by all kernel features for event management. */
    TickType_t              xTimerPeriodInTicks;/*<< How quickly and often the timer expires. */
    UBaseType_t
    uxAutoReload;       /*<< Set to pdTRUE if the timer should be automatically restarted once expired.  Set to pdFALSE if the timer is, in effect, a one-shot timer. */
    void
    *pvTimerID;         /*<< An ID to identify the timer.  This allows the timer to be identified when the same callback is used for multiple timers. */
    TimerCallbackFunction_t
    pxCallbackFunction; /*<< The function that will be called when the timer expires. */
} xTIMER;

typedef xTIMER newTimer_t;

/* The definition of messages that can be sent and received on the timer queue.
Two types of message can be queued - messages that manipulate a software timer,
and messages that request the execution of a non-timer related callback.  The
two message types are defined in two separate structures, xTimerParametersType
and xCallbackParametersType respectively. */
typedef struct tmrTimerParameters
{
    TickType_t
    xMessageValue;      /*<< An optional value used by a subset of commands, for example, when changing the period of a timer. */
    newTimer_t            *pxTimer;            /*<< The timer to which the command will be applied. */
} TimerParameter_t;

typedef struct tmrTimerQueueMessage
{
    BaseType_t          xMessageID;         /*<< The command being sent to the timer service task. */
    union
    {
        TimerParameter_t xTimerParameters;

        /* Don't include xCallbackParameters if it is not going to be used as
        it makes the structure (and therefore the timer queue) larger. */
    } u;
} DaemonTaskMessage_t;

static List_t newActiveTimerList1;
static List_t newActiveTimerList2;
static List_t *newCurrentTimerList;
static List_t *newOverflowTimerList;
void *newTimerQueue = NULL;
void *newTimer_task_handle;

TickType_t newTickCount = (TickType_t) 0U;
TickType_t LastCount = (TickType_t) 0U;
TickType_t Tick_add = (TickType_t) 0U;
TickType_t Tick_fix = 0;
TickType_t xNextExpireForSwitch = 0;
static void newSwitchTimerLists(void);
static BaseType_t newInsertTimerInActiveList(newTimer_t *const pxTimer,
                                             const TickType_t xNextExpiryTime, const TickType_t xTimeNow, const TickType_t xCommandTime,
                                             bool need_check);
bool CheckFirstTimer(newTimer_t *const pxTimer);

BaseType_t newTimerCreateTimerTask(void)
{
    BaseType_t xReturn = pdFALSE;

    if (newTimerQueue == NULL)
    {
        vListInitialise(&newActiveTimerList1);
        vListInitialise(&newActiveTimerList2);
        newCurrentTimerList = &newActiveTimerList1;
        newOverflowTimerList = &newActiveTimerList2;
        os_msg_queue_create(&newTimerQueue, MAX_NUMBER_OF_NEWTIMER_MESSAGE, sizeof(DaemonTaskMessage_t));
    }

    xReturn = os_task_create(&newTimer_task_handle, "newtimer", newtimer_main_task, 0,
                             NEWTIMER_TASK_STACK_SIZE, NEWTIMER_TASK_PRIORITY);

    RTC_INIT();

    return xReturn;
}

TimerHandle_t newxTimerCreate(const char *const pcTimerName,  TickType_t xTimerPeriodInTicks,
                              const UBaseType_t uxAutoReload, void *const pvTimerID,
                              TimerCallbackFunction_t
                              pxCallbackFunction)   /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
{
    newTimer_t *pxNewTimer;

    /* Allocate the timer structure. */
    if (xTimerPeriodInTicks == (TickType_t) 0U)
    {
        pxNewTimer = NULL;
    }
    else
    {
        pxNewTimer = (newTimer_t *) pvPortMalloc(RAM_TYPE_DATA_ON, sizeof(newTimer_t));

        if (pxNewTimer != NULL)
        {
            /* Initialise the timer structure members using the function parameters. */
            pxNewTimer->pcTimerName = pcTimerName;
            pxNewTimer->xTimerPeriodInTicks = xTimerPeriodInTicks * 32;
            pxNewTimer->uxAutoReload = uxAutoReload;
            pxNewTimer->pvTimerID = pvTimerID;
            pxNewTimer->pxCallbackFunction = pxCallbackFunction;
            vListInitialiseItem(&(pxNewTimer->xTimerListItem));

        }
    }

    /* 0 is not a valid value for xTimerPeriodInTicks. */

    return (TimerHandle_t) pxNewTimer;
}
/*-----------------------------------------------------------*/

BaseType_t newTimerGenericCommand(TimerHandle_t xTimer, const BaseType_t xCommandID,
                                  const TickType_t xOptionalValue)
{
    BaseType_t xReturn = pdFALSE;
    newTimer_t *pxTimer;
    DaemonTaskMessage_t xMessage;
    BaseType_t xTimerListsWereSwitched, xListWasEmpty;
    TickType_t xTimeNow;
    TickType_t xExpireTime, xNextExpireTime;
    uint32_t saved_interrupt_status;
    uint32_t temp_u32 = 0;
    uint32_t next_timeout = 0;
    bool current_timer_is_not_first = false;

    /* Send a message to the timer service task to perform a particular action
    on a particular timer definition. */
    if (newTimerQueue != NULL)
    {
        /* Send a command to the timer service task to start the xTimer timer. */
        xMessage.xMessageID = xCommandID;
        xMessage.u.xTimerParameters.xMessageValue = xOptionalValue;
        xMessage.u.xTimerParameters.pxTimer = (newTimer_t *) xTimer;

        if (xMessage.xMessageID >= (BaseType_t) 0 && (xTimer != NULL))
        {
            /* The messages uses the xTimerParameters member to work on a
            software timer. */
            pxTimer = xMessage.u.xTimerParameters.pxTimer;

            if (!CheckFirstTimer(pxTimer))
            {
                current_timer_is_not_first = true;
            }

            if (listIS_CONTAINED_WITHIN(NULL, &(pxTimer->xTimerListItem)) == pdFALSE)
            {
                /* The timer is in a list, remove it. */
                (void) uxListRemove(&(pxTimer->xTimerListItem));
            }

            xTimeNow = newSampleTimeNow(&xTimerListsWereSwitched);

            switch (xCommandID)
            {
            case tmrCOMMAND_START :
            case tmrCOMMAND_START_FROM_ISR :
            case tmrCOMMAND_RESET :
            case tmrCOMMAND_RESET_FROM_ISR :
            case tmrCOMMAND_START_DONT_TRACE:
                /* Start or restart a timer. */
                if (xNextExpireForSwitch > xTimeNow)
                {
                    saved_interrupt_status = rtlEnterCritical();

                    xNextExpireTime = xTimeNow + pxTimer->xTimerPeriodInTicks;
                    listSET_LIST_ITEM_VALUE(&(pxTimer->xTimerListItem), xNextExpireTime);
                    listSET_LIST_ITEM_OWNER(&(pxTimer->xTimerListItem), pxTimer);
                    vListInsert(newOverflowTimerList, &(pxTimer->xTimerListItem));

                    xNextExpireTime = listGET_ITEM_VALUE_OF_HEAD_ENTRY(newOverflowTimerList);

                    if (CheckDiff(xTimeNow, xNextExpireTime, &temp_u32))
                    {
                        next_timeout = temp_u32;
                        RTC_SetComp(RTC_COMP_INDEX, RTC_GetCounter() + next_timeout);
                    }
                    rtlExitCritical(saved_interrupt_status);
                }
                else
                {
                    if (newInsertTimerInActiveList(pxTimer,
                                                   xMessage.u.xTimerParameters.xMessageValue + pxTimer->xTimerPeriodInTicks, xTimeNow,
                                                   xMessage.u.xTimerParameters.xMessageValue, current_timer_is_not_first) == pdTRUE)
                    {
                        /* The timer expired before it was added to the active
                        timer list.  Process it now. */

                        pxTimer->pxCallbackFunction((TimerHandle_t) pxTimer);

                        if (pxTimer->uxAutoReload == (UBaseType_t) pdTRUE)
                        {

                            if (listIS_CONTAINED_WITHIN(NULL, &(pxTimer->xTimerListItem)) == pdFALSE)
                            {
                                /* The timer is in a list, remove it. */
                                (void) uxListRemove(&(pxTimer->xTimerListItem));
                            }
                            xTimeNow = newSampleTimeNow(&xTimerListsWereSwitched);
                            (void) newInsertTimerInActiveList(pxTimer, (xTimeNow + pxTimer->xTimerPeriodInTicks), xTimeNow,
                                                              xTimeNow, current_timer_is_not_first);
                        }
                    }
                }
                xReturn = pdTRUE;
                break;

            case tmrCOMMAND_STOP :
            case tmrCOMMAND_STOP_FROM_ISR :
                /* The timer has already been removed from the active list.
                There is nothing to do here. */
                if (!current_timer_is_not_first)
                {
                    xExpireTime = newGetNextExpireTime(&xListWasEmpty);
                    if (CheckDiff(xTimeNow, xExpireTime, &temp_u32))
                    {
                        xExpireTime = temp_u32;
                        saved_interrupt_status = rtlEnterCritical();
                        RTC_SetComp(RTC_COMP_INDEX, RTC_GetCounter() + xExpireTime);
                        rtlExitCritical(saved_interrupt_status);
                    }
                }
                xReturn = pdTRUE;
                break;

            case tmrCOMMAND_CHANGE_PERIOD :
            case tmrCOMMAND_CHANGE_PERIOD_FROM_ISR :
                pxTimer->xTimerPeriodInTicks = xMessage.u.xTimerParameters.xMessageValue * 32;
//                    configASSERT((pxTimer->xTimerPeriodInTicks > 0));
                if (xNextExpireForSwitch > xTimeNow)
                {
                    saved_interrupt_status = rtlEnterCritical();

                    xNextExpireTime = xTimeNow + pxTimer->xTimerPeriodInTicks;
                    listSET_LIST_ITEM_VALUE(&(pxTimer->xTimerListItem), xNextExpireTime);
                    listSET_LIST_ITEM_OWNER(&(pxTimer->xTimerListItem), pxTimer);
                    vListInsert(newOverflowTimerList, &(pxTimer->xTimerListItem));

                    xNextExpireTime = listGET_ITEM_VALUE_OF_HEAD_ENTRY(newOverflowTimerList);

                    if (CheckDiff(xTimeNow, xNextExpireTime, &temp_u32))
                    {
                        next_timeout = temp_u32;
                        RTC_SetComp(RTC_COMP_INDEX, RTC_GetCounter() + next_timeout);
                    }
                    rtlExitCritical(saved_interrupt_status);
                }
                else
                {
                    (void) newInsertTimerInActiveList(pxTimer, (xTimeNow + pxTimer->xTimerPeriodInTicks), xTimeNow,
                                                      xTimeNow, current_timer_is_not_first);
                }

                xReturn = pdTRUE;
                break;

            case tmrCOMMAND_DELETE:

                if (!current_timer_is_not_first)
                {
                    xExpireTime = newGetNextExpireTime(&xListWasEmpty);
                    if (CheckDiff(xTimeNow, xExpireTime, &temp_u32))
                    {
                        xExpireTime = temp_u32;
                        saved_interrupt_status = rtlEnterCritical();
                        RTC_SetComp(RTC_COMP_INDEX, RTC_GetCounter() + xExpireTime);
                        rtlExitCritical(saved_interrupt_status);
                    }
                }
                vPortFree(pxTimer);
                xReturn = pdTRUE;
                break;

            default :
                /* Don't expect to get here. */
                break;
            }
        }

    }

    return xReturn;
}
/*-----------------------------------------------------------*/

static void newProcessExpiredTimer(const TickType_t xNextExpireTime, const TickType_t xTimeNow)
{
    newTimer_t *const pxTimer = (newTimer_t *) listGET_OWNER_OF_HEAD_ENTRY(newCurrentTimerList);
    TickType_t xExpireTime;
    BaseType_t xListWasEmpty;
    uint32_t saved_interrupt_status;

    (void) uxListRemove(&(pxTimer->xTimerListItem));

    if (pxTimer->uxAutoReload == (UBaseType_t) pdTRUE)
    {
        (void) newInsertTimerInActiveList(pxTimer, (xTimeNow + pxTimer->xTimerPeriodInTicks), xTimeNow,
                                          xTimeNow, false);
    }
    else
    {
        uint32_t temp_u32 = 0;
        xExpireTime = newGetNextExpireTime(&xListWasEmpty);
        if (CheckDiff(xTimeNow, xExpireTime, &temp_u32))
        {
            xExpireTime = temp_u32;
            saved_interrupt_status = rtlEnterCritical();
            RTC_SetComp(RTC_COMP_INDEX, RTC_GetCounter() + xExpireTime);
            rtlExitCritical(saved_interrupt_status);
        }

    }

    /* Call the timer callback. */
    pxTimer->pxCallbackFunction((TimerHandle_t) pxTimer);
}

static void newtimer_main_task(void *pvParameters)
{
    TickType_t xNextExpireTime;
    BaseType_t xListWasEmpty, xTimerListsWereSwitched;
    TickType_t xTick, xTimeNow;

    /* Just to avoid compiler warnings. */
    (void) pvParameters;

    while (true)
    {
        if (os_msg_recv(newTimerQueue, &xTick, 0xFFFFFFFF) == true)
        {
            while (true)
            {
                xTimeNow = newSampleTimeNow(&xTimerListsWereSwitched);

                if (xTimerListsWereSwitched == pdFALSE)
                {
                    xNextExpireTime = newGetNextExpireTime(&xListWasEmpty);
                    if ((xListWasEmpty == pdFALSE) && (xNextExpireTime <= xTimeNow))
                    {
                        newProcessExpiredTimer(xNextExpireTime, xTimeNow);
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    xTimeNow = newSampleTimeNow(&xTimerListsWereSwitched);   //deal over list
                    xNextExpireTime = newGetNextExpireTime(&xListWasEmpty);
                    if ((xListWasEmpty == pdFALSE) && (xNextExpireTime <= xTimeNow))
                    {
                        newProcessExpiredTimer(xNextExpireTime, xTimeNow);
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }
}


static TickType_t newGetNextExpireTime(BaseType_t *const pxListWasEmpty)
{
    TickType_t xNextExpireTime;

    *pxListWasEmpty = listLIST_IS_EMPTY(newCurrentTimerList);
    if (*pxListWasEmpty == pdFALSE)
    {
        xNextExpireTime = listGET_ITEM_VALUE_OF_HEAD_ENTRY(newCurrentTimerList);
    }
    else
    {
        if ((listLIST_IS_EMPTY(newOverflowTimerList)) == pdFALSE)
        {
            xNextExpireTime = listGET_ITEM_VALUE_OF_HEAD_ENTRY(
                                  newOverflowTimerList);    //add over xNextExpireTime
        }
        else
        {
            xNextExpireTime = RTC_GetCounter() - 1;
        }

    }

    return xNextExpireTime;
}
/*-----------------------------------------------------------*/
static TickType_t newSampleTimeNow(BaseType_t *const pxTimerListsWereSwitched)
{
    TickType_t xTimeNow;
    static TickType_t xLastTime = (TickType_t) 0U;


    xTimeNow = RTC_GetCounter();

    if (xTimeNow < xLastTime)
    {
        newSwitchTimerLists();
        *pxTimerListsWereSwitched = pdTRUE;
    }
    else
    {
        *pxTimerListsWereSwitched = pdFALSE;
    }

    xLastTime = xTimeNow;

    return xTimeNow;
}

static BaseType_t newInsertTimerInActiveList(newTimer_t *const pxTimer,
                                             TickType_t xNextExpiryTime, const TickType_t xTimeNow, const TickType_t xCommandTime,
                                             bool need_check)
{
    BaseType_t xProcessTimerNow = pdFALSE;
    BaseType_t xListWasEmpty;
    TickType_t xExpireTime ;
    uint32_t temp_u32 = 0;
    uint32_t next_timeout = 0;
    uint32_t saved_interrupt_status;

    saved_interrupt_status = rtlEnterCritical();

    if (xNextExpiryTime > MAX_RTC_Counter)
    {
        xNextExpiryTime = xNextExpiryTime - MAX_RTC_Counter ;
    }

    listSET_LIST_ITEM_VALUE(&(pxTimer->xTimerListItem), xNextExpiryTime);
    listSET_LIST_ITEM_OWNER(&(pxTimer->xTimerListItem), pxTimer);

    if (xNextExpiryTime <= xTimeNow)
    {
        if ((xTimeNow - xCommandTime) >= pxTimer->xTimerPeriodInTicks)
        {
            xProcessTimerNow = pdTRUE;
        }
        else
        {
            vListInsert(newOverflowTimerList, &(pxTimer->xTimerListItem));
        }
    }
    else
    {
        if ((xTimeNow < xCommandTime) && (xNextExpiryTime >= xCommandTime))
        {
            xProcessTimerNow = pdTRUE;
        }
        else
        {
            vListInsert(newCurrentTimerList, &(pxTimer->xTimerListItem));
        }
    }
    rtlExitCritical(saved_interrupt_status);

    if (need_check)
    {
        if ((CheckFirstTimer(pxTimer) == false))
        {
            return xProcessTimerNow;
        }
    }
    xExpireTime = newGetNextExpireTime(&xListWasEmpty);

    if (CheckDiff(xTimeNow, xExpireTime, &temp_u32))
    {
        next_timeout = temp_u32;
        saved_interrupt_status = rtlEnterCritical();
        RTC_SetComp(RTC_COMP_INDEX, RTC_GetCounter() + next_timeout);
        rtlExitCritical(saved_interrupt_status);
    }

    return xProcessTimerNow;
}
/*-----------------------------------------------------------*/

static void newSwitchTimerLists(void)
{
    TickType_t xTimeNow, xNextExpireTime;
    List_t *pxTemp;
    newTimer_t *pxTimer;
    uint32_t temp_u32 = 0;
    uint32_t next_timeout = 0;
    uint32_t saved_interrupt_status;

    xTimeNow = RTC_GetCounter();
    while (listLIST_IS_EMPTY(newCurrentTimerList) == pdFALSE)
    {
        /* Remove the timer from the list. */
        pxTimer = (newTimer_t *) listGET_OWNER_OF_HEAD_ENTRY(newCurrentTimerList);

        xNextExpireForSwitch = listGET_ITEM_VALUE_OF_HEAD_ENTRY(newCurrentTimerList);

        (void) uxListRemove(&(pxTimer->xTimerListItem));

        pxTimer->pxCallbackFunction((TimerHandle_t) pxTimer);
        xNextExpireForSwitch = 0;

        if (pxTimer->uxAutoReload == (UBaseType_t) pdTRUE)
        {
            saved_interrupt_status = rtlEnterCritical();

            xNextExpireTime = xTimeNow + pxTimer->xTimerPeriodInTicks;
            listSET_LIST_ITEM_VALUE(&(pxTimer->xTimerListItem), xNextExpireTime);
            listSET_LIST_ITEM_OWNER(&(pxTimer->xTimerListItem), pxTimer);
            vListInsert(newOverflowTimerList, &(pxTimer->xTimerListItem));

            xNextExpireTime = listGET_ITEM_VALUE_OF_HEAD_ENTRY(newOverflowTimerList);  //need to set comp

            if (CheckDiff(xTimeNow, xNextExpireTime, &temp_u32))
            {
                next_timeout = temp_u32;
                RTC_SetComp(RTC_COMP_INDEX, RTC_GetCounter() + next_timeout);
            }

            rtlExitCritical(saved_interrupt_status);
        }
    }

    pxTemp = newCurrentTimerList;
    newCurrentTimerList = newOverflowTimerList;
    newOverflowTimerList = pxTemp;
}

BaseType_t newTimerIsTimerActive(TimerHandle_t xTimer)
{
    BaseType_t xTimerIsInActiveList;
    newTimer_t *pxTimer = (newTimer_t *) xTimer;

    uint32_t saved_interrupt_status;

    saved_interrupt_status = rtlEnterCritical();
    {
        /* Checking to see if it is in the NULL list in effect checks to see if
        it is referenced from either the current or the overflow timer lists in
        one go, but the logic has to be reversed, hence the '!'. */
        xTimerIsInActiveList = (BaseType_t) !(listIS_CONTAINED_WITHIN(NULL, &(pxTimer->xTimerListItem)));
    }
    rtlExitCritical(saved_interrupt_status);

    return xTimerIsInActiveList;
}

void *newTimerGetTimerID(const TimerHandle_t xTimer)
{
    uint32_t saved_interrupt_status;
    newTimer_t *const pxTimer = (newTimer_t *) xTimer;
    void *pvReturn;

    saved_interrupt_status = rtlEnterCritical();
    pvReturn = pxTimer->pvTimerID;
    rtlExitCritical(saved_interrupt_status);

    return pvReturn;
}
void RTC_Handler(void)
{
    if (RTC_GetINTStatus(RTC_INT_CMP_1) == SET)
    {
        os_msg_send(newTimerQueue, &newTickCount, 0);
        RTC_ClearCompINT(RTC_COMP_INDEX);
    }
}

void RTC_INIT(void)
{
    RTC_DeInit();
    RTC_SetPrescaler(RTC_PRESCALER_VALUE);
    RTC_MaskINTConfig(RTC_INT_CMP_1, DISABLE);
    RTC_CompINTConfig(RTC_INT_CMP_1, ENABLE);

    /* Config RTC interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /* Start RTC */
    RTC_SystemWakeupConfig(ENABLE);
    RTC_RunCmd(ENABLE);

    RTC_ResetCounter();
    newTickCount = 0;


}

uint8_t CheckDiff(uint32_t now, uint32_t next, uint32_t *diff)
{
    uint32_t temp_diff;
    if (next >= now)
    {
        temp_diff = next - now;
    }
    else
    {
        temp_diff = MAX_RTC_Counter + next - now;

        if (temp_diff > MAX_RTC_Counter)
        {
            return false;
        }
    }
    *diff = temp_diff;

    return true;
}

bool CheckFirstTimer(newTimer_t *const pxTimer)
{
    newTimer_t *pxTimerHead, *pxTimerHeadOver;
    pxTimerHead = (newTimer_t *) listGET_OWNER_OF_HEAD_ENTRY(newCurrentTimerList);
    pxTimerHeadOver = (newTimer_t *) listGET_OWNER_OF_HEAD_ENTRY(newOverflowTimerList);
    if ((pxTimerHead != pxTimer) && (pxTimerHeadOver != pxTimer))
    {
        return false;
    }
    return true;
}

bool DlpsEnterCheck(void)
{
    TickType_t xNextExpireTime, xNowTick;
    BaseType_t xListWasEmpty;
    xNextExpireTime = newGetNextExpireTime(&xListWasEmpty);
    xNowTick = RTC_GetCounter();
    if ((xNextExpireTime - xNowTick) <= DLPS_THRESHOLD)
    {
        return false;
    }

    return true;
}

