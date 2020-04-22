/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      ftl_app_cb.c
   * @brief     ftl app callback functions
   * @author
   * @date
   * @version
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "os_mem.h"
#include "os_sync.h"
#include "crc16btx.h"
#include "rtl876x.h"
#include "string.h"
#include "flash_device.h"
#include "ftl.h"
#include "ftl_app_cb.h"
//#include "otp_cfg.h" //need open on b-cut IC
#include "trace.h"

/*============================================================================*
 *                              macro
 *============================================================================*/

#define taskSCHEDULER_NOT_STARTED   ( ( uint32_t ) 1 )
#define WRITABLE_32BIT              0xffffffff
#define portMAX_DELAY               ( uint32_t )0xffffffffUL

#define INFO_beg_index (0)
#define INFO_size      (2)
#define PAGE_element             (FMC_PAGE_SIZE/4)

#define LOGIC_ADDR_MAP_BIT_NUM 12
#define PAGE_element_data        ( (FMC_PAGE_SIZE/8)-1)    // 511
#define MAX_logical_address_size (((PAGE_element_data*(g_PAGE_num-1))-1)<<2)

#define MAPPING_table_size   (MAX_logical_address_size / 4 * LOGIC_ADDR_MAP_BIT_NUM / 8)

#define FTL_MAX_INDEX 14

extern __asm void vAssertHandler(const char *pFuncName, uint32_t funcLine);
#define configASSERT( x ) if( ( x ) == 0 ) { vAssertHandler(__FUNCTION__, __LINE__); }

#define DEBUG_FTL

# ifdef DEBUG_FTL
#define PLATFORM_ASSERT(x) configASSERT(x)
#else
#define PLATFORM_ASSERT(x)
#endif

struct Page_T
{
    uint32_t Data[PAGE_element];
};

extern struct Page_T *g_pPage;
extern uint16_t g_free_cell_index;
extern uint8_t g_cur_pageID;
extern uint8_t g_doingGarbageCollection;
extern uint8_t g_PAGE_num;
extern uint8_t g_free_page_count;
extern void *ftl_sem;
extern uint8_t *ftl_mapping_table;

extern uint16_t read_mapping_table(uint16_t logical_addr);
extern void write_mapping_table(uint16_t logical_addr, uint8_t pageID, uint16_t cell_index);
extern void ftl_mapping_table_init(void);

extern uint8_t ftl_get_prev_page(uint8_t CurPageID, uint8_t *pPrePageID);
extern uint16_t ftl_get_page_end_position(struct Page_T *p, uint16_t *pEndPos);
extern uint8_t ftl_get_page_seq(struct Page_T *p);
extern uint32_t ftl_page_is_valid(struct Page_T *p);
extern uint32_t ftl_set_page_end_position(struct Page_T *p, uint16_t Endpos);
extern uint32_t ftl_check_logical_addr(uint16_t logical_addr);
extern uint8_t ftl_get_free_page_count(void);
extern uint8_t ftl_page_get_oldest(void);

extern uint32_t ftl_page_read(struct Page_T *p, uint32_t index);
extern uint32_t ftl_page_write(struct Page_T *p, uint32_t index, uint32_t data);
extern bool ftl_page_erase(struct Page_T *p);
extern bool ftl_page_format(struct Page_T *p, uint8_t sequence);

extern uint8_t ftl_page_garbage_collect(uint32_t page_thresh, uint32_t cell_thresh);

extern uint32_t ftl_ioctl(uint32_t cmd, uint32_t p1, uint32_t p2);

extern bool flash_write(uint32_t start_addr, uint32_t data_len, uint8_t *data);

extern uint32_t xTaskGetSchedulerState(void);

/*============================================================================*
 *                              Types
 *============================================================================*/
enum ftl_buffer_state
{
    FTL_SM_INIT = 0,
    WRITTING_BACK_NOT_START = 1,
    WRITTING_BACK_ING = 2,
    WRITTING_BACK_DONE = 3,
};
typedef struct
{
    uint32_t addr_flash;  //flash addr where ftl buffer data shoule be written
    uint32_t valid_len;   //valid data length in ftl buffer
    uint32_t *pFtl_buf;
    uint8_t  write_index; //ftl buffer write index
    uint8_t  state;
} T_FTL_BUFFER_SM;

