/**
*****************************************************************************************
*     Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file    system_rtl8762c.c
   * @brief   system init file
   * @author  lory xu
   * @date    2017-11-9
   * @version v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2016 Realtek Semiconductor Corporation</center></h2>
   * *************************************************************************************
  */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "version.h"
#include "rtl876x.h"
#include "patch_header_check.h"
#include "app_section.h"
#include "rom_uuid.h"
#include "app_define.h"
#include "trace.h"
#include "core_cmFunc.h"
//#include "debug_monitor.h"
#include "mem_config.h"
#include "otp.h"
#include "platform_autoconf.h"
#include "rtl876x_wdg.h"
#include "overlay_mgr.h"
#include "flash_device.h"
#include "os_sched.h"
#include "otp_config.h"
#include "test_mode.h"
#include "platform_utils.h"
#include "os_mem.h"
#include "crc16btx.h"
#include "rtl876x_wdg.h"
#include "dfu_api.h"
#if (SYSTEM_TRACE_ENABLE == 1)
#include "system_trace.h"
#include "trace_config.h"
#endif

#if (FTL_APP_CALLBACK_ENABLE == 1)  //enable ftl cb in otp_config.h
#include "ftl_app_cb.h"
#endif

#define DATA_SIZE_FOR_RANDOM_SEED    16
#if (HEAP_DATA_ON_SIZE < DATA_SIZE_FOR_RANDOM_SEED)
#error "heap on size should be larger then DATA_SIZE_FOR_RANDOM_SEED"
#endif

static void AppUpdateVectorTable(void);
void SystemInit(void);
void app_cb0_wdg_reset(T_WDG_MODE wdg_mode, T_SW_RESET_REASON reset_reason);
void app_cb5_dfu_status(DFU_PROGRESS_STATUS status, uint32_t length);

extern void GPIO0_Handler(void);
extern void GPIO1_Handler(void);
extern void GPIO2_Handler(void);
extern void GPIO3_Handler(void);
extern void GPIO4_Handler(void);
extern void GPIO5_Handler(void);
extern void GPIO6_Handler(void);
extern void GPIO7_Handler(void);
extern void GPIO8_Handler(void);
extern void GPIO9_Handler(void);
extern void GPIO10_Handler(void);
extern void GPIO11_Handler(void);
extern void GPIO12_Handler(void);
extern void GPIO13_Handler(void);
extern void GPIO14_Handler(void);
extern void GPIO15_Handler(void);
extern void GPIO16_Handler(void);
extern void GPIO17_Handler(void);
extern void GPIO18_Handler(void);
extern void GPIO19_Handler(void);
extern void GPIO20_Handler(void);
extern void GPIO21_Handler(void);
extern void GPIO22_Handler(void);
extern void GPIO23_Handler(void);
extern void GPIO24_Handler(void);
extern void GPIO25_Handler(void);
extern void GPIO26_Handler(void);
extern void GPIO27_Handler(void);
extern void GPIO28_Handler(void);
extern void GPIO29_Handler(void);
extern void GPIO30_Handler(void);
extern void GPIO31_Handler(void);

extern uint32_t __flash_start_load_ad__[];
extern uint32_t *__encryption_ram_code_load_ad__;
extern uint32_t *__encryption_ram_code_exe_ad__;
extern uint32_t *__encryption_ram_code_length__;
extern char Image$$ENCRYPTION_RAM_CODE$$Base[];
extern char Load$$ENCRYPTION_RAM_CODE$$Base[];
extern char Load$$ENCRYPTION_RAM_CODE$$Length[];

extern char Image$$FLASH_START_ADDR$$RO$$Base[];
extern char Load$$FLASH_START_ADDR$$RO$$Base[];
extern char Load$$FLASH_START_ADDR$$RO$$Length[];
/** @defgroup  SYSTEM_INIT System Init
    * @brief Start up code for user application.
    * @{
    */
