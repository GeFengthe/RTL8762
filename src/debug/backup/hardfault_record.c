#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include "otp.h"
#include "trace.h"
#include "flash_device.h"
#include "rtl876x.h"
#include "app_section.h"
#include "hardfault_record.h"

/******** bit definitions of hardfault configuration ********/
#define BIT_ENABLE_SAVE_HARDFAULT       0x00000001    /* BIT0 */
#define BIT_ENABLE_DUMP_HARDFAULT       0x00000004    /* BIT2 */
#define BIT_CLEAR_HISTROY_AFTER_DUMP    0x00000008   /* BIT3 */
#define BIT_CLEAR_HISTROY_BEFORE_SAVING 0x00000010  /* BIT4 */

/******** magic pattern for hardfault record ********/
#define HARDFAULT_PATTERN_BEG           (0xbeefface)
#define HARDFAULT_PATTERN_END           (0xbabeface)
#define HARDFAULT_PATTERN_MID           (0xdcabface)

#define TRACE_CALL_STACK_SIZE           1024
#define TRACE_TASK_OVERFLOW_SIZE        16
#define MAX_DUMP_TASK_STACK_NUM         8


extern BOOL_PATCH_FUNC patch_HardFaultRecord_TryToSave;

void **p_task_handle[MAX_DUMP_TASK_STACK_NUM];
uint32_t dump_task_stack_size[MAX_DUMP_TASK_STACK_NUM];
uint32_t task_handle_num = 0;