/*============================================================================*
 *                              Variables
 *============================================================================*/
uint32_t ftl_buffer[14];

T_FTL_BUFFER_SM ftl_buffer_sm;

/*============================================================================*
 *                              Functions
 *============================================================================*/

uint16_t ftl_get_crc_key(uint32_t key)
{
    uint16_t crc_key = (uint16_t)(key >> 16 & 0xffff); //high 16 bit is crc
    return crc_key;
}

//check crc before read ftl data
uint8_t ftl_crc_key_check(uint32_t key, uint32_t data)
{
    uint16_t crc_key = 0;
    uint16_t  logical_addr = 0;
    uint8_t payload[6] = {0};
    uint16_t calced_crc16 = 0;

    logical_addr = (uint16_t)(key & 0xffff);
    crc_key = (uint16_t)(key >> 16 & 0xffff);

    payload[0] = (uint8_t)(data & 0x000000ff);
    payload[1] = (uint8_t)((data & 0x0000ff00) >> 8);
    payload[2] = (uint8_t)((data & 0x00ff0000) >> 16);
    payload[3] = (uint8_t)((data & 0xff000000) >> 24);
    payload[4] = (uint8_t)(logical_addr & 0x000000ff);
    payload[5] = (uint8_t)((logical_addr & 0x0000ff00) >> 8);

    calced_crc16 = btxfcs(BTXFCS_INIT, payload, 6);

    if (crc_key == calced_crc16)
    {
        // valid
        return FTL_CHECK_CRC_SUCCESS;
    }
    else
    {
        // invalid
        return FTL_CHECK_CRC_FAIL;
    }


}

uint32_t ftl_crc_key_init(uint16_t logical_addr, uint32_t data)
{
    uint32_t key;
    uint16_t crc16 = 0;
    uint8_t payload[6] = {0};

    payload[0] = (uint8_t)(data & 0x000000ff);
    payload[1] = (uint8_t)((data & 0x0000ff00) >> 8);
    payload[2] = (uint8_t)((data & 0x00ff0000) >> 16);
    payload[3] = (uint8_t)((data & 0xff000000) >> 24);
    payload[4] = (uint8_t)(logical_addr & 0x000000ff);
    payload[5] = (uint8_t)((logical_addr & 0x0000ff00) >> 8);

    crc16 = btxfcs(BTXFCS_INIT, payload, 6);
    key = crc16;
    key <<= 16;
    key |= logical_addr;

    return key;
}

uint8_t ftl_page_can_addr_drop_app(uint16_t logical_addr, uint8_t EndPageID)
{
    uint8_t found = 0;

    if (*(uint8_t *)0x00200357 & BIT2)  //test in FPGA
        //if (OTP->ftl_use_mapping_table == 1)
    {
        uint16_t phy_addr = read_mapping_table(logical_addr);
        if ((phy_addr != 0) && (phy_addr / PAGE_element != EndPageID))
        {
            found = 1;
        }
        return found;
    }

    uint8_t  pageID = g_cur_pageID;
    uint16_t key_index = g_free_cell_index - 1;

L_retry:

    if (EndPageID != pageID)
    {
        for (; key_index >= 3; key_index -= 2)
        {
            uint32_t key = ftl_page_read(g_pPage + pageID, key_index);
            uint16_t addr = key & 0xffff;
            if (addr == logical_addr)
            {
                found = 1;
                break;
            }
        }

        uint8_t prePageID;
        if (found == 0 && 0 == ftl_get_prev_page(pageID, &prePageID))
        {
            uint16_t EndPos;
            if (0 == ftl_get_page_end_position(g_pPage + prePageID, &EndPos))
            {
                pageID = prePageID;
                key_index = EndPos;

                goto L_retry;
            }
            else
            {
                // todo, error recovery
                //TODO;

                pageID = prePageID;
                key_index = PAGE_element - 1;

                goto L_retry;
            }
        }
    }

    return found;
}

