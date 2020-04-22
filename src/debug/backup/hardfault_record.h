/////////////////////////////////////////////////
//
// HardFault_Record, HardFault_Record save to flash
// Realtek CN3-BT, Raven Su
//
/////////////////////////////////////////////////

#ifndef HardFault_Record_h
#define HardFault_Record_h

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

typedef struct
{
    uint32_t stacked_r0;
    uint32_t stacked_r1;
    uint32_t stacked_r2;
    uint32_t stacked_r3;
    uint32_t stacked_r4;
    uint32_t stacked_r5;
    uint32_t stacked_r6;
    uint32_t stacked_r7;
    uint32_t stacked_r8;
    uint32_t stacked_r9;
    uint32_t stacked_r10;
    uint32_t stacked_r11;
    uint32_t stacked_r12;
    uint32_t stacked_lr;
    uint32_t stacked_pc;
    uint32_t stacked_psr;
    uint32_t old_sp;
    uint32_t msp;
    uint32_t cfsr;
    uint32_t bus_fault_address;
    uint32_t memmanage_fault_address;

    uint32_t PRIMASK;
    uint32_t BASEPRI;
    uint32_t EXC_RETURN;

    uint32_t HFSR;
    uint32_t DFSR;
    uint32_t AFSR;
} T_HARDFAULT_RECORD;

typedef struct tskTaskControlBlock
{
    uint32_t *pxTopOfStack;
    uint32_t xGenericListItem[5];
    uint32_t xEventListItem[5];
    uint32_t uxPriority;
    uint32_t *pxStack;
    char pcTaskName[ 12 ];
    uint32_t uxTCBNumber;
    uint32_t uxTaskNumber;
    uint32_t uxBasePriority;
    uint32_t uxMutexesHeld;
} TCB_t;

typedef bool (*BOOL_PATCH_FUNC)();

extern bool patch_hardfault_save_to_flash_init(void);
/**
 * extern void *app_task_handle;
 * TCB_t *pxLowerTCB =(TCB_t *)(*(uint32_t *)0x0020176c);
 * TCB_t *pxTimerTCB =(TCB_t *)(*(uint32_t *)0x00200e5c);
 * TCB_t *pxUpperTCB =(TCB_t *)(*(uint32_t *)0x00201494);
 * TCB_t *pxAppTCB =(TCB_t *)app_task_handle;
*/
/**
 * @brief  Initializes trace_hardfault save to flash including critical task stack
 * @param  dump_task_stack_num: number of task
 * @return if it is successfully initialized , return true

 * For example
   hardfault_save_to_flash_init(2, (uint32_t *)0x0020176c, 1024, &app_task_handle, 1024);
   @param  2: dump two task stack
   @param  (uint32_t *)0x0020176c: lowstack task handler address
   @param  1024: dump task stack size
   @param  &app_task_handle: app task handler address
   @param  1024: dump task stack size
 */
extern void hardfault_save_to_flash_init(uint32_t dump_task_stack_num, ...);


#endif

