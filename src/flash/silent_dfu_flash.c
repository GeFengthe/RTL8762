#include <string.h>
#include "trace.h"
#include "app_section.h"
#include "flash_device.h"
#include "flash_adv_cfg.h"
#include "patch_header_check.h"
#include "hw_aes.h"
#include "rtl876x_hw_aes.h"
#include "os_sync.h"
#include "otp.h"
#include "dfu_api.h"
#include "silent_dfu_flash.h"
#include "board.h"

#if (SUPPORT_SILENT_OTA_128M_USER_DATA == 1)
#include "sha256.h"
#include "crc16btx.h"

#define CRC_BUFFER_SIZE                 128
#define SHA256_BUFFER_SIZE              128
#define SHA256_LENGTH                   32

#define DFU_UPDATE_LEN                  60
#endif

uint8_t prev_bp_lv = 0xff;
uint32_t appdefine_offset = 0;

/**
*  @brief: unlock flash is need when erase or write flash.
*/
bool unlock_flash_all(void)
{
    DFU_PRINT_TRACE0("**********[Flash Set] Flash unlock ***********");
    if (FLASH_SUCCESS == flash_sw_protect_unlock_by_addr_locked(0x00800000, &prev_bp_lv))
    {
        DFU_PRINT_TRACE1("[Flash Set] Flash unlock address = 0x800000, prev_bp_lv = %d", prev_bp_lv);
        return true;
    }
    return false;
}

/**
*  @brief: lock flash after erase or write flash.
*/
void lock_flash(void)
{
    if (prev_bp_lv != 0xff)
    {
        flash_set_block_protect_locked(prev_bp_lv);
    }
}

/**
 * @brief erase a sector of the flash.
 *
 * @param  addr          flash addr in sector to be erase.
 * @return  0 if erase successfully, error line number otherwise
*/
uint32_t flash_erase_sector(uint32_t addr)
{
    uint32_t result = 0xff;
    result = flash_erase_locked(FLASH_ERASE_SECTOR, addr);
    DFU_PRINT_INFO2("flash_erase_sector: addr=%x, result=%d", addr, result);

    return result;
}

/**
 * @brief erase a sector of the flash.
 *
 * @param  signature          signature to identify FW.
 * @param  offset             offset of the image.
 * @return  0 if erase successfully, error line number otherwise
*/
uint32_t sil_dfu_flash_erase(uint16_t signature, uint32_t offset)
{
    uint32_t result = 0;
    uint32_t dfu_base_addr;
    dfu_base_addr = get_temp_ota_bank_addr_by_img_id((T_IMG_ID)signature);
    if (dfu_base_addr == 0)
    {
        result = __LINE__;
        goto L_Return;
    }

    flash_erase_sector(dfu_base_addr + offset);
L_Return:
    DFU_PRINT_INFO1("<====sil_dfu_flash_erase: result=%d", result);
    return result;
}

uint32_t sil_dfu_flash_check_blank(uint16_t image_id, uint32_t offset, uint16_t size)
{
    uint32_t result = 0;
    DFU_PRINT_INFO3("sil_dfu_flash_check_blank:image_id=0x%x, offset=0x%x, size=%d",
                    image_id, offset, size);
    if (image_id == IMAGE_USER_DATA)
    {
        uint32_t i = 0;
        uint32_t start_addr = flash_get_bank_addr(FLASH_BKP_DATA1) + offset;

        for (i = 0; i < size / 4; i++)
        {
            uint32_t r_data;
            flash_read_locked(start_addr, 4, (uint8_t *)&r_data);
            if (r_data != 0xffffffff)
            {
                DFU_PRINT_ERROR1("sil_dfu_flash_check_blank:error! i=%d", i);
                result = __LINE__;
                goto L_Return;
            }
            start_addr += 4;
        }
    }
    else
    {
        uint32_t signal = os_lock(); //need? grace
        /*get start addr from temp addr and use auto mode read to check erase result*/
        result = dfu_flash_check_blank(image_id, offset, size);
        os_unlock(signal);
    }

L_Return:
    DFU_PRINT_INFO1("sil_dfu_flash_check_blank: result=%d", result);
    return result;
}