uint16_t ftl_gc_imp_app_cb(void)
{

    uint16_t RecycleNum = 0;

    int8_t retry_count = g_PAGE_num - ftl_get_free_page_count() - 1;
    PLATFORM_ASSERT(g_PAGE_num > retry_count && retry_count >= 0);

    uint8_t Recycle_page;
    uint16_t key_index;
    uint16_t error;

L_retry:

    Recycle_page = ftl_page_get_oldest();

    error = ftl_get_page_end_position(g_pPage + Recycle_page, &key_index);
    if (error)
    {
        // error
        key_index = PAGE_element - 1;
        //TODO;
    }

    // drop or copy it
    for (; key_index >= 3; key_index -= 2)
    {
        uint32_t key = ftl_page_read(g_pPage + Recycle_page, key_index);
        uint32_t rdata = ftl_page_read(g_pPage + Recycle_page, key_index - 1);
        if (!ftl_crc_key_check(key, rdata)) //need or not,? slown down gc speed
        {
            uint16_t addr = key & 0xffff;

            if (ftl_page_can_addr_drop_app(addr, Recycle_page))
            {
                // drop / recycle
                //DPRINTF("drop [%d] addr: 0x%08x \n", candidate_key_index, candidate_addr);

                ++RecycleNum;
            }
            else
            {
                // copy it
                uint32_t rdata = ftl_page_read(g_pPage + Recycle_page, key_index - 1);

                // write to another place
                ftl_write_app_cb(addr, rdata);    //todo:check if need assert rdata is all FF?
            }
        }
        else
        {
            FLASH_PRINT_ERROR4("ftl_page_garbage_collect_Imp:check crc fail!recycle page:%x, retry_count:%x, index:%x, read value:%x",
                               Recycle_page, retry_count, key_index, key);
            ++RecycleNum;
        }
    }
    ftl_buffer_write_back();

    if (!ftl_page_erase(g_pPage + Recycle_page))
    {
        g_free_page_count = ftl_get_free_page_count();
        return RecycleNum;
    }

    FLASH_PRINT_INFO3("[ftl_cb] ftl_gc_imp_app_cb: Recycle_page:%d, RecycleNum:%d, retry_count:%d",
                      Recycle_page, RecycleNum, retry_count);

    if (RecycleNum == 0)
    {
        if (retry_count)
        {
            --retry_count;

            //DPRINTF("retry\n");
            goto L_retry;
        }
    }

    g_free_page_count = ftl_get_free_page_count();

    return RecycleNum;
}


void ftl_recover_from_power_lost_app(void)
{
    if (ftl_get_free_page_count())
    {
        return;
    }

    int16_t RecycleNum = 0;
    uint8_t Recycle_page;
    uint16_t key_index;
    uint16_t error;
    uint16_t later_to_write_item_num;

//L_retry:
    RecycleNum = 0;
    later_to_write_item_num = 0;
    Recycle_page = ftl_page_get_oldest();

    //DPRINTF("Recycle_page:%d\n", Recycle_page);

    error = ftl_get_page_end_position(g_pPage + Recycle_page, &key_index);
    if (error)
    {
        // error
        key_index = PAGE_element - 1;
    }

    // drop or copy it
    for (; key_index >= 3; key_index -= 2)
    {
        uint32_t key = ftl_page_read(g_pPage + Recycle_page, key_index);
        uint32_t rdata = ftl_page_read(g_pPage + Recycle_page, key_index - 1);
        if (!ftl_crc_key_check(key, rdata))
        {
            uint16_t addr = key & 0xffff;

            if (ftl_page_can_addr_drop_app(addr, Recycle_page))
            {
                ++RecycleNum;
            }
            else
            {
                // copy it
                uint32_t rdata = ftl_page_read(g_pPage + Recycle_page, key_index - 1);
                // write to another place
                if ((g_free_cell_index + 1) < PAGE_element)
                {
                    --RecycleNum;
                    ftl_write_app_cb(addr, rdata);
                }
                else
                {
                    ++later_to_write_item_num;
                    // need more safe?? use another eflash page ?? or ram ??
                }
            }
        }
        else
        {
            FLASH_PRINT_ERROR2("[ftl_cb] check crc fail ! func: %s, line: %d", __FUNCTION__, __LINE__);
            ++RecycleNum;
        }
    }
    ftl_buffer_write_back();

    uint32_t handle_error = 0;
    if (later_to_write_item_num)
    {
        handle_error = 1;
    }

    // make the page to invalid page
    ftl_page_write(g_pPage + Recycle_page, INFO_beg_index, 0x0);

    if (handle_error)
    {
        ftl_ioctl(FTL_IOCTL_CLEAR_ALL, 0, 0);
        FLASH_PRINT_WARN0("recovery FTL fail... clean all");
    }

}