/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup SYSTEM_INIT_Exported_Macros System Init Exported Macros
    * @brief
    * @{
    */
#define SHARE_CACHE_RAM_0K          0x82F70000
#define SHARE_CACHE_RAM_8K          0x2F2D0002
#define SHARE_CACHE_RAM_16K         0xA2AA0003

#define VTOR_RAM_ADDR               0x00200000 //!< vector table address in RAM.
#define SIGNATURE_APP_ENTRY_ENTRY   0xd592
#if (FTL_APP_CALLBACK_ENABLE == 1)
#define APP_CB_NUMBERS              6
#else
#define APP_CB_NUMBERS              6
#endif
#define APP_FAKE_PAYLOAD_LEN        0x100

#define DEFAULT_HEADER_SIZE         1024

#define RESET_RAM_PATTERN           0x726574
/** End of SYSTEM_INIT_Exported_Macros
    * @}
    */

/*============================================================================*
 *                              Types
 *============================================================================*/
/** @defgroup SYSTEM_INIT_Exported_Types System Init Exported Types
    * @brief
    * @{
    */
typedef struct
{
    uint32_t app_cb_signature;
    uint32_t app_cb_numbers;
    uint32_t app_cb_addr[APP_CB_NUMBERS];
} T_APP_CB_TABLE;

typedef enum
{
    SYSTEM_CALL_BASE            = 0x0,
    SYSTEM_CALL_WDG_RESET,
} T_SYSTEM_CALL_OPCODE;

/** End of SYSTEM_INIT_Exported_Types
    * @}
    */

/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @defgroup SYSTEM_INIT_Exported_Variables System Init Exported Variables
    * @{
    */

BOOL_WDG_CB user_wdg_cb = NULL;
BOOL_DFU_STATUS_CB user_dfu_status_cb __attribute__((weak)) = NULL;

USER_CALL_BACK app_pre_main_cb __attribute__((weak)) = NULL ;

#if FEATURE_ENCRYPTION
#if defined ( __CC_ARM ) || defined ( __GNUC__ )
#define ENC_ALIGN_SECTION __attribute__((aligned(16), used, section(".enc.dummy.align")));
const uint8_t enc_dummy_align[16] ENC_ALIGN_SECTION;
#else
#define ENC_ALIGN_SECTION @ ".enc.dummy.align"
#pragma data_alignment=16
__root const uint8_t enc_dummy_align[16] ENC_ALIGN_SECTION;
#endif
#endif

typedef struct _CHECK_RESET_RAM_RECORD
{
    uint32_t check_reset_ram_pattern : 24;
    uint32_t check_reset_ram_type : 8;
} T_CHECK_RESET_RAM_RECORD;

//init ram in ram_init();
T_CHECK_RESET_RAM_RECORD check_reset_ram =
{
    RESET_RAM_PATTERN,
    1,    //reset reason default value
};

#if defined ( __CC_ARM   )
#pragma push
#endif
const T_APP_CB_TABLE app_cb_table =
{
    .app_cb_signature = SIGNATURE_APP_CB,
    .app_cb_numbers = APP_CB_NUMBERS,
#if defined ( __CC_ARM   )
#pragma diag_suppress 1296 /* disable warning 1296(extended constant initialiser used)*/
#endif
    .app_cb_addr[0] = (uint32_t)app_cb0_wdg_reset,
#if (FTL_APP_CALLBACK_ENABLE == 1)
    .app_cb_addr[1] = (uint32_t)ftl_init_app_cb,
    .app_cb_addr[2] = (uint32_t)ftl_read_app_cb,
    .app_cb_addr[3] = (uint32_t)ftl_write_app_cb,
    .app_cb_addr[4] = (uint32_t)ftl_gc_imp_app_cb,
    .app_cb_addr[5] = (uint32_t)app_cb5_dfu_status,
#else
    .app_cb_addr[1] = (uint32_t)NULL,
    .app_cb_addr[2] = (uint32_t)NULL,
    .app_cb_addr[3] = (uint32_t)NULL,
    .app_cb_addr[4] = (uint32_t)NULL,
    .app_cb_addr[5] = (uint32_t)app_cb5_dfu_status,
#endif
    //add more cb here
};
#if defined ( __CC_ARM   )
#pragma pop

#pragma push
#pragma diag_suppress 1296 /* disable warning 1296(extened constant initialiser used)*/
#endif
/**
* @brief: application header.
* @note: items in ENCRYPT_RAM_CODE macro is for encrytion solution only
*/

#if defined ( __ICCARM__   )
#pragma section = ".app.encryption.text"
#pragma section = ".app.encryption.text_init"
#endif

#if defined ( __ICCARM__   )
__root
#endif


const T_IMG_HEADER_FORMAT img_header APP_FLASH_HEADER =
{
    .ctrl_header =
    {
#ifdef SDK_8772
        .ic_type = 0x9,
#else
        .ic_type = 0x5,
#endif
        .secure_version = 0,
#if FEATURE_ENCRYPTION
        .ctrl_flag.flag_value.enc = 1,
        .ctrl_flag.flag_value.xip = 0,
        .ctrl_flag.flag_value.load_when_boot = 1,
        .ctrl_flag.flag_value.enc_key_select = ENC_KEY_OCEK_WITH_OEMCONST,
#else
        .ctrl_flag.flag_value.xip = 1,
        .ctrl_flag.flag_value.enc = 0,
        .ctrl_flag.flag_value.load_when_boot = 0,
        .ctrl_flag.flag_value.enc_key_select = 0,
#endif
        .ctrl_flag.flag_value.enc_load = 0,
        .ctrl_flag.flag_value.not_ready = 0,
        .ctrl_flag.flag_value.not_obsolete = 1,
#if (BOOT_INTEGRITY_CHECK_EN == 0)
        .ctrl_flag.flag_value.integrity_check_en_in_boot = 0,
#else
        .ctrl_flag.flag_value.integrity_check_en_in_boot = 1,
#endif
        .image_id = AppPatch,
        .payload_len = APP_FAKE_PAYLOAD_LEN,    //Will modify by build tool later
    },
    .uuid = DEFINE_rom_uuid,

#if FEATURE_ENCRYPTION
#if defined ( __ICCARM__ )
    .exe_base = (uint32_t)__section_begin(".app.encryption.text"),
    .load_base = (uint32_t)__section_begin(".app.encryption.text_init"),
    .load_len = (uint32_t)__section_size(".app.encryption.text"),
#elif defined (__CC_ARM)
    .exe_base = (uint32_t)Image$$ENCRYPTION_RAM_CODE$$Base,
    .load_base = (uint32_t)Load$$ENCRYPTION_RAM_CODE$$Base,
    .load_len = (uint32_t)Load$$ENCRYPTION_RAM_CODE$$Length,
#else
    .exe_base = (uint32_t) &__encryption_ram_code_exe_ad__, //(uint32_t)IMAGE_FLASH_START_ADDR,
    .load_base = (uint32_t) &__encryption_ram_code_load_ad__,
    .load_len = (uint32_t) &__encryption_ram_code_length__,
#endif
#else
#if defined ( __ICCARM__ )
    .load_base = 0,
    .exe_base = (uint32_t)SystemInit,
#elif defined (__CC_ARM)
    .load_base = (uint32_t)Load$$FLASH_START_ADDR$$RO$$Base,
    .exe_base = (uint32_t)Image$$FLASH_START_ADDR$$RO$$Base,
#else
    .load_base = 0,
    .exe_base = (uint32_t)SystemInit, //(uint32_t)IMAGE_FLASH_START_ADDR,
#endif
    .load_len = 0,  //0 indicates all XIP
#endif

#if (APP_BANK == 0)
    .img_base = BANK0_APP_ADDR,
#else
    .img_base = BANK1_APP_ADDR,
#endif

    .git_ver =
    {
        .ver_info.sub_version._version_major = VERSION_MAJOR,
        .ver_info.sub_version._version_minor = VERSION_MINOR,
        .ver_info.sub_version._version_revision = VERSION_REVISION,
        .ver_info.sub_version._version_reserve = VERSION_BUILDNUM % 32, //only 5 bit
        ._version_commitid = VERSION_GCID,
        ._customer_name = {CN_1, CN_2, CN_3, CN_4, CN_5, CN_6, CN_7, CN_8},
    },

    .app_cb_signature = SIGNATURE_APP_CB,
    .app_cb_table_base_address = (uint32_t) &app_cb_table
};

#if defined ( __ICCARM__   )
__root
#endif
#if defined ( __ICCARM__   ) || defined ( __CC_ARM   )
const T_AUTH_HEADER_FORMAT auth_header APP_FLASH_HEADER =
{
#if defined ( __ICCARM__ )
    .header_mac = {0xFF},
    .payload_mac = {0xFF},
#else
    .header_mac = {[0 ... 15] = 0xFF},
    .payload_mac = {[0 ... 15] = 0xFF},
#endif
};
#elif defined ( __GNUC__   )
const T_AUTH_HEADER_FORMAT auth_header APP_FLASH_HEADER_AUTH =
{
    .header_mac = {[0 ... 15] = 0xFF},
    .payload_mac = {[0 ... 15] = 0xFF},
};
#endif
#if defined ( __CC_ARM   )
#pragma pop
#endif


/** End of SYSTEM_INIT_Exported_Variables
    * @}
    */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup SYSTEM_INIT_Exported_Functions System Init Exported Functions
    * @{
    */
void common_main(void)
{
//add common system code here before enter user defined main function
    OTP->run_in_app = 1;

#if (RUN_APP_IN_HCIMODE_ENABLE == 0)
    if (OTP->stack_en)
    {
        DBG_DIRECT("In SoC Mode");
    }
    else
    {
        DBG_DIRECT("WARNING: In HCI Mode, will not run APP Task");
        WDG_Disable();
        os_sched_start();
    }
#endif

    //fix system hang if app call log_module_trace_set after enable RELEASE_VERSION
    if (OTP->logDisable == 1)
    {
        log_module_trace_init(NULL);
    }

#if (SYSTEM_TRACE_ENABLE == 1)
    extern void system_trace_init(void);
    system_trace_init();
#endif

#if defined ( __ICCARM__ )
    extern void __iar_program_start(const int);
    __iar_program_start(0);
#elif defined (__CC_ARM)
    extern int __main(void);
    __main();
#elif defined (__GNUC__)
    extern int main(void);
    main();
#endif
}

#if (DEBUG_WATCHPOINT_ENABLE == 1)
/**
 * @brief  Enable Debug Monitor Function (include NVIC Enable and DWT configuration)
 * @param  none
 * @return none
 */

void debug_monitor_enable(void) DATA_RAM_FUNCTION
{
    //DBG_DIRECT("debug_monitor_enable");

    //set debug monitor priority
    NVIC_SetPriority(DebugMonitor_IRQn, 3);

    //enable exception and monitor control register
    CoreDebug->DEMCR |= CoreDebug_DEMCR_MON_EN_Msk | CoreDebug_DEMCR_TRCENA_Msk;

    //set DWT compare registers (max 4 comparators)
    //watch_point_0_setting(0x1000180C, DWT_DATAVSIZE_WORD, DWT_FUNCTION_WRITE);
    //watch_point_1_setting(0x10000004, DWT_DATAVSIZE_WORD, DWT_FUNCTION_READ_OR_WRITE);
    //watch_point_2_setting(0x10000008, DWT_DATAVSIZE_WORD, DWT_FUNCTION_READ_OR_WRITE);
    //watch_point_3_setting(0x1000000C, DWT_DATAVSIZE_WORD, DWT_FUNCTION_READ_OR_WRITE);

    //enable DWT control register
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    return;
}
#endif
void ram_init(void) APP_FLASH_TEXT_SECTION;
void ram_init(void)
{
#if defined ( __CC_ARM   )
    //copy data on ro
    extern char Image$$RAM_DATA_ON$$RO$$Base[];
    extern char Load$$RAM_DATA_ON$$RO$$Base[];
    extern unsigned int Image$$RAM_DATA_ON$$RO$$Length;

    memcpy(Image$$RAM_DATA_ON$$RO$$Base,
           Load$$RAM_DATA_ON$$RO$$Base,
           (unsigned int)&Image$$RAM_DATA_ON$$RO$$Length);

    //copy data on rw
    extern char Image$$RAM_DATA_ON$$RW$$Base[];
    extern char Load$$RAM_DATA_ON$$RW$$Base[];
    extern unsigned int Image$$RAM_DATA_ON$$RW$$Length;

    memcpy(Image$$RAM_DATA_ON$$RW$$Base,
           Load$$RAM_DATA_ON$$RW$$Base,
           (unsigned int)&Image$$RAM_DATA_ON$$RW$$Length);

    //clear data on zi
    extern char Image$$RAM_DATA_ON$$ZI$$Base[];
    extern unsigned int Image$$RAM_DATA_ON$$ZI$$Length;

    memset(Image$$RAM_DATA_ON$$ZI$$Base,
           0,
           (unsigned int)&Image$$RAM_DATA_ON$$ZI$$Length);

    //copy cache ro
    extern char Image$$CACHE_DATA_ON$$RO$$Base[];
    extern char Load$$CACHE_DATA_ON$$RO$$Base[];
    extern unsigned int Image$$CACHE_DATA_ON$$RO$$Length;

    memcpy(Image$$CACHE_DATA_ON$$RO$$Base,
           Load$$CACHE_DATA_ON$$RO$$Base,
           (unsigned int)&Image$$CACHE_DATA_ON$$RO$$Length);

    //copy share cache ram rw
    extern char Image$$CACHE_DATA_ON$$RW$$Base[];
    extern char Load$$CACHE_DATA_ON$$RW$$Base[];
    extern unsigned int Image$$CACHE_DATA_ON$$RW$$Length;

    memcpy(Image$$CACHE_DATA_ON$$RW$$Base,
           Load$$CACHE_DATA_ON$$RW$$Base,
           (unsigned int)&Image$$CACHE_DATA_ON$$RW$$Length);

    //clear share cache ram zi
    extern char Image$$CACHE_DATA_ON$$ZI$$Base[];
    extern unsigned int Image$$CACHE_DATA_ON$$ZI$$Length;

    memset(Image$$CACHE_DATA_ON$$ZI$$Base,
           0,
           (unsigned int)&Image$$CACHE_DATA_ON$$ZI$$Length);
#elif defined ( __GNUC__   )
    //copy data on rw
    extern uint32_t *__ram_dataon_rw_start__;
    extern uint32_t *__ram_rw_load_ad__;
    extern uint32_t *__ram_dataon_rw_length__;

    memcpy(&__ram_dataon_rw_start__,
           &__ram_rw_load_ad__,
           (unsigned int)&__ram_dataon_rw_length__);

    //clear data on zi
    extern uint32_t *__ram_dataon_zi_start__;
    extern uint32_t *__ram_dataon_zi_length__;

    memset(&__ram_dataon_zi_start__,
           0,
           (unsigned int)&__ram_dataon_zi_length__);

    //copy share cache ram rw
    extern uint32_t *__cache_dataon_start__;
    extern uint32_t *__cache_data_load_ad__;
    extern uint32_t *__cache_dataon_length__;

    memcpy(&__cache_dataon_start__,
           &__cache_data_load_ad__,
           (unsigned int)&__cache_dataon_length__);

    //clear share cache ram zi


#endif
}

uint32_t random_seed_value;

void random_seed_init(void) APP_FLASH_TEXT_SECTION;
void random_seed_init(void)
{
    uint16_t crc16;
    uint8_t buf[DATA_SIZE_FOR_RANDOM_SEED];
    uint8_t *sour_addr = (uint8_t *)(OTP->appDataAddr + OTP->appDataSize + OTP->heapDataONSize);

    memcpy(buf, sour_addr, DATA_SIZE_FOR_RANDOM_SEED);
    crc16 = btxfcs(0, buf, DATA_SIZE_FOR_RANDOM_SEED);

    random_seed_value = (crc16 << 16) | (*(volatile uint32_t *)(0x4005817C) & 0xFFFF);

    random_seed_value *= platform_random(0xFFFFFFFF);

    srand(random_seed_value);

    for (int i = 0; i < (DATA_SIZE_FOR_RANDOM_SEED / 4); ++i)
    {
        *(uint32_t *)(sour_addr + i * 4) = rand();
    }

    random_seed_value = rand();
}

#if defined ( __ICCARM__ )
#if FEATURE_ENCRYPTION
void SystemInit(void) APP_ENCRYPTION_TEXT_SECTION;
#else
void SystemInit(void) APP_FLASH_TEXT_SECTION;
#endif
#else
void SystemInit(void) APP_FLASH_TEXT_SECTION;
#endif
void SystemInit(void)
{
    //hci mode check and bypass app
    if (check_hci_mode_flag() || (OTP->stack_en == 0))
    {
        return;
    }

    //init pre_main and main functions
    extern void pre_main(void);
    app_pre_main = (APP_MAIN_FUNC)pre_main;
    app_main = (APP_MAIN_FUNC)common_main;

#if (ENABLE_WRITE_HARDFAULT_RECORD_TO_FLASH > 0) && (SYSTEM_TRACE_ENABLE == 1) && (TRACE_HARDFAULT == 1)
    patch_hardfault_save_to_flash_init();
#endif

#if (WRITE_REASON_TO_FLASH_BEFORE_RESET_ENABLE > 0) && (SYSTEM_TRACE_ENABLE == 1) && (TRACE_WDG_TIMEOUT == 1)
    patch_wdg_timeout_reason_save_to_flash_init();
#endif

    /******** update otp here**********/
    //ram config
    OTP->appDataSize = APP_GLOBAL_SIZE;
    OTP->heapDataONSize = HEAP_DATA_ON_SIZE - DATA_SIZE_FOR_RANDOM_SEED;

    /* not share: SHARE_CACHE_RAM_0K; 8K: SHARE_CACHE_RAM_8K; 16K: SHARE_CACHE_RAM_16K */
#if ((16 * 1024) == SHARE_CACHE_RAM_SIZE)
    OTP->share_cache_ram_reg = SHARE_CACHE_RAM_16K;
#elif ((8 * 1024) == SHARE_CACHE_RAM_SIZE)
    OTP->share_cache_ram_reg = SHARE_CACHE_RAM_8K;
#else
    OTP->share_cache_ram_reg = SHARE_CACHE_RAM_0K;
#endif

#ifdef SUPPORT_SINGLE_BANK_OTA_USER_DATA
    OTP->bkp_data1_addr = USER_DATA_START_ADDR;
    OTP->bkp_data1_size = USER_DATA_MAX_SIZE;
#endif

    //sw timer config
#ifdef TIMER_MAX_NUMBER
    //define TIMER_MAX_NUMBER in otp_config.h
    OTP->timerMaxNumber = TIMER_MAX_NUMBER;
#endif

    //flash config
    /*config enable flash block proect depending on flash layout and flash id*/
#if (FLASH_BLOCK_PROTECT_ENABLE == 1)
    OTP->bp_enable = 1;
#else
    OTP->bp_enable = 0;
#endif
    OTP->delay_10us_after_toggle_cs = AFTER_TOGGLE_CS_DELAY;

    OTP->image_split_read = 0; //use flash auto mode to check image

    //os config
    /*config enable check task stack overflow*/
#if (CHECK_STACK_OVERFLOW_ENABLE == 1)
    OTP->checkForStackOverflow = 1;
#else
    OTP->checkForStackOverflow = 0;
#endif


    //platform config
    /*config enable platform assert*/
#if (PLATFORM_ASSERT_ENABLE == 1)
    OTP->enableASSERT = 1;
#else
    OTP->enableASSERT = 0;
#endif

    /*Print all log in log buffer before entering DLPS */
#if (CHECK_LOG_BUFFER_BEFORE_DLPS_ENABLE == 1)
    OTP->printAllLogBeforeEnterDLPS = 1;
#else
    OTP->printAllLogBeforeEnterDLPS = 0;
#endif

    /*config enable log or not*/
#if (CONFIG_LOG_FUNCTION_ENABLE == 1)
    OTP->logDisable = 0;
#else
    OTP->logDisable = 1;
#endif

    /*to fix bug need disable dump callstack info before WDG_SystemReset, default enable */
#if (DUMP_INFO_BEFORE_RESET_DISABLE == 1)
    OTP->dump_info_before_reset = 0;
#endif

    /*before wdg system reset, write reset reason to specific flash addr*/
#if (WRITE_REASON_TO_FLASH_BEFORE_RESET_ENABLE > 0)
    OTP->write_reason_to_flash_before_reset = WRITE_REASON_TO_FLASH_BEFORE_RESET_ENABLE;
    OTP->reboot_reason_record_address = REBOOT_REASON_RECORD_ADDRESS;
    OTP->reboot_reason_record_limit_power_2 = REBOOT_REASON_RECORD_LIMIT_POWERT2;
#endif

    /*config enable swd pinmux*/
#if (SWD_PINMUX_ENABLE == 1)
    OTP->SWD_ENABLE = 1;
#else
    OTP->SWD_ENABLE = 0;
#endif

    /*config enable watch dog in rom*/
#if (ROM_WATCH_DOG_ENABLE == 1)
    OTP->wdgEnableInRom = 1;
#else
    OTP->wdgEnableInRom = 0;
#endif

    /*config watch dog mode in rom, defualt 4s timeout and reset all*/
#if (ROM_WATCH_DOG_ENABLE == 1)
    OTP->wdgConfigDivfactor = ROM_WATCH_DOG_CFG_DIV_FACTOR;
    OTP->wdgConfigCntLimit = ROM_WATCH_DOG_CFG_CNT_LIMIT;
    OTP->wdgMode = ROM_WATCH_DOG_MODE;
#endif

    /*use os tick as log timestamp instead of TIM7*/
    OTP->log_timestamp_src = LOG_TIMESTAMP_OS;

#if (XTAL_40M_DELAY_TIME > 0)
    /*Do not modify this setting unless you know what you are doing*/
    OTP->LOP_XTAL_delay = XTAL_40M_DELAY_TIME;
#endif

    /*config enable write hardfault record to flash*/
#if (ENABLE_WRITE_HARDFAULT_RECORD_TO_FLASH > 0)
    OTP->write_info_to_flash_when_hardfault = ENABLE_WRITE_HARDFAULT_RECORD_TO_FLASH;
    OTP->HardFault_Record_BegAddr = HARDFAULT_RECORD_BEG_ADDR;
    OTP->HardFault_Record_EndAddr = HARDFAULT_RECORD_END_ADDR;
    OTP->HardFault_Record_CFG = HARDFAULT_RECORD_CFG;
#endif

#if (ENABLE_FLASH_READ_TURN_OFF_RF > 0)
    OTP->read_turn_on_off_rf = ENABLE_FLASH_READ_TURN_OFF_RF;
#endif

#ifdef SDK_8772

#if (USE_ALIOS == 0)
    OTP->UseAliOS = 0x0;
#else
    OTP->UseAliOS = 0x1;
#endif

#endif

    //app config
    OTP->ota_timeout_total = OTA_TIMEOUT_TOTAL;
    OTP->ota_timeout_wait4_conn = OTA_TIMEOUT_WAIT4_CONN;
    OTP->ota_timeout_wait4_image_transfer = OTA_TIMEOUT_WAIT4_IMAGE_TRANS;
    OTP->ota_timeout_ctittv = OTA_TIMEOUT_CTITTV;

#if ROM_OTA_LINKLOSS_RST
    OTP->ota_link_loss_reset = 1;
#endif
    /*config bt stack parameters in rom*/
#ifdef BT_STACK_CONFIG_ENABLE
    bt_stack_config_init();
#endif

#if defined ( __ICCARM__ )
    app_pre_main_cb = 0;
#endif

//add more otp config here
}

//Note: call print_reset_reason() before ram_init();
#ifdef SDK_8772
void print_reset_reason(void) APP_FLASH_TEXT_SECTION
{
    T_SW_RESET_REASON sw_reset_type = reset_reason_get();
    if (sw_reset_type == RESET_REASON_HW)
    {
        BOOT_PRINT_INFO0("RESET Reason: HW");
    }
    else
    {
        BOOT_PRINT_INFO1("RESET Reason: SW, TYPE 0x%x", sw_reset_type);
    }
}
#else

void print_reset_reason(void) APP_FLASH_TEXT_SECTION;
void print_reset_reason(void)
{
    if (check_reset_ram.check_reset_ram_pattern != RESET_RAM_PATTERN)
    {
        BOOT_PRINT_INFO0("RESET Reason: HW or OTA");
    }
    else
    {
        T_SW_RESET_REASON sw_reset_type = get_aon_record_reset_reason();

        if (sw_reset_type != (T_SW_RESET_REASON)0)
        {
            BOOT_PRINT_INFO1("RESET Reason: SW(reset except aon), TYPE 0x%x", sw_reset_type);
        }
        else //reset all will clear aon register
        {
            BOOT_PRINT_INFO1("RESET Reason: SW(reset all), TYPE 0x%x", check_reset_ram.check_reset_ram_type);
        }
    }
}
#endif

void pre_main(void) APP_FLASH_TEXT_SECTION;
void pre_main(void)
{
    __disable_irq();

    print_reset_reason();  //Note: call this function before ram_init();

    ram_init();

    random_seed_init();

#if defined ( __CC_ARM   )
    load_overlay(OVERLAY_SCENARIO_BOOT_ONCE);
#endif

    //reset NVIC of DMA channel used in image decryption
    NVIC_DisableIRQ(GDMA0_Channel2_IRQn);
    NVIC_DisableIRQ(GDMA0_Channel3_IRQn);

#if defined ( __CC_ARM   )
    setlocale(LC_ALL, "C");
#endif

    BOOT_PRINT_ERROR2("SDK Ver: %s, Build Time: %s",
                      TRACE_STRING(VERSION_BUILD_STR),
                      TRACE_STRING(BUILDING_TIME));

    AppUpdateVectorTable();

#if (DEBUG_WATCHPOINT_ENABLE == 1)
    debug_monitor_enable();
#endif

#ifdef SDK_8772
    app_cb_wdg_reset = app_cb0_wdg_reset;
    app_cb_dfu_status = app_cb5_dfu_status;
#endif

    if (app_pre_main_cb)
    {
        app_pre_main_cb();
    }

    return;
}
/**
 * @brief  update vector table in app
 * @param  none
 * @return none
  */

//static void AppUpdateVectorTable(void) OVERLAY_SECTION_BOOT_ONCE;
static void AppUpdateVectorTable(void)
{
#if defined ( __CC_ARM )
    extern uint32_t Load$$RAM_VECTOR_TABLE$$RO$$Base;
    extern uint32_t Image$$RAM_VECTOR_TABLE$$RO$$Length;
#elif defined ( __GNUC__ )
    extern uint32_t *__ram_vector_load_ad__;
    extern uint32_t *__ram_vector_table_length__;
#endif
    extern void Default_Handler(void);
    const char *SysException[] =
    {
        "InitialSP", "Reset", "NMI", "HardFault", "MemManage", "BusFault", "UsageFault", "Rsvd",
        "Rsvd", "Rsvd", "Rsvd", "SVC", "DebugMon", "Rsvd", "PendSV", "SysTick"
    };
    const char *ExtIrq[] =
    {
        "System", "WDG", "BTMAC", "TIM3", "TIM2", "Platform", "I2S0_TX", "I2S0_RX", "Timer4-7",
        "GPIO4", "GPIO5", "UART1", "UART0", "RTC", "SPI0", "SPI1", "I2C0", "I2C1", "ADC",
        "Peripheral", "GDMA0 Channel0", "GDMA0 Channel1", "GDMA0 Channel2", "GDMA0 Channel3",
        "GDMA0 Channel4", "GDMA0 Channel5", "GPIO_Group3", "GPIO_Group2", "IR", "GPIO_Group1",
        "GPIO_Group0", "UART2", "TIM4", "TIM5", "TIM6", "TIM7", "SPI_Flash", "Qdecode",
        "Keyscan", "SPI2W", "LPCOMP", "PTA_Mailbox", "SPORT1 TX", "SPORT1 RX", "LCD"
    };

    IRQ_Fun *pRamVector    = (IRQ_Fun *)VTOR_RAM_ADDR;
#if defined ( __ICCARM__ )
#pragma section = ".intvec"

    IRQ_Fun *pAppVector    = (IRQ_Fun *)__section_begin(".intvec");
    uint32_t AppVectorSize = (uint32_t)__section_size(".intvec");
    DBG_DIRECT("pAppVector = 0x%x, AppVectorSize = %d", pAppVector, AppVectorSize);
#elif defined (__CC_ARM)
    IRQ_Fun *pAppVector    = (IRQ_Fun *)&Load$$RAM_VECTOR_TABLE$$RO$$Base;
    uint32_t AppVectorSize = (uint32_t)&Image$$RAM_VECTOR_TABLE$$RO$$Length;
#elif defined (__GNUC__)
    IRQ_Fun *pAppVector    = (IRQ_Fun *)(&__ram_vector_load_ad__);
    uint32_t AppVectorSize = (uint32_t)&__ram_vector_table_length__;
#endif
    uint32_t i             = 0;

    if (SCB->VTOR != VTOR_RAM_ADDR)
    {
        RamVectorTableInit(VTOR_RAM_ADDR);
    }

    /* Update APP defined handlers */
    for (i = 0; i < AppVectorSize / 4; ++i)
    {
        if (i == 0 || i == 1) //skip __initial_sp and reset_handler remap
        {
            continue;
        }

        if (((pAppVector[i] != Default_Handler) && (pAppVector[i] != 0)) ||
            ((pAppVector[i] == Default_Handler) && (pRamVector[i] == (IRQ_Fun)0x155)))  //rom default handler
        {
            if (i < System_VECTORn)
            {
                OS_PRINT_WARN1("Warning! %s is updated by APP!", TRACE_STRING(SysException[i]));
            }
            else
            {
                OS_PRINT_WARN1("Warning! ISR %s is updated by APP!",
                               TRACE_STRING(ExtIrq[i - System_VECTORn]));
            }

            pRamVector[i] = pAppVector[i];
        }
    }

    __DMB();
    __DSB();
}
/**
 * @brief  GPIO Group3 Handler
 * @param  none
 * @return none
  */
void GPIO_Group3_Handler(void) DATA_RAM_FUNCTION;
void GPIO_Group3_Handler(void)
{
    uint32_t GPIOIrqStatus = GPIO->INTSTATUS;

    //Check exact IRQ function
    if (GPIOIrqStatus & BIT3)
    {
        GPIO3_Handler();
    }
    if (GPIOIrqStatus & BIT7)
    {
        GPIO7_Handler();
    }
    if (GPIOIrqStatus & BIT11)
    {
        GPIO11_Handler();
    }
    if (GPIOIrqStatus & BIT15)
    {
        GPIO15_Handler();
    }
    if (GPIOIrqStatus & BIT19)
    {
        GPIO19_Handler();
    }
    if (GPIOIrqStatus & BIT23)
    {
        GPIO23_Handler();
    }
    if (GPIOIrqStatus & BIT27)
    {
        GPIO27_Handler();
    }
    if (GPIOIrqStatus & BIT31)
    {
        GPIO31_Handler();
    }
}
/**
 * @brief  GPIO Group2 Handler
 * @param  none
 * @return none
  */
void GPIO_Group2_Handler(void) DATA_RAM_FUNCTION;
void GPIO_Group2_Handler(void)
{
    uint32_t GPIOIrqStatus = GPIO->INTSTATUS;

    //Check exact IRQ function
    if (GPIOIrqStatus & BIT2)
    {
        GPIO2_Handler();
    }
    if (GPIOIrqStatus & BIT6)
    {
        GPIO6_Handler();
    }
    if (GPIOIrqStatus & BIT10)
    {
        GPIO10_Handler();
    }
    if (GPIOIrqStatus & BIT14)
    {
        GPIO14_Handler();
    }
    if (GPIOIrqStatus & BIT18)
    {
        GPIO18_Handler();
    }
    if (GPIOIrqStatus & BIT22)
    {
        GPIO22_Handler();
    }
    if (GPIOIrqStatus & BIT26)
    {
        GPIO26_Handler();
    }
    if (GPIOIrqStatus & BIT30)
    {
        GPIO30_Handler();
    }
}
/**
 * @brief  GPIO Group1 Handler
 * @param  none
 * @return none
  */
void GPIO_Group1_Handler(void) DATA_RAM_FUNCTION;
void GPIO_Group1_Handler(void)
{
    uint32_t GPIOIrqStatus = GPIO->INTSTATUS;

    //Check exact IRQ function
    if (GPIOIrqStatus & BIT1)
    {
        GPIO1_Handler();
    }
    if (GPIOIrqStatus & BIT9)
    {
        GPIO9_Handler();
    }
    if (GPIOIrqStatus & BIT13)
    {
        GPIO13_Handler();
    }
    if (GPIOIrqStatus & BIT17)
    {
        GPIO17_Handler();
    }
    if (GPIOIrqStatus & BIT21)
    {
        GPIO21_Handler();
    }
    if (GPIOIrqStatus & BIT25)
    {
        GPIO25_Handler();
    }
    if (GPIOIrqStatus & BIT29)
    {
        GPIO29_Handler();
    }
}
/**
 * @brief  GPIO Group0 Handler
 * @param  none
 * @return none
  */
void GPIO_Group0_Handler(void) DATA_RAM_FUNCTION;
void GPIO_Group0_Handler(void)
{
    uint32_t GPIOIrqStatus = GPIO->INTSTATUS;

    //Check exact IRQ function
    if (GPIOIrqStatus & BIT0)
    {
        GPIO0_Handler();
    }
    if (GPIOIrqStatus & BIT8)
    {
        GPIO8_Handler();
    }
    if (GPIOIrqStatus & BIT12)
    {
        GPIO12_Handler();
    }
    if (GPIOIrqStatus & BIT16)
    {
        GPIO16_Handler();
    }
    if (GPIOIrqStatus & BIT20)
    {
        GPIO20_Handler();
    }
    if (GPIOIrqStatus & BIT24)
    {
        GPIO24_Handler();
    }
    if (GPIOIrqStatus & BIT28)
    {
        GPIO28_Handler();
    }
}

#ifdef SDK_8772
#else
void WDG_SystemReset(T_WDG_MODE wdg_mode, T_SW_RESET_REASON reset_reason)
{
    uint32_t parm = wdg_mode | (reset_reason << 8);
    SystemCall(SYSTEM_CALL_WDG_RESET, parm);
}
#endif

/**
 * @brief  flash try to switch to high speed bit mode
 * @note  switch back to 1 bit mode, if flash switch to high speed bit mode fail
 * @param  bit_mode config bit mode @ref T_FLASH_MODE
 * @retval 0 fail
 * @retval 1 success
  */
uint32_t flash_try_high_speed(T_FLASH_MODE bit_mode)
{
    uint32_t result = 0;
    OTP->bit_mode = bit_mode;
    result = flash_ioctl(flash_ioctrl_try_high_speed, 0, 0);
    //if try fail, set back OTP->bit_mode to one bit mode
    if (!result)
    {
        OTP->bit_mode = FLASH_MODE_1BIT;
    }
    return result;
}

void *malloc(size_t size)
{
    return os_mem_alloc(RAM_TYPE_DATA_ON, size);
}

void *calloc(size_t n, size_t size)
{
    return os_mem_zalloc(RAM_TYPE_DATA_ON, n * size);
}

void *realloc(void *ptr, size_t size)
{
    if (ptr)
    {
        os_mem_free(ptr);
    }

    return os_mem_alloc(RAM_TYPE_DATA_ON, size);
}

void free(void *ptr)
{
    os_mem_free(ptr);
}

void app_cb0_wdg_reset(T_WDG_MODE wdg_mode, T_SW_RESET_REASON reset_reason)
{
    check_reset_ram.check_reset_ram_type = reset_reason;

    if (user_wdg_cb)
    {
        if (user_wdg_cb(wdg_mode, reset_reason))
        {
            return;
        }
    }

    //do something necessary before watch dog reset
}

void app_cb5_dfu_status(DFU_PROGRESS_STATUS status, uint32_t length)
{
//    uint32_t value = 0;
//    switch(status)
//    {
//        case DFU_START:
//            value = *((uint32_t *)0x180e000);
//            DBG_DIRECT("DFU start, total length = %d, value = 0x%x", length, value);
//            break;
//        case DFU_DOING:
//            value = *((uint32_t *)0x180e000);
//            DBG_DIRECT("DFU doing, paccket length = %d, value = 0x%x", length, value);
//            break;
//        case DFU_END:
//            value = *((uint32_t *)0x180e000);
//            DBG_DIRECT("DFU end, check result = %d, value = 0x%x", length, value);
//            break;
//        default:
//            DBG_DIRECT("wrong status input");
//    }
    if (user_dfu_status_cb)
    {
        if (user_dfu_status_cb(status, length))
        {
            return;
        }
    }
}

/**
 * @brief  get cpu clock
 * @param  none
 * @return uint32, for example 40000000 is 40M, 20000000 is 20M.
  */
uint32_t get_cpu_clock(void)
{
    extern uint32_t SystemCpuClock;
    return SystemCpuClock;
}

/**
 * @brief  get 14 bytes EUID
 * @param  none
 * @return uint8_t *, the pointer to a copy of EUID.
  */
uint8_t *get_ic_euid(void)
{
    static uint8_t euid[15] = {0};

    memcpy(euid, (uint8_t *)0x2028F5, 14);
    return euid;
}

/**
 * @brief  get ic type
 * @param  none
 * @return uint8_t, ic type:
 * #define RTL8752CRF              0x08
 * #define RTL8752CGF              0x38
 * #define RTL8762CJ               0x11
 * #define RTL8752CJ               0x01
 * #define RTL8762CJF              0x19
 * #define RTL8752CJF              0x09
 * #define RTL8762CK               0x12
 * #define RTL8762CKF              0x1A
 * #define RTL8762CKO              0x1B
 * #define RTL8752CKO              0x0B
 * #define RTL8762CAF              0x0A
 * #define RTL8762CMF              0x2A
 * #define RTL8752CMF              0x29
 */
uint8_t get_ic_type(void)
{
    return *(uint8_t *)0x002028f3;
}

/** @} */ /* End of group SYSTEM_INIT_Exported_Functions */
/** @} */ /* End of group SYSTEM_INIT */