/**
 * @brief erase a sector of the flash, will retry three times at most
 *
 * @param  signature          signature to identify FW.
 * @param  offset             offset of the image.
 * @return  0 if erase successfully, error line number otherwise,
*/
uint32_t sil_dfu_flash_erase_with_retry(uint16_t image_id, uint32_t offset, uint16_t size)
{
    uint32_t result = 0;
    uint32_t cnt = 0;

    uint32_t dfu_base_addr;
    if (image_id == IMAGE_USER_DATA)
    {
        dfu_base_addr = flash_get_bank_addr(FLASH_BKP_DATA1);;
    }
    else
    {
        dfu_base_addr = get_temp_ota_bank_addr_by_img_id((T_IMG_ID)image_id);
    }

    if (dfu_base_addr == 0)
    {
        result = __LINE__;
        goto L_Return;
    }

    do
    {
        flash_erase_sector(dfu_base_addr + offset);
        result = sil_dfu_flash_check_blank(image_id, offset, size);

        if (!result)
        {
            //erase ok
            break;
        }
        else
        {
            //retry
            cnt++;
        }
        if (cnt >= 3)
        {
            /*retry three times all fail*/
            goto L_Return;
        }
    }
    while (1);

L_Return:
    DFU_PRINT_TRACE2("sil_check_erase_flash: cnt=%d, result=%d",
                     cnt, result);
    return result;
}