#if 1 //(CHECK_ADDR_FIRST == 1)
/*check addr first, when addr is right,crc fail, go to find, so may return old value==>more reasonable!!
test time:
read from buffer:7-9us
read from flash:52us = 45us + 7us
test time in puran flash:
read from buffer:30us
read from buffer+flash:74.5us
read from just flash:70us */

//4:crc error, 1:not found, 0 success
uint32_t ftl_buffer_read(uint16_t logical_addr, uint32_t *value)
{
    uint32_t ret = FTL_READ_SUCCESS;
    if (ftl_buffer_sm.write_index == 0)
    {
        return FTL_READ_ERROR_READ_NOT_FOUND;
    }

    uint8_t write_index = ftl_buffer_sm.write_index;
    for (; write_index >= 2; write_index -= 2)
    {
        uint32_t key = ftl_buffer_sm.pFtl_buf[write_index - 1];
        uint32_t data = ftl_buffer_sm.pFtl_buf[write_index - 2];
        uint16_t addr = key & 0xffff;
        if (addr == logical_addr)
        {
            if (!ftl_crc_key_check(key, data))
            {
                //data valid
                *value = data;
                return FTL_READ_SUCCESS;

            }
            else
            {
                //if check addr pass but check crc fail, go to read pre data,so not return directly
                //so may not return the newest valid data
                ret = FTL_READ_ERROR_CRC_FAIL;
            }
        }

    }
    ret = FTL_READ_ERROR_READ_NOT_FOUND;

    return ret;

}

// logical_addr is 4 bytes alignment addr
uint32_t ftl_read_app_cb(uint16_t logical_addr, uint32_t *value)
{
    uint32_t ret = FTL_READ_SUCCESS;

    if (ftl_check_logical_addr(logical_addr))
    {
        FLASH_PRINT_ERROR2("[ftl_cb]check addr align error! func: %s,logical addr 0x%x", __FUNCTION__,
                           logical_addr);
        PLATFORM_ASSERT(0);
        return FTL_READ_ERROR_INVALID_LOGICAL_ADDR;
    }
    else
    {
        uint32_t r_data;
        ret = ftl_buffer_read(logical_addr, &r_data);
        if (!ret)
        {
            *value = r_data;
#if (TEST_FTL_SPEED == 0)
            FLASH_PRINT_WARN3("[ftl_cb] br 0x%08x: 0x%08x (%d)\r\n", logical_addr, *value, ret);
#endif
            return FTL_READ_SUCCESS;
        }
        else
        {
            if (ret == FTL_READ_ERROR_CRC_FAIL)
            {
                FLASH_PRINT_ERROR3("[ftl_cb]check crc key error! func: %s,logical addr 0x%x,line: %d", __FUNCTION__,
                                   logical_addr, __LINE__);
            }
        }

        /*if not found in ram, go to find flash space*/
        if (!(*(uint8_t *)0x00200357 & BIT2))//test on FPGA
            //if (OTP->ftl_use_mapping_table == 0)
        {
            uint8_t  found = 0;
            uint8_t pageID = g_cur_pageID;
            int32_t key_index = g_free_cell_index - 1;

L_retry:

            for (; key_index >= 3; key_index -= 2)
            {
                uint32_t key = ftl_page_read(g_pPage + pageID, key_index);

                uint16_t addr = key & 0xffff;
                if (addr == logical_addr)
                {
                    found = 1;
                    *value = ftl_page_read(g_pPage + pageID, key_index - 1);
                    if (!ftl_crc_key_check(key, *value))
                    {
                        //data valid
                        ret = FTL_READ_SUCCESS;

                        break;

                    }
                    else
                    {
                        FLASH_PRINT_ERROR3("[ftl_cb]check crc key error! func: %s,logical addr 0x%x,line: %d", __FUNCTION__,
                                           logical_addr, __LINE__);
                        ret = FTL_READ_ERROR_CRC_FAIL;  //not return directly , or record check crc fail num?
                    }

                }

            }

            uint8_t prePageID;
            if (found == 0 && 0 == ftl_get_prev_page(pageID, &prePageID))
            {
                uint16_t EndPos;

                if (0 == ftl_get_page_end_position(g_pPage + prePageID, &EndPos))
                {
                    key_index = EndPos;
                    pageID = prePageID;

                    goto L_retry;
                }
                else
                {
                    //TODO;
                    // todo, error recovery

                    key_index = PAGE_element - 1;
                    pageID = prePageID;

                    goto L_retry;
                }
            }

            if (!found)
            {
                ret = FTL_READ_ERROR_READ_NOT_FOUND;
                //ftl_ioctl( FTL_IOCTL_DEBUG,0,0);
            }
        }
        else
        {
            uint16_t phy_addr = read_mapping_table(logical_addr);//index of data element

            if (phy_addr == 0)//0 means invalid logical address
            {
                FLASH_PRINT_ERROR2("[ftl_cb] invalid logical address! func: %s, line: %d", __FUNCTION__, __LINE__);
                return FTL_READ_ERROR_READ_NOT_FOUND;
            }

            uint8_t pageID = phy_addr / PAGE_element;
            uint16_t key_index = phy_addr % PAGE_element;

            uint32_t key = ftl_page_read(g_pPage + pageID, key_index + 1);


            uint16_t addr = key & 0xffff;
            if (addr == logical_addr)
            {
                *value = ftl_page_read(g_pPage + pageID, key_index);
                if (!ftl_crc_key_check(key, *value))
                {
                    //data valid
                    ret = 0;
                }
                else
                {

                    FLASH_PRINT_ERROR3("[ftl_cb]check crc key error! func: %s,logical addr 0x%x,line: %d", __FUNCTION__,
                                       logical_addr, __LINE__);
                    return FTL_READ_ERROR_CRC_FAIL;  //if use mapping table return directly
                }
            }
            else
            {
                ret = FTL_READ_ERROR_PARSE_ERROR;
                FLASH_PRINT_ERROR2("[ftl_cb] logical address parse error! func: %s, line: %d", __FUNCTION__,
                                   __LINE__);
            }

        }
    }

    FLASH_PRINT_INFO3("[ftl_cb] r  0x%08x: 0x%08x (%d)\r\n", logical_addr, *value, ret);
    return  ret;
}

