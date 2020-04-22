#include "platform_utils.h"
#include "os_sched.h"
#include "os_sync.h"
#include "user_flash_driver.h"
#include "trace.h"
#include "app_section.h"
#include "flash_device.h"
#include "board.h"

#if SUPPORT_ERASE_SUSPEND
extern bool allowedDfuEnterDlps;
bool erase_in_progress = false;
bool is_suspend = false;
uint32_t signal;//os lock signal
DATA_RAM_FUNCTION void app_flash_erase_suspend()
{
    if (erase_in_progress && !is_suspend)
    {
        bool ret;
        uint8_t status1, status2;
        uint32_t ctrlr0 = SPIC->ctrlr0;
        uint32_t addr_len = SPIC->addr_length;

        signal = os_lock();

        spic_enable(DISABLE);
        spic_clr_multi_ch();
        spic_set_tx_mode();

        SPIC->addr_length = 0;
        spic_set_dr(DATA_BYTE, 0x75); //send erase/program suspend cmd

        spic_enable(ENABLE);
        spic_wait_busy();
        spic_enable(DISABLE);

        while (1)
        {
            ret = flash_cmd_rx(0x35, 1, &status1);
            ret &= flash_cmd_rx(0x05, 1, &status2);

            DFU_PRINT_INFO3("ret is %x,status1 is %x,status2 is %x", ret, status1, status2);

            if ((!(status1 & BIT_STATUS_WIP) && (status2 & BIT_STATUS_SUSPEND)) ||
                (!(status1 & BIT_STATUS_WIP) && !(status1 & BIT_STATUS_WEL)))
            {
                DFU_PRINT_INFO0("SUSPEND OK");
                platform_delay_us(1);
                is_suspend = true;
                break;
            }
        }
        //restore ctrl0 and addr_len register
        SPIC->ctrlr0 = ctrlr0;
        SPIC->addr_length = addr_len;
        os_unlock(signal);
    }
}
DATA_RAM_FUNCTION void app_flash_erase_resume()
{
    if (erase_in_progress && is_suspend)
    {
        uint32_t ctrlr0 = SPIC->ctrlr0;
        uint32_t addr_len = SPIC->addr_length;

        signal = os_lock();
        spic_enable(DISABLE);
        spic_clr_multi_ch();
        spic_set_tx_mode();

        SPIC->addr_length = 0;

        spic_set_dr(DATA_BYTE, 0x7A);
        spic_enable(ENABLE);
        spic_wait_busy();
        spic_enable(DISABLE);
        is_suspend = false;
        SPIC->ctrlr0 = ctrlr0;
        SPIC->addr_length = addr_len;
        os_unlock(signal);
        DFU_PRINT_INFO0("RESUME OK");
    }
}

DATA_RAM_FUNCTION bool app_flash_wait_busy(void)
{
    uint8_t status1 = 0, status2 = 0;
    uint32_t ctr = 0;
    bool ret;

    while (ctr++ <= 0x10000)
    {
        signal = os_lock();
        ret = flash_cmd_rx(0x05, 1, &status1);
        ret &= flash_cmd_rx(0x35, 1, &status2);

        DFU_PRINT_INFO3("ret is %x,status1 is %x,status2 is %x", ret, status1, status2);
        if (!ret)
        {
            os_unlock(signal);
            goto wait_busy_fail;
        }

        if (!(status1 & BIT_STATUS_WIP) && !(status2 & BIT_STATUS_SUSPEND))
        {
            erase_in_progress = false;
            os_unlock(signal);
            return true;
        }
        os_unlock(signal);
        DFU_PRINT_INFO1("CNT is %x", ctr);
        os_delay(1);
    }

wait_busy_fail:
    return false;
}

DATA_RAM_FUNCTION bool app_flash_cmd_tx(uint8_t cmd, uint8_t data_len, uint8_t *data_buf)
{
    bool retval = true;
    DFU_PRINT_INFO0("app_flash_cmd_tx");
    uint32_t ctrlr0 = SPIC->ctrlr0;
    uint32_t addr_len = SPIC->addr_length;

    spic_enable(DISABLE);
    spic_clr_multi_ch();
    spic_set_tx_mode();

    SPIC->addr_length = data_len;

    spic_set_dr(DATA_BYTE, cmd);

    while (data_len--)
    {
        spic_set_dr(DATA_BYTE, *data_buf++);
    }

    spic_enable(ENABLE);

    if (!spic_wait_busy())
    {
        retval = false;
    }

    spic_enable(DISABLE);

    os_unlock(signal);

    if (retval == true && !app_flash_wait_busy())
    {
        retval = false;
    }
    //restore ctrl0 and addr_len register
    SPIC->ctrlr0 = ctrlr0;
    SPIC->addr_length = addr_len;

    return retval;
}
/**
 * @brief erase a sector of the flash.
 *
 * @param  addr          flash addr in sector to be erase.
 * @return  0 if erase successfully, error line number otherwise
*/
DATA_RAM_FUNCTION uint32_t flash_erase_sector(uint32_t addr)
{

    static uint8_t address[3];
    DFU_PRINT_INFO1("==> flash_erase_sector :%x \r\n", addr);
    address[0] = (addr >> 16) & 0xff;
    address[1] = (addr >> 8) & 0xff;
    address[2] = addr & 0xff;

    allowedDfuEnterDlps = false;//dlps io driver store/restore in xip
    signal = os_lock();
    erase_in_progress = true;
    app_flash_erase_resume();
    flash_write_enable();
    app_flash_cmd_tx(CMD_SECTOR_ERASE, 3, address);
    allowedDfuEnterDlps = true;
    //os_unlock(signal);
    return 0;
}
#endif