#if APP_DATA_COPY_ENABLE
extern uint8_t *p_ota_temp_buffer_head;
/**
 * @brief   copy app data from active bank to temp bank when support bank switch.
 *
 * @param   image_id    signature to identify image.
 * @param   dlAddress   address the img copy to.
 * @param   dlSize      copy size.
 * @return  true if the image copied success, false otherwise
*/
bool sil_dfu_copy_img(uint16_t image_id, uint32_t dlAddress, uint32_t dlSize)
{
    uint32_t error_code = 0;
    uint32_t source_base_addr;
    uint32_t dest_base_addr;
    int remain_size = dlSize;
    uint32_t s_val;
    uint32_t dlOffset, tmp_offset;

    if ((image_id < AppData1) || (image_id > AppData6))
    {
        error_code = __LINE__;
        goto L_Return;
    }
    if (dlAddress % 4096)
    {
        error_code = __LINE__;
        goto L_Return;
    }

    if (!is_ota_support_bank_switch())
    {
        error_code = __LINE__;
        goto L_Return;
    }

    source_base_addr = get_active_ota_bank_addr() & 0xffffff;

    if (flash_get_bank_addr(FLASH_OTA_BANK_0) == get_active_ota_bank_addr())
    {
        dest_base_addr = flash_get_bank_addr(FLASH_OTA_BANK_1) & 0xffffff;
    }
    else
    {
        dest_base_addr = flash_get_bank_addr(FLASH_OTA_BANK_0) & 0xffffff;
    }
    if ((source_base_addr % 4096) || (dest_base_addr % 4096))
    {
        error_code = __LINE__;
        goto L_Return;
    }
    if (dest_base_addr >= dlAddress)
    {
        error_code = __LINE__;
        goto L_Return;
    }
    dlOffset = dlAddress - dest_base_addr;
    tmp_offset = dlOffset;
    if (dlOffset % 4096)
    {
        error_code = __LINE__;
        goto L_Return;
    }
    T_IMG_HEADER_FORMAT *p_data_header;
    p_data_header = (T_IMG_HEADER_FORMAT *)(source_base_addr + dlOffset);
    if (p_data_header->ctrl_header.image_id != image_id)
    {
        error_code = __LINE__;
        goto L_Return;
    }

    while (remain_size > 0)
    {
        if (!((dest_base_addr + tmp_offset) % 4096)) //must 4k align
        {
            flash_erase_sector(dest_base_addr + tmp_offset);
        }
        if (remain_size > 2048)
        {
            memcpy(p_ota_temp_buffer_head, (uint8_t *)(source_base_addr + tmp_offset), 2048);
            if (remain_size ==  dlSize)
            {
                T_IMG_CTRL_HEADER_FORMAT *p_header = (T_IMG_CTRL_HEADER_FORMAT *) p_ota_temp_buffer_head;
                p_header->ctrl_flag.flag_value.not_ready = 0x1; /*make sure image is not ready, will use it later*/
            }
            for (int i = 0; i < 2048; i = i + 4)
            {
                flash_auto_write_locked(dest_base_addr + tmp_offset + i, *(uint32_t *)p_ota_temp_buffer_head);

                flash_auto_read_locked((dest_base_addr + tmp_offset + i) | FLASH_OFFSET_TO_NO_CACHE, &s_val);
                if (s_val != *(uint32_t *)p_ota_temp_buffer_head)
                {
                    DFU_PRINT_TRACE3("s_val:0x%08x, *p_void:0x%08x, i:0x%08x",
                                     s_val, *(uint32_t *)p_ota_temp_buffer_head, i);
                    error_code = __LINE__;
                    goto L_Return;
                }
                else
                {
                    p_ota_temp_buffer_head += 4;
                }
            }


            remain_size -= 2048;
        }
        else
        {
            memcpy(p_ota_temp_buffer_head, (uint8_t *)(source_base_addr + tmp_offset), remain_size);
            for (int i = 0; i < remain_size; i = i + 4)
            {
                flash_auto_write_locked(dest_base_addr + tmp_offset + i, *(uint32_t *)p_ota_temp_buffer_head);

                flash_auto_read_locked((dest_base_addr + tmp_offset + i) | FLASH_OFFSET_TO_NO_CACHE, &s_val);
                if (s_val != *(uint32_t *)p_ota_temp_buffer_head)
                {
                    DFU_PRINT_TRACE3("s_val:0x%08x, *p_void:0x%08x, i:0x%08x",
                                     s_val, *(uint32_t *)p_ota_temp_buffer_head, i);
                    error_code = __LINE__;
                    goto L_Return;
                }
                else
                {
                    p_ota_temp_buffer_head += 4;
                }
            }
            remain_size = 0;
        }
        tmp_offset += 2048;
    }

L_Return:
    DFU_PRINT_INFO1("<====dfu_copy_img  error_code:%d", error_code);
    if (error_code)
    {
        return false;
    }
    return true;
}
#endif //#if APP_DATA_COPY_ENABLE