#endif


void ftl_buffer_sm_init(void)
{
    ftl_buffer_sm.addr_flash = (uint32_t) & (g_pPage + g_cur_pageID)->Data[g_free_cell_index];
    ftl_buffer_sm.valid_len = (PAGE_element - g_free_cell_index > FTL_MAX_INDEX) ? FTL_MAX_INDEX :
                              (PAGE_element - g_free_cell_index);
    ftl_buffer_sm.pFtl_buf = ftl_buffer;
    ftl_buffer_sm.write_index = 0;
    ftl_buffer_sm.state = FTL_SM_INIT;
}

// return !0 ftl buffer data write back success
// return 0 mean  ftl buffer data is none
uint32_t ftl_buffer_write_back(void)
{
    uint32_t ret = 0;
    if (ftl_buffer_sm.state == FTL_SM_INIT)
    {
        ret = FTL_BUFFER_DATA_WRITE_BACK_NONE;
        return ret;
    }

    //need assert flash read back data?
    flash_write(ftl_buffer_sm.addr_flash, ftl_buffer_sm.write_index  * 4,
                (uint8_t *)ftl_buffer_sm.pFtl_buf);
    g_free_cell_index += ftl_buffer_sm.write_index;
    ret = FTL_BUFFER_DATA_WRITE_BACK_SUCCESS;

    if (g_free_cell_index >= PAGE_element) //do gc
    {
        uint16_t tmp;
        if (ftl_get_page_end_position(g_pPage + g_cur_pageID, &tmp))
        {
            // invalid end pos
            // so set end pos
            ftl_set_page_end_position(g_pPage + g_cur_pageID, g_free_cell_index - 1);
        }

        // find invalid(free) page
        uint8_t new_cur_pageID = g_cur_pageID + 1;
        new_cur_pageID %= g_PAGE_num;

        if (ftl_page_is_valid(g_pPage + new_cur_pageID))
        {
            //DPRINTF("ftl_write: before format\n");
            //ftl_ioctl( FTL_IOCTL_DEBUG, 0, 0);

            // invalid page and format it
            uint8_t new_sequence = ftl_get_page_seq(g_pPage + g_cur_pageID) + 1;
            if (!ftl_page_format(g_pPage + new_cur_pageID, new_sequence))
            {
                return FTL_WRITE_ERROR_ERASE_FAIL;
            }

            // updata current page info
            g_cur_pageID = new_cur_pageID;
            g_free_cell_index = INFO_size;

            ftl_buffer_sm_init();

            if (!g_doingGarbageCollection)
            {
                //if (otp.platform_cfg.ftl_only_gc_in_idle)
                if (*(uint8_t *) 0x0020034b & BIT6)
                {
                    ret = FTL_WRITE_ERROR_NEED_GC;
                }
                else
                {
                    ftl_page_garbage_collect(0, PAGE_element / 2);
                    ret = FTL_BUFFER_DATA_WRITE_BACK_DO_GC;
                }
            }
        }
    }
    ftl_buffer_sm_init();
    return ret;
}