/************Dump info:  *********
*********Part0: beg pattern*******************
4:                  PATTERN_BEG
*********Part1: hardfault register record****
sizeof(*record):    hardfault reg info
4:                  PATTERN_MID
*********Part2: Current task stack****
1024:               current task stack
4:                  PATTERN_MID
16:                 bottom of current task stack
4:                  PATTERN_MID
*********Part3: specific task stack****
task stack size0:   task stack0
4:                  PATTERN_MID
16:                 bottom of task stack0
4:                  PATTERN_MID
task stack size1:   task stack1
4:                  PATTERN_MID
16:                 bottom of task stack1
4:                  PATTERN_MID
.......
task stack size n:   task stack n
4:                  PATTERN_MID
16:                 bottom of task stack n
4:                  PATTERN_MID
**********Part4: end pattern*******************
4:                  PATTERN_END
*********************************/
bool patch_HardFaultRecord_TryToSave_imp(T_HARDFAULT_RECORD *record)
{
    uint32_t cfg = 0;
    uint32_t beg_addr = 0;
    uint32_t end_addr = 0;
    uint32_t write_addr = 0;
    uint32_t total_dump_tack_size = 0;
    uint32_t total_dump_size = 0;
    uint32_t *data = NULL;
    uint32_t len = 0;
    uint32_t old_sp = 0;
    TCB_t *p_dump_tcb = NULL;

    if (!flash_get_flash_exist() || !OTP->write_info_to_flash_when_hardfault)
    {
        return true;
    }

    cfg = OTP->HardFault_Record_CFG;
    beg_addr = flash_get_bank_addr(FLASH_HARDFAULT_RECORD);
    end_addr = beg_addr + flash_get_bank_size(FLASH_HARDFAULT_RECORD);

    if (!(cfg & BIT_ENABLE_SAVE_HARDFAULT))
    {
        return true;
    }

    DBG_DIRECT("%s(%d,0x%x,0x%x)", __FUNCTION__, cfg, beg_addr, end_addr);

    /* clear history hardfault in flash if set*/
    if (cfg & BIT_CLEAR_HISTROY_BEFORE_SAVING)
    {
        uint32_t erase_addr = beg_addr;
        while (erase_addr + FMC_PAGE_SIZE <= end_addr)
        {
            flash_erase_locked(FLASH_ERASE_SECTOR, erase_addr);
            erase_addr += FMC_PAGE_SIZE;
        }
    }

    /******** save hardfault info to flash**********/
    for (uint32_t i = 0; i < task_handle_num; i++)
    {
        total_dump_tack_size += 4 + dump_task_stack_size[i] + 16 + 4;
    }
    total_dump_size = 4 + sizeof(*record) + 4 + (1024 + 4 + 16 + 4) +
                      total_dump_tack_size + 4;
    /* find first free addr*/
    for (write_addr = beg_addr; write_addr < end_addr; write_addr += 4)
    {
        uint32_t value = *(uint32_t *)write_addr;
        if (value == FMC_ERASE_PATTERN)
        {
            break;
        }
        if (value == HARDFAULT_PATTERN_BEG)
        {
            write_addr += total_dump_size - 4;
        }
    }

    /*if left buffer is not enough for the record */
    if (write_addr  + total_dump_size >= end_addr)
    {
        flash_erase_locked(FLASH_ERASE_SECTOR, beg_addr);
        write_addr = beg_addr;
    }
    DBG_DIRECT("First write_addr=0x%x, total_dump_size=0x%x", write_addr, total_dump_size);

    /*Part0: beg pattern*/
    flash_auto_write_locked(write_addr, HARDFAULT_PATTERN_BEG);
    write_addr += 4;

    /*Part1: hardfault register record*/
    data = (uint32_t *)record;
    len = sizeof(*record);

    for (uint32_t i = 0; i < len; i += 4)
    {
        flash_auto_write_locked(write_addr, *data);
        data++;
        write_addr += 4;
    }
    flash_auto_write_locked(write_addr, HARDFAULT_PATTERN_MID);
    write_addr += 4;

    /*Part2: Current task stack */
    p_dump_tcb = (TCB_t *)(*(uint32_t *)0x00200d4c);
    DBG_DIRECT("Current TCB=0x%x", p_dump_tcb);
    old_sp = *((uint32_t *)record + 16);
    len = TRACE_CALL_STACK_SIZE;
    data = (uint32_t *)old_sp;
    for (uint32_t i = 0; i < len; i += 4)
    {
        flash_auto_write_locked(write_addr, *data);
        data++;
        write_addr += 4;
    }
    /*middle pattern*/
    flash_auto_write_locked(write_addr, HARDFAULT_PATTERN_BEG);
    write_addr += 4;
    /*check stack overflow*/
    len = TRACE_TASK_OVERFLOW_SIZE;
    data = p_dump_tcb->pxStack;
    for (uint32_t i = 0; i < len; i += 4)
    {
        flash_auto_write_locked(write_addr, *data);
        data++;
        write_addr += 4;
    }
    /*middle pattern*/
    flash_auto_write_locked(write_addr, HARDFAULT_PATTERN_MID);
    write_addr += 4;

    /*Part3: write the stack of specific task determined by init function parameter */
    for (uint32_t i = 0; i < task_handle_num; i++)
    {
        p_dump_tcb = (TCB_t *)(*p_task_handle[i]);
        DBG_DIRECT("task handle=0x%x, p_task_handle[i]=0x%x", p_dump_tcb, p_task_handle[i]);
        /*1. wrtie specific task stack size*/
        len = dump_task_stack_size[i];
        data = p_dump_tcb->pxTopOfStack;
        for (uint32_t i = 0; i < len; i += 4)
        {
            flash_auto_write_locked(write_addr, *data);
            data++;
            write_addr += 4;
        }
        /*middle pattern*/
        flash_auto_write_locked(write_addr, HARDFAULT_PATTERN_MID);
        write_addr += 4;

        /*2. wrtie task stack  bottom to check overflow*/
        len = TRACE_TASK_OVERFLOW_SIZE;
        data = p_dump_tcb->pxStack;
        for (uint32_t i = 0; i < len; i += 4)
        {
            flash_auto_write_locked(write_addr, *data);
            data++;
            write_addr += 4;
        }
        /*middle pattern*/
        flash_auto_write_locked(write_addr, HARDFAULT_PATTERN_MID);
        write_addr += 4;
    }

    /*Part4: End pattern */
    flash_auto_write_locked(write_addr, HARDFAULT_PATTERN_END);

    return true;
}

APP_FLASH_TEXT_SECTION bool patch_hardfault_save_to_flash_init(void)
{
    if (patch_HardFaultRecord_TryToSave == NULL)
    {
        patch_HardFaultRecord_TryToSave = patch_HardFaultRecord_TryToSave_imp;
        return true;
    }
    else
    {
        return false;
    }
}

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
void hardfault_save_to_flash_init(uint32_t dump_task_stack_num, ...)
{
    if (dump_task_stack_num > MAX_DUMP_TASK_STACK_NUM)
    {
        dump_task_stack_num = MAX_DUMP_TASK_STACK_NUM;
    }

    task_handle_num = dump_task_stack_num;

    va_list arg;
    va_start(arg, dump_task_stack_num);
    for (uint32_t i = 0; i < dump_task_stack_num; i++)
    {
        p_task_handle[i] = va_arg(arg, void **);
        dump_task_stack_size[i] = va_arg(arg, uint32_t);
    }
    va_end(arg);
}