/**
*  @brief: get temp image size accord to the image id.
*/
uint32_t get_temp_ota_bank_size_by_img_id(T_IMG_ID image_id)
{
    uint32_t image_size = 0;

    bool enable_old_ota = !is_ota_support_bank_switch();
    if (enable_old_ota)
    {
#ifdef SDK_8772
        if (image_id == SecureBoot || image_id == RomPatch || image_id == AppPatch
            || image_id == AppData1 || image_id == AppData2 || image_id == AppData3
            || image_id == AppData4 || image_id == AppData5 || image_id == AppData6)
#else

#if (SUPPORT_OTA_APP_DATA_EXTENSION == 1)
        if (image_id == SecureBoot || image_id == RomPatch || image_id == AppPatch
            || image_id == AppData1 || image_id == AppData2 || image_id == AppData3
            || image_id == AppData4 || image_id == AppData5 || image_id == AppData6)
#else
        if (image_id == SecureBoot || image_id == RomPatch || image_id == AppPatch
            || image_id == AppData1 || image_id == AppData2)
#endif

#endif
        {
            image_size = flash_get_bank_size(FLASH_OTA_TMP);
        }
        //others will return 0
    }
    else
    {
        uint32_t ota_bank0_addr = flash_get_bank_addr(FLASH_OTA_BANK_0);
        uint32_t temp_bank_addr;
        if (ota_bank0_addr == get_active_ota_bank_addr())
        {
            temp_bank_addr = flash_get_bank_addr(FLASH_OTA_BANK_1);
        }
        else
        {
            temp_bank_addr = ota_bank0_addr;
        }

        if (image_id == OTA)
        {
            image_size = OTA_HEADER_SIZE;
        }
#ifdef SDK_8772
        else if (image_id == SecureBoot || image_id == RomPatch || image_id == AppPatch
                 || image_id == AppData1 || image_id == AppData2 || image_id == AppData3
                 || image_id == AppData4 || image_id == AppData5 || image_id == AppData6)
#else

#if (SUPPORT_OTA_APP_DATA_EXTENSION == 1)
        else if (image_id == SecureBoot || image_id == RomPatch || image_id == AppPatch
                 || image_id == AppData1 || image_id == AppData2 || image_id == AppData3
                 || image_id == AppData4 || image_id == AppData5 || image_id == AppData6)
#else
        else if (image_id == SecureBoot || image_id == RomPatch || image_id == AppPatch
                 || image_id == AppData1 || image_id == AppData2)
#endif

#endif
        {
            // auth ota temp bank and get address
            // image_authencation will fail after secure boot, so remove it
            if (!check_header_valid(temp_bank_addr, OTA))
            {
                image_size = 0;
            }
            else
            {
                image_size = HAL_READ32((uint32_t) & ((T_OTA_HEADER_FORMAT *)temp_bank_addr)->secure_boot_size,
                                        (image_id - SecureBoot) * 8);

                //attention: if use old ota header generate tool, app data3-6 addr will be default value 0xffffffff
                if (OTA_HEADER_DEFAULT_VALUE == image_size)
                {
                    image_size = 0;
                }
            }
        }
        else //others will return 0
        {
        }
    }

    return image_size;
}

/**
* @brief check ota image size whether exceed flash layout address.
*/
bool check_dfu_update_image_length(uint16_t signature, uint32_t offset, uint32_t length,
                                   void *p_void, uint32_t *ret)
{
    uint32_t temp_bank_size = 0;
    *ret = 0;

    if (p_void == NULL)
    {
        *ret = __LINE__;
        return false;
    }

    if (IMAGE_USER_DATA == signature)
    {
        temp_bank_size = flash_get_bank_size(FLASH_BKP_DATA1);
    }
    else
    {
        temp_bank_size = get_temp_ota_bank_size_by_img_id((T_IMG_ID)signature);
    }


    if (offset == 0)
    {
        T_IMG_CTRL_HEADER_FORMAT *p_header = (T_IMG_CTRL_HEADER_FORMAT *) p_void;
        uint32_t total_length = p_header->payload_len + IMG_HEADER_SIZE;

        if (total_length > temp_bank_size)
        {
            DFU_PRINT_ERROR2("New Image too large! total_length = %d, temp_bank_size = %d", total_length,
                             temp_bank_size);
            *ret = __LINE__;
            return false;
        }
    }

    if (offset + length > temp_bank_size)
    {
        DFU_PRINT_ERROR3("New Image single packet too large! offset = %d, length = %d, temp_bank_size = %d",
                         offset, length, temp_bank_size);
        *ret = __LINE__;
        return false;
    }

    //check pass
    return true;
}