void ftl_buffer_push(uint32_t data)
{
    ftl_buffer_sm.state = WRITTING_BACK_NOT_START;
    ftl_buffer_sm.pFtl_buf[ftl_buffer_sm.write_index] = data;
    ftl_buffer_sm.write_index++;

    if (ftl_buffer_sm.write_index >= ftl_buffer_sm.valid_len)
    {
        ftl_buffer_write_back();
    }
}

void ftl_buffer_push_key(uint32_t key)
{
    uint16_t logical_addr = key & 0xffff;

    if (*(uint8_t *)0x00200357 & BIT2) //test on FPGA
        //if (OTP->ftl_use_mapping_table == 1)
    {
        write_mapping_table(logical_addr, g_cur_pageID, g_free_cell_index + ftl_buffer_sm.write_index - 1);

    }
    ftl_buffer_push(key);  //may triger write back, and may do gc
}



uint32_t ftl_write_app_cb(uint16_t logical_addr, uint32_t w_data)
{
    uint32_t ret = FTL_WRITE_SUCCESS;
    uint8_t sem_flag = false;

    if (0 != __get_IPSR())
    {
        FLASH_PRINT_WARN0("[ftl_cb] FTL_write should not be called in interrupt handler!\n");
        return FTL_WRITE_ERROR_IN_INTR;
    }

    if ((NULL != ftl_sem) && (taskSCHEDULER_NOT_STARTED != xTaskGetSchedulerState()))
    {
        if (os_mutex_take(ftl_sem, portMAX_DELAY) == true)
        {
            sem_flag = true;
        }
    }

    if (ftl_check_logical_addr(logical_addr))
    {
        FLASH_PRINT_ERROR2("[ftl_cb]check addr align error! func: %s,logical addr 0x%x", __FUNCTION__,
                           logical_addr);

        PLATFORM_ASSERT(0);
        ret = FTL_WRITE_ERROR_INVALID_ADDR;
    }
    else
    {
        ftl_buffer_push(w_data);
        //change to more secure integrity checkout, not hanming(hard to decode), crc
        uint32_t key = ftl_crc_key_init(logical_addr, w_data);

        ftl_buffer_push_key(key);

    }

    if (sem_flag)
    {
        os_mutex_give(ftl_sem);
    }
#if (TEST_FTL_SPEED == 0)
    FLASH_PRINT_WARN3("[ftl_cb] w 0x%08x: 0x%08x (%d)\r\n", logical_addr, w_data, ret);
#endif
    return ret;
}

void ftl_mapping_table_init_app(void)
{
    if (NULL == ftl_mapping_table)
    {
        uint32_t mapping_table_size = MAPPING_table_size;
        ftl_mapping_table = os_mem_zalloc(RAM_TYPE_BUFFER_ON,
                                          mapping_table_size);//table is initialised as 0
    }

    uint8_t pageID = g_cur_pageID;
    int32_t key_index = g_free_cell_index - 1;

L_retry:

    for (; key_index >= 3; key_index -= 2)
    {
        uint32_t key = ftl_page_read(g_pPage + pageID, key_index);
        uint16_t addr = key & 0xffff;
        if (!read_mapping_table(addr))
        {
            write_mapping_table(addr, pageID, key_index - 1);
        }
    }

    uint8_t prePageID;
    if (0 == ftl_get_prev_page(pageID, &prePageID))
    {
        uint16_t EndPos;

        if (0 == ftl_get_page_end_position(g_pPage + prePageID, &EndPos))
        {
            key_index = EndPos;
            pageID = prePageID;

            goto L_retry;
        }
        else
        {
            //TODO;
            // todo, error recovery

            key_index = PAGE_element - 1;
            pageID = prePageID;

            goto L_retry;
        }
    }

}


uint32_t ftl_init_app_cb(uint32_t u32PageStartAddr, uint8_t pagenum)
{
    if (ftl_sem == NULL)
    {
        os_mutex_create(&ftl_sem);
    }

    g_pPage = (struct Page_T *)(u32PageStartAddr);


    // find latest valid page by sequence num
    uint8_t cur_valid = 0;
    uint8_t cur_sequence = 0;
    uint8_t cur_pageID = 0;

    uint8_t need_to_update = 0;

    uint32_t i;
    for (i = 0; i < g_PAGE_num; ++i)
    {
        if (0 == ftl_page_is_valid(g_pPage + i))
        {
            if (cur_valid)
            {
                uint8_t tmp_sequence = ftl_get_page_seq(g_pPage + i);
                if (tmp_sequence > cur_sequence) // newer
                {
                    if (tmp_sequence == (cur_sequence + 1))
                    {
                        need_to_update = 1;
                    }
                    else
                    {
                        //DPRINTF("tmp_sequence,cur_sequence: %d,%d\n", tmp_sequence, cur_sequence );
                    }

                }
                else if (tmp_sequence == 0 && cur_sequence == 0xff) // wrap
                {
                    need_to_update = 1;
                }
                else
                {
                    // old
                }
            }
            else
            {
                need_to_update = 1;
            }
        }

        if (need_to_update)
        {
            need_to_update = 0;

            cur_valid = 1;
            cur_pageID = i;
            cur_sequence = ftl_get_page_seq(g_pPage + i);
        }
    }

    if (!cur_valid)
    {
        // not any valid, first time to init
        cur_pageID = 0;
        cur_sequence = 0;
        ftl_page_format(g_pPage + cur_pageID, cur_sequence);
    }

    // find free cell from bottom
    uint16_t free_cell_index = INFO_size;
    for (i = PAGE_element - 1 ; i >= INFO_size ; --i)
    {
        if (WRITABLE_32BIT != ftl_page_read(g_pPage + cur_pageID, i))
        {
            free_cell_index = i + 1;
            break;
        }
    }

    g_cur_pageID = cur_pageID;
    g_free_cell_index = free_cell_index;

    ftl_ioctl(FTL_IOCTL_DEBUG, 0, 0);

    ftl_buffer_sm_init();

    //if (OTP->ftl_use_mapping_table == 1)
    if (*(uint8_t *)0x00200357 & BIT2)  //test on FPGA
    {
        ftl_mapping_table_init_app();
    }
    ftl_recover_from_power_lost_app();

    g_free_page_count = ftl_get_free_page_count();

    return 0;
}

#if 0
/*check crc first, read data need more time ,may not suggest!!
worst case read 4 bytes 78us = 45us + 30us
test time in puran flash:
read from buffer:27-50us
read from buffer+flash:98us
read from just flash:70us */

//1:not found, 0 success, check crc first
uint32_t ftl_buffer_read(uint16_t logical_addr, uint32_t *value)
{
    if (ftl_buffer_sm.write_index == 0)
    {
        return FTL_READ_ERROR_READ_NOT_FOUND;
    }

    uint8_t write_index = ftl_buffer_sm.write_index;
    for (; write_index >= 2; write_index -= 2)
    {
        uint32_t key = ftl_buffer_sm.pFtl_buf[write_index - 1];
        uint32_t data = ftl_buffer_sm.pFtl_buf[write_index - 2];
        if (!ftl_crc_key_check(key, data))
        {
            uint16_t addr = (uint16_t)(key & 0x0000ffff);
            if (addr == logical_addr)
            {
                //data valid
                *value = data;
                return FTL_READ_SUCCESS;
            }
            //else find pre pos, so may return old value not the newest data
        }
        else
        {
            //continue find pre pos, so may return old value not the newest data
            FLASH_PRINT_ERROR3("[ftl] check crc key error! func: %s, key:0x%08x,line: %d", __FUNCTION__, key,
                               __LINE__);
        }


    }

    return FTL_READ_ERROR_READ_NOT_FOUND;
}