#if (SUPPORT_SILENT_OTA_128M_USER_DATA == 1)
uint32_t sil_dfu_write(uint32_t start_addr, uint32_t length, uint8_t *p_void)
{
    uint32_t result = 0;
    uint16_t loop_cnt = 0;
    uint16_t remain_size = 0;
    uint8_t s_val[DFU_UPDATE_LEN];

    loop_cnt = length / DFU_UPDATE_LEN;
    remain_size = length % DFU_UPDATE_LEN;
    DFU_PRINT_INFO2("sil_dfu_write:loop_cnt=0x%x, remain_size=0x%x", loop_cnt, remain_size);

    for (int i = 0; i < loop_cnt; i++)
    {
        if (flash_write_locked(start_addr + i * DFU_UPDATE_LEN, DFU_UPDATE_LEN,
                               p_void + i * DFU_UPDATE_LEN))
        {
            if (flash_read_locked(start_addr + i * DFU_UPDATE_LEN | FLASH_OFFSET_TO_NO_CACHE, DFU_UPDATE_LEN,
                                  s_val))
            {
                for (int j = 0; j < DFU_UPDATE_LEN; j++)
                {
                    if (s_val[j] != *(p_void + i * DFU_UPDATE_LEN + j))
                    {
                        DFU_PRINT_TRACE3("s_val:0x%x, *p_void:0x%x, i:%d",
                                         s_val[j], *(p_void + i * DFU_UPDATE_LEN + j), i * DFU_UPDATE_LEN + j);
                        result = __LINE__;
                    }
                }

            }
            else
            {
                result = __LINE__;
            }
        }
        else
        {
            result = __LINE__;
        }
    }

    if (remain_size)
    {
        if (flash_write_locked(start_addr + loop_cnt * DFU_UPDATE_LEN, remain_size,
                               p_void + loop_cnt * DFU_UPDATE_LEN))
        {
            if (flash_read_locked(start_addr + loop_cnt * DFU_UPDATE_LEN | FLASH_OFFSET_TO_NO_CACHE,
                                  remain_size, s_val))
            {
                for (int i = 0; i < remain_size; i++)
                {
                    if (s_val[i] != *(p_void + loop_cnt * DFU_UPDATE_LEN + i))
                    {
                        DFU_PRINT_TRACE3("s_val:0x%x, *p_void:0x%x, i:%d",
                                         s_val[i], *(p_void + loop_cnt * DFU_UPDATE_LEN + i), loop_cnt * DFU_UPDATE_LEN + i);
                        result = __LINE__;
                    }
                }
            }
            else
            {
                result = __LINE__;
            }
        }
        else
        {
            result = __LINE__;
        }
    }


    return result;
}
#endif