// logical_addr is 4 bytes alignment addr
uint32_t ftl_read_app_cb(uint16_t logical_addr, uint32_t *value)
{
    uint32_t ret = FTL_READ_SUCCESS;
    uint32_t r_data;

    if (ftl_check_logical_addr(logical_addr))
    {
        FLASH_PRINT_ERROR2("[ftl_cb]check addr align error! func: %s,logical addr 0x%x", __FUNCTION__,
                           logical_addr);
        PLATFORM_ASSERT(0);
        return FTL_READ_ERROR_INVALID_LOGICAL_ADDR;
    }
    else
    {
        uint32_t r_buffer_data;
        if (!ftl_buffer_read(logical_addr, &r_buffer_data))
        {
            *value = r_buffer_data;
            FLASH_PRINT_INFO3("[ftl_cb] br 0x%08x: 0x%08x (%d)\r\n", logical_addr, *value, ret);
            return FTL_READ_SUCCESS;
        }

        if (!(*(uint8_t *)0x00200357 & BIT2)) //test on FPGA
            //if (OTP->ftl_use_mapping_table == 0)
        {
            uint8_t  found = 0;
            uint8_t pageID = g_cur_pageID;
            int32_t key_index = g_free_cell_index - 1;

L_retry:

            for (; key_index >= 3; key_index -= 2)
            {
                uint32_t key = ftl_page_read(g_pPage + pageID, key_index);
                r_data = ftl_page_read(g_pPage + pageID, key_index - 1);

                if (!ftl_crc_key_check(key, r_data))
                {
                    uint16_t addr = key & 0xffff;
                    if (addr == logical_addr)
                    {
                        found = 1;

                        //data valid
                        *value = r_data;
                        ret = FTL_READ_SUCCESS;
                        break;
                    }

                }
                else
                {
                    //continue find pre pos, so may return old value not the newest data
                    FLASH_PRINT_ERROR3("[ftl_cb] check crc key error! func: %s, key:0x%08x,line: %d", __FUNCTION__, key,
                                       __LINE__);
                }

            }

            uint8_t prePageID;
            if (found == 0 && 0 == ftl_get_prev_page(pageID, &prePageID))
            {
                uint16_t EndPos;

                if (0 == ftl_get_page_end_position(g_pPage + prePageID, &EndPos))
                {
                    key_index = EndPos;
                    pageID = prePageID;

                    goto L_retry;
                }
                else
                {
                    //TODO;
                    // todo, error recovery

                    key_index = PAGE_element - 1;
                    pageID = prePageID;

                    goto L_retry;
                }
            }

            if (!found)
            {
                ret = FTL_READ_ERROR_READ_NOT_FOUND;
                //ftl_ioctl( FTL_IOCTL_DEBUG,0,0);
            }
        }
        else
        {
            uint16_t phy_addr = read_mapping_table(logical_addr);//index of data element

            if (phy_addr == 0)//0 means invalid logical address
            {
                FLASH_PRINT_ERROR2("[ftl_cb] invalid logical address! func: %s, line: %d", __FUNCTION__, __LINE__);
                return FTL_READ_ERROR_READ_NOT_FOUND;
            }

            uint8_t pageID = phy_addr / PAGE_element;
            uint16_t key_index = phy_addr % PAGE_element;

            uint32_t key = ftl_page_read(g_pPage + pageID, key_index + 1);
            r_data = ftl_page_read(g_pPage + pageID, key_index);
            if (!ftl_crc_key_check(key, r_data))
            {
                uint16_t addr = key & 0xffff;
                if (addr == logical_addr)
                {
                    //data valid
                    *value = r_data;
                    ret = FTL_READ_SUCCESS;
                }
                else
                {
                    ret = FTL_READ_ERROR_PARSE_ERROR;
                    FLASH_PRINT_ERROR2("[ftl_cb] logical address parse error! func: %s, line: %d", __FUNCTION__,
                                       __LINE__);
                }

            }
            else
            {
                ret = FTL_READ_ERROR_CRC_FAIL;
                FLASH_PRINT_ERROR3("[ftl_cb]check crc key error! func: %s,key: 0x%08x,line: %d", __FUNCTION__, key,
                                   __LINE__);
            }

        }
    }
    FLASH_PRINT_INFO3("[ftl_cb] r  0x%08x: 0x%08x (%d)\r\n", logical_addr, *value, ret);
    return  ret;
}
#endif