/**
 * @brief  write specified image data with specified length to flash
 * @param  signature          signature to identify FW.
 * @param  offset             offset of the image.
 * @param  length             length of data.
 * @param  p_void             pointer to data.
 * @return 0 if write FW image successfully, error line number otherwise
*/
uint32_t sil_dfu_update(uint16_t signature, uint32_t offset, uint32_t length,
                        uint32_t/*void*/ *p_void)
{
    uint32_t result = 0;
    uint32_t dfu_base_addr;
    uint32_t start_addr;

    DFU_PRINT_INFO3("====>dfu_update: signature=0x%x, offset=%d, length=%d", signature, offset, length);

    if (length % 4)
    {
        result = __LINE__;
        goto L_Return;
    }

    if (p_void == 0)
    {
        result = __LINE__;
        goto L_Return;
    }

    if (IMAGE_USER_DATA != signature)
    {
        /*get back up area address*/
#ifdef SDK_8772
        dfu_base_addr = get_temp_ota_bank_addr_by_img_id((T_IMG_ID)signature);
#else

#if (SUPPORT_OTA_APP_DATA_EXTENSION == 1)
        dfu_base_addr = flash_ioctl(flash_ioctl_get_temp_bank_addr_by_image_id_ext, (T_IMG_ID)signature, 0);
#else
        dfu_base_addr = get_temp_ota_bank_addr_by_img_id((T_IMG_ID)signature);
#endif

#endif
    }
    else
    {
        /*get back up area address*/
        dfu_base_addr = flash_get_bank_addr(FLASH_BKP_DATA1);
    }

    if (dfu_base_addr == 0)
    {
        result = __LINE__;
        goto L_Return;
    }

    /* before erase temp image or write image to flash temp, check access length depend flash layout */
    if (!check_dfu_update_image_length(signature, offset, length, p_void, &result))
    {
        goto L_Return;
    }

    /*if it's start_packet*/
    if (offset == 0)
    {
        T_IMG_CTRL_HEADER_FORMAT *p_header = (T_IMG_CTRL_HEADER_FORMAT *) p_void;
        /*access ram addr, make sure image is not ready*/
        p_header->ctrl_flag.flag_value.not_ready = 0x1;
        DFU_PRINT_TRACE2("====>dfu_update: New Image Header! signature=0x%x, dfu_base_addr=0x%x",
                         signature, dfu_base_addr);
    }

    if ((offset % FMC_SEC_SECTION_LEN) == 0)   //new page starts
    {
        flash_erase_sector(dfu_base_addr + offset);
    }
    else  // cross page
    {
        if ((offset / FMC_SEC_SECTION_LEN) != ((offset + length) / FMC_SEC_SECTION_LEN))
        {
            flash_erase_sector((dfu_base_addr + offset + length) & ~(FMC_SEC_SECTION_LEN - 1));
        }
    }
    start_addr = dfu_base_addr + offset;

#if (SUPPORT_SILENT_OTA_128M_USER_DATA == 1)
    result = sil_dfu_write(start_addr, length, (uint8_t *)p_void);
    if (result)
    {
        goto L_Return;
    }
#else
    for (int i = 0; i < length; i = i + 4)
    {
        uint32_t s_val;
        flash_auto_write_locked(start_addr + i, *(uint32_t *)p_void);

        flash_auto_read_locked((start_addr + i) | FLASH_OFFSET_TO_NO_CACHE, &s_val);
        if (s_val != *(uint32_t *)p_void)
        {
            DFU_PRINT_TRACE3("<====dfu_update:error! w_data=0x%x, r_data=0x%x, adddr=0x%x",
                             *(uint32_t *)p_void, s_val, start_addr + i);
            result = __LINE__;
            goto L_Return;
        }
        else
        {
            p_void++;
        }
    }
#endif

//    ota_offset = offset + length; //for re-ota
    if (IMAGE_USER_DATA == signature)
    {
        appdefine_offset = offset + length;
    }

L_Return:

    DFU_PRINT_INFO1("<====dfu_update result:%d", result);
    return result;
}

#if (SUPPORT_SILENT_OTA_128M_USER_DATA == 1)
static void slient_dfu_read_image_data(uint8_t *dest, uint8_t *sour, uint32_t len)
{
    uint32_t counter = 0;
    if (OTP->image_split_read)
    {
        flash_split_read_locked((uint32_t)sour, len, dest, &counter);  //better performance
    }
    else
    {
        flash_read_locked((uint32_t)sour, len, dest);
    }
}

static uint16_t slient_dfu_process_crc(uint16_t fcs, uint8_t *buf, uint32_t buf_max_size,
                                       uint8_t *pdata,
                                       size_t data_len)
{
    uint16_t loop_cnt = data_len / buf_max_size;
    uint16_t remain_size = data_len % buf_max_size;
    uint16_t i;

    for (i = 0; i < loop_cnt; ++i)
    {
        slient_dfu_read_image_data(buf, pdata, buf_max_size);
        fcs = btxfcs(fcs, buf, buf_max_size);
        pdata += buf_max_size;
    }
    if (remain_size)
    {
        if ((remain_size % 4) == 0)
        {
            slient_dfu_read_image_data(buf, pdata, remain_size);
            fcs = btxfcs(fcs, buf, remain_size);
        }
        else
        {
            fcs = btxfcs(fcs, pdata, remain_size);
        }
    }

    return fcs;
}

/**
 * @brief  calculated CRC checksum over the image, and compared with given checksum value.
 *
 * @param  p_header          image header info of the given image.
 * @return true if image integrity check pass via checksum compare, false otherwise.
*/
bool slient_dfu_check_crc(T_IMG_CTRL_HEADER_FORMAT *p_header)
{
    uint8_t buf[CRC_BUFFER_SIZE];
    uint16_t calced_crc16;
    uint16_t img_crc16;
    uint32_t temp;

    uint32_t length = IMG_HEADER_SIZE - DFU_HEADER_SIZE - 256 - 16;
    uint8_t *pdata = (uint8_t *) p_header + DFU_HEADER_SIZE;
    calced_crc16 = slient_dfu_process_crc(BTXFCS_INIT, buf, CRC_BUFFER_SIZE, pdata, length);

    //get payload length
    flash_read_locked((uint32_t)p_header + 8, 4, (uint8_t *)&length);
    length += 16;
    pdata = (uint8_t *) p_header + IMG_HEADER_SIZE - 16;
    calced_crc16 = slient_dfu_process_crc(calced_crc16, buf, CRC_BUFFER_SIZE, pdata, length);

    //get crc16
    flash_read_locked((uint32_t)p_header + 4, 4, (uint8_t *)&temp);
    img_crc16 = temp >> 16;

    DFU_PRINT_INFO2("<====slient_dfu_check_crc: checksum16=0x%04x, expect=0x%04x",
                    calced_crc16, img_crc16);

    return (img_crc16 == calced_crc16);
}


static void slient_dfu_process_sha256(SHA256_CTX *ctx, uint8_t *buf, uint32_t buf_max_size,
                                      uint8_t *pdata,
                                      size_t data_len)
{
    uint16_t loop_cnt = data_len / buf_max_size;
    uint16_t remain_size = data_len % buf_max_size;
    uint16_t i;

    for (i = 0; i < loop_cnt; ++i)
    {
        slient_dfu_read_image_data(buf, pdata, buf_max_size);
        SHA256_Update(ctx, buf, buf_max_size);
        pdata += buf_max_size;
    }
    if (remain_size)
    {
        slient_dfu_read_image_data(buf, pdata, remain_size);
        SHA256_Update(ctx, buf, remain_size);
    }
}


bool slient_dfu_check_sha256(T_IMG_CTRL_HEADER_FORMAT *p_header)
{
    SHA256_CTX ctx;
    uint8_t buf[SHA256_BUFFER_SIZE];
    uint8_t sha256sum[SHA256_LENGTH];
    uint8_t sha256img[SHA256_LENGTH];
    uint32_t payload_len = 0;

    SHA256_Init(&ctx);

    uint8_t *pdata = (uint8_t *)p_header + DFU_HEADER_SIZE;
    slient_dfu_process_sha256(&ctx, buf, SHA256_BUFFER_SIZE, pdata, 360);

    pdata = (uint8_t *)p_header + 404;
    slient_dfu_process_sha256(&ctx, buf, SHA256_BUFFER_SIZE, pdata, 348);

    pdata = (uint8_t *)p_header + IMG_HEADER_SIZE - 16;
    //get payload length in img header
    flash_read_locked((uint32_t)p_header + 8, 4, (uint8_t *)&payload_len);
    slient_dfu_process_sha256(&ctx, buf, SHA256_BUFFER_SIZE, pdata, (16 + payload_len));

    SHA256_Final(&ctx, sha256sum);
    //get sha256 in img header
    flash_read_locked((uint32_t)p_header + 372, sizeof(sha256img), sha256img);

    DFU_PRINT_INFO2("slient_dfu_check_sha256: Calculating result: %b %b",
                    TRACE_BINARY(SHA256_LENGTH, sha256sum), TRACE_BINARY(SHA256_LENGTH, sha256img));

    return (memcmp(sha256img, sha256sum, SHA256_LENGTH) == 0);
}

/**
 * @brief  calculated checksum(CRC16 or SHA256 determined by image) over the image, and compared
 *         with given checksum value.
 * @param  p_header image header info of the given image.
 * @return true if image integrity check pass via checksum compare, false otherwise.
*/
bool silent_check_image_chksum(T_IMG_CTRL_HEADER_FORMAT *p_header)
{
    bool ret;
    uint16_t crc16 = 0;
    uint32_t r_data = 0;
    ret = flash_read_locked((uint32_t)p_header + 4, sizeof(r_data), (uint8_t *)&r_data);
    if (ret)
    {
        crc16 = r_data >> 16;
        DFU_PRINT_INFO1("silent_check_image_chksum: header->crc16=0x%x", crc16);

        if (crc16)
        {
            ret = slient_dfu_check_crc(p_header);
        }
        else
        {
            ret = slient_dfu_check_sha256(p_header);
        }
    }

    return ret;
}
#endif

/**
 * @brief calculate checksum of the image and compare with given checksum value.
 *
 * @param   image_id     signature to identify image.
 * @return  true if the image integrity check passes, false otherwise
*/
bool silent_dfu_check_checksum(uint16_t image_id)
{
    bool check_result;

    if (image_id == IMAGE_USER_DATA)
    {
        silent_dfu_reset(image_id);

        /* because flash_get_bank_addr(FLASH_BKP_DATA1) is not located flash block protect range, needn't unlock bp */
        T_IMG_CTRL_HEADER_FORMAT *p_appdefine_header = (T_IMG_CTRL_HEADER_FORMAT *)(
                                                           flash_get_bank_addr(FLASH_BKP_DATA1) | FLASH_OFFSET_TO_NO_CACHE);
        /*use flash auto mode if OTP->image_split_read == 0, can skip flash_lock and unlock*/
        if (OTP->image_split_read)
        {
            flash_lock(FLASH_LOCK_USER_MODE_READ);
        }
#if (SUPPORT_SILENT_OTA_128M_USER_DATA == 1)
        check_result = silent_check_image_chksum(p_appdefine_header);
#else
        check_result = check_image_chksum(p_appdefine_header);
#endif
        /*flash auto mode when OTP->image_split_read = 0, skip flash_lock and unlock*/
        if (OTP->image_split_read)
        {
            flash_unlock(FLASH_LOCK_USER_MODE_READ);
        }
        /*don't need unlock bp*/

        /*
        mark image as ready:
        p_appdefine_header->ctrl_flag.flag_value.not_ready = 0;
        there are two reasons we must use user mode API:
        1. support flash larger than 64M bits.
        2. support 4 bit mode: PRM enabled and programming non cache space doesn't supported.
        */
        uint32_t r_data;
        flash_read_locked((uint32_t)p_appdefine_header, sizeof(r_data), (uint8_t *)&r_data);
        r_data &= ~BIT23; /* not_ready */
        flash_write_locked((uint32_t)p_appdefine_header, sizeof(r_data), (uint8_t *)&r_data);
    }
    else
    {
        /* need modify p_header->ctrl_flag.not_ready, so first unlock flash bp*/
        unlock_flash_all();
        /*use flash auto mode if OTP->image_split_read == 0, can skip flash_lock and unlock*/
        if (OTP->image_split_read)
        {
            flash_lock(FLASH_LOCK_USER_MODE_READ);
        }
        check_result = dfu_check_checksum(image_id);
        /*flash auto mode when OTP->image_split_read = 0, skip flash_lock and unlock*/
        if (OTP->image_split_read)
        {
            flash_unlock(FLASH_LOCK_USER_MODE_READ);
        }
        lock_flash();
    }

    return check_result;
}

bool silent_dfu_reset(uint16_t image_id)
{
    bool ret = true;
    if (image_id == IMAGE_USER_DATA)
    {
        appdefine_offset = 0;
    }
    else
    {
        ret = dfu_reset(image_id);
    }
    return ret;
}








