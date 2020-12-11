/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file      data_uart_test.c
* @brief    This file provides data uart test.
* @details
* @author  chenjie
* @date     2018-05-29
* @version  v1.0
*********************************************************************************************************
*/

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "board.h"
#include <data_uart_test.h>
#include <version.h>
#include <patch_header_check.h>
#include <gap.h>
#include <string.h>
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gpio.h"
#include "rtl876x_uart.h"
#include "gap_bond_le.h"
#if MP_TEST_MODE_SUPPORT_AUTO_K_RF
#include "auto_k_rf_bonding_dut.h"
#include "auto_k_rf.h"
#endif
#include "rtl876x_wdg.h"
#include "rtl876x_lib_platform.h"
#include "test_mode.h"
#include "flash_adv_cfg.h"

#if MP_TEST_MODE_SUPPORT_DATA_UART_TEST

#define TEST_DEVICE_TYPE 0x21  /* Bee2 DUT device type */

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
static bool uart_test_is_dlps_allowed = true;

/*============================================================================*
 *                              Local Functions
 *============================================================================*/
static void uart_test_get_cmd_func(UART_PacketTypeDef *p_data);
static void uart_test_read_patch_version(void *p_data);
static void uart_test_read_app_version(void *p_data);
static void uart_test_read_mac_addr(void *p_data);
static void uart_test_write_mac_addr(void *p_data);
static void uart_test_enter_hci_test_mode(void *p_data);
static void uart_test_reboot_device(void *p_data);
static void uart_test_get_dut_info(void *p_data);
static void uart_test_auto_k_rf_freq(void *p_data);
static void uart_test_find_device_type(void *p_data);
static void uart_test_direct_k_rf_freq(void *p_data);
static void uart_test_manual_k_rf_freq(void *p_data);
static void uart_test_enter_hci_download_mode(void *p_data);
static void uart_test_enter_ampere_test(void *p_data);
static void uart_test_get_sum(void *p_data);
static void uart_test_io(void *p_data);
static void uart_test_change_baudrate(void *p_data);
static void uart_test_verify_dut_info(void *p_data);
static void uart_test_enter_single_tone(void *p_data);

/*============================================================================*
 *                              Global Variables
 *============================================================================*/
/**<  Array of all used test function informations */
const T_UART_TEST_PROTOCOL uart_test_func_map[UART_TEST_SUPPORT_NUM] =
{
    /* Opcode, Parameter Length, Function */
    {READ_PATCH_VERSION_CMD, 0, uart_test_read_patch_version},
    {READ_APP_VERSION_CMD, 0, uart_test_read_app_version},
    {READ_MAC_ADDR_CMD, 0, uart_test_read_mac_addr},
    {ENTER_FAST_PAIR_MODE_CMD, 1, NULL},
    {GET_DEVICE_STATE_CMD, 0, NULL},
    {VOICE_TEST_START_CMD, 0, NULL},
    {VOICE_TEST_STOP_CMD, 0, NULL},
    {SET_VOICE_CONFIG_CMD, 0, NULL},
    {GET_VOICE_CONFIG_CMD, 0, NULL},
    {ENTER_DLPS_TEST_MODE_CMD, 0, NULL},
    {START_STOP_ADV_CMD, 1, NULL},
    {START_IR_TEST_MODE_CMD, 0, NULL},
    {ENTER_HCI_TEST_MODE_CMD, 0, uart_test_enter_hci_test_mode},
    {DISABLE_TEST_MODE_FLG_CMD, 0, NULL},
    {ENABLE_TEST_MODE_FLG_CMD, 0, NULL},
    {ERASE_PAIR_INFO_CMD, 0, NULL},
    {CHANGE_BAUDRATE_CMD, 1, uart_test_change_baudrate},
    {DIRECT_K_RF_FREQ_CMD, 2, uart_test_direct_k_rf_freq},
    {GET_GLODEN_INFO_CMD, 0, NULL},
    {GET_DUT_INFO_CMD, 32, uart_test_get_dut_info},
    {VERIFY_DUT_INFO_CMD, 0, uart_test_verify_dut_info},
    {AUTO_K_RF_FREQ_CMD, 18, uart_test_auto_k_rf_freq},
    {FIND_DEVICE_TYPE_CMD, 0, uart_test_find_device_type},
    {REBOOT_DEVICE_CMD, 0, uart_test_reboot_device},
    {UPDATE_MAC_ADDR_CMD, 6, uart_test_write_mac_addr},
    {ENTER_SINGLE_TONE_MODE_CMD, 1, uart_test_enter_single_tone},//support
    {READ_HARDWARE_VERSION_CMD, 0, NULL},
    {TERMINATE_CONNECT_CMD, 0, NULL},
    {MANUAL_K_RF_FREQ_CMD, 1, uart_test_manual_k_rf_freq},
    {ENTER_HCI_DOWNLOAD_MODE_CMD, 0, uart_test_enter_hci_download_mode},
	
    {ENTER_AMPERE_TEST_CMD, 0, uart_test_enter_ampere_test},
    {GET_CHECKSUM_CMD,1,uart_test_get_sum},      //support need to add content
    {IO_TEST_CMD,1,uart_test_io},                 //support need to add content
    //Add more command here,Please store in order according to opcode!
};

void uart_test_get_sum(void *p_data)
{
    uint8_t cmd_type = *((uint8_t *)p_data + 3);
    uint32_t checksum = 0;
    uint32_t address =0;
    switch(cmd_type)
    {
        case 0: //patch checksum
            {
                address = 0x80d000;
                checksum = *(uint32_t *)address;
            }
            break;
        case 1: //app checksum
                address = 0x87FFFC;
                checksum = *(uint32_t *)address;
            break;
        default:
            break; 
    }
    DBG_DIRECT("tian debug checksum=0x%08x\n",checksum);
    UARTCmd_Response(GET_CHECKSUM_CMD, UART_TEST_SUCCESS, (uint8_t *)&checksum, 4);
}
#define MODULE_PIN5       P4_1
#define MODULE_PIN6       P4_0
#define MODULE_PIN7       P0_6
#define MODULE_PIN8       P0_5
#define MODULE_PIN11     P3_3
#define MODULE_PIN12     P3_2
#define MODULE_PIN13     P2_2
#define MODULE_PIN14     P2_3
#define MODULE_PIN15     P2_4
#define MODULE_PIN16     P2_5

uint8_t  module_io_pin[]={MODULE_PIN5,MODULE_PIN6,MODULE_PIN7,MODULE_PIN8,MODULE_PIN11,\
                                             MODULE_PIN12,MODULE_PIN13,MODULE_PIN14,MODULE_PIN15,MODULE_PIN16};
static void test_board_gpio_init(void)
{    
        uint8_t pin_num=0;
        uint8_t i;
        pin_num=sizeof(module_io_pin);
        DBG_DIRECT("tian debug test_board_gpio_init num=%d\n",pin_num);
        for(i=0;i<pin_num;i++){
            Pad_Config(module_io_pin[i], PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);    
            Pinmux_Config(module_io_pin[i], DWGPIO);            
        }
}
void test_driver_gpio_init(void)
 {
      uint8_t pin_num=0;
      uint8_t i;
      GPIO_InitTypeDef GPIO_InitStruct;   
      pin_num=sizeof(module_io_pin);
      DBG_DIRECT("tian debug test_driver_gpio_init num=%d\n",pin_num);
      RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);  
      GPIO_StructInit(&GPIO_InitStruct);   
      GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_OUT;   
      GPIO_InitStruct.GPIO_ITCmd  = DISABLE; 
      for(i=0;i<pin_num;i++){
          GPIO_InitStruct.GPIO_Pin    = GPIO_GetPin(module_io_pin[i]);    
          GPIO_Init(&GPIO_InitStruct);
      }   
 }
void gpio_all_clear(void){
    uint8_t i;
    uint32_t gpiopins=0;
    for(i=0;i<sizeof(module_io_pin);i++)
    {
        //DBG_DIRECT("tian debug pin %08x\n",GPIO_GetPin(module_io_pin[i]);
        gpiopins |= GPIO_GetPin(module_io_pin[i]);
    }
    DBG_DIRECT("tian debug gpiopins=%08x\n",gpiopins);
    GPIO_ResetBits(gpiopins);
}
void gpio_init(void)
{    
    test_board_gpio_init();    /* Initialize gpio peripheral */   
    test_driver_gpio_init(); 
	gpio_all_clear();
}
void gpio_test_control(uint8_t pin_num)
{
    uint8_t test_num;

    test_num=pin_num-1;
    DBG_DIRECT("tian debug test pin %d\n",pin_num);
    gpio_all_clear();
    GPIO_WriteBit(GPIO_GetPin(module_io_pin[test_num]), (BitAction)(1));
}
static void uart_test_io(void *p_data)
{
    //uint8_t cmd_type = *((uint8_t *)p_data + 3);
    uint8_t IO_Test= *((uint8_t *)p_data + 3);
    static uint8_t gpio_inited=0;
    DBG_DIRECT("tian debug type=%d,data=%04x\n",IO_Test);
    if(gpio_inited==0){
            gpio_init();
            gpio_inited=1;
    }
    if(IO_Test==0 || IO_Test>sizeof(module_io_pin)){
        gpio_init();
        gpio_inited=1;
        UARTCmd_Response(IO_TEST_CMD, UART_TEST_ERROR, NULL, 0);
    }else{
         gpio_test_control(IO_Test);//test io
         UARTCmd_Response(IO_TEST_CMD, UART_TEST_SUCCESS, NULL, 0);
    }
}
/**
  * @brief Read patch command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_read_patch_version(void *p_data)
{
    uint16_t patch_version = 0;
    uint8_t patch_version_bytes[2] = {0};

    T_IMG_HEADER_FORMAT *p_header;
    uint32_t addr = get_header_addr_by_img_id(RomPatch);
    p_header = (T_IMG_HEADER_FORMAT *)addr;

    if (p_header)
    {
        patch_version = p_header->git_ver.ver_info.version;
    }
    else
    {
        patch_version = 0x00;
    }

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[uart_test_read_patch_version] patch version: %d", 1,
                    patch_version);

    patch_version_bytes[0] = (uint8_t)(patch_version & 0x00FF);
    patch_version_bytes[1] = (uint8_t)(patch_version >> 8);

    /* Response data */
    UARTCmd_Response(READ_PATCH_VERSION_CMD, UART_TEST_SUCCESS, patch_version_bytes, 2);
}

/**
  * @brief Read App command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
 typedef union
{
    uint32_t dword;
    uint8_t byte[4];
    struct
    {
        uint32_t first:  4; // 
        uint32_t second: 8; // 
        uint32_t third:  15; // 
        uint32_t forth:  5; //
    } bit;
} appverchanger_t;
void uart_test_read_app_version(void *p_data)
{
 	// 从 PRODUCT_VERSION 宏上获取
 	// 注意要转换，有点变态 如1.2.3.4
 	// 00100 000000000000011 00000010 0001  版本号转换
 	// 00100000 00000000 00110000 00100001  对应传输[3]~[0]
 	appverchanger_t appver;
	appver.bit.first  = 1;
	appver.bit.second = 0;
	appver.bit.third  = 13;
	appver.bit.forth  = 0;
    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[uart_test_read_app_version] app version: %d.%d.%d.%d", 4,
                    appver.byte[0], appver.byte[1], appver.byte[2], appver.byte[3]);

    /* Response data */
    UARTCmd_Response(READ_APP_VERSION_CMD, UART_TEST_SUCCESS, appver.byte, 4);
}

/**
  * @brief Read Mac address command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_read_mac_addr(void *p_data)
{
    uint8_t mac_addr[6];
    gap_get_param(GAP_PARAM_BD_ADDR, mac_addr);

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                    "[uart_test_read_mac_addr] MAC Addr: %02X %02X %02X %02X %02X %02X",
                    6, mac_addr[5], mac_addr[4], mac_addr[3], mac_addr[2], mac_addr[1], mac_addr[0]);

    /* Response data */
    UARTCmd_Response(READ_MAC_ADDR_CMD, UART_TEST_SUCCESS, mac_addr, 6);
}

/**
  * @brief Write MAC address command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_write_mac_addr(void *p_data)
{
    uint8_t mac_addr[6];

    memcpy(mac_addr, (uint8_t *)p_data + 3, 6);

    DBG_DIRECT("[uart_test_write_mac_addr] MAC Addr: %02X %02X %02X %02X %02X %02X",
               mac_addr[5], mac_addr[4], mac_addr[3], mac_addr[2], mac_addr[1], mac_addr[0]);

    if (UpdateMAC(mac_addr))
    {
        UARTCmd_Response(UPDATE_MAC_ADDR_CMD, UART_TEST_SUCCESS, NULL, 0);
        DBG_DIRECT("[uart_test_write_mac_addr] UpdateMAC reboot!");
        WDG_SystemReset(RESET_ALL, UART_CMD_RESET);
    }
    else
    {
        UARTCmd_Response(UPDATE_MAC_ADDR_CMD, UART_TEST_ERROR, NULL, 0);
    }
}

/**
  * @brief Enter HCI test mode command.
  * @param None.
  * @retval None.
  */
void uart_test_enter_hci_test_mode(void *pPacket)
{
    UARTCmd_Response(ENTER_HCI_TEST_MODE_CMD, UART_TEST_SUCCESS, NULL, 0);

    DBG_DIRECT("[uart_test_enter_hci_test_mode] Enter HCI Mode!!!");
    switch_to_hci_mode();
}

/**
  * @brief Reboot device command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_reboot_device(void *p_data)
{
    DBG_DIRECT("[uart_test_reboot_device] reboot device!");

    UARTCmd_Response(REBOOT_DEVICE_CMD, UART_TEST_SUCCESS, NULL, 0);

    WDG_SystemReset(RESET_ALL, UART_CMD_RESET);
}
#define BT_CHANNEL_MAX 40
void uart_test_enter_single_tone(void *p_data)
{
	uint8_t test_channel=0;

	test_channel=*((uint8_t *)p_data + 3);
    DBG_DIRECT("[uart_test_enter_single_tone] tian Switch to Single Tone test mode!,ch=%d\n",test_channel);

	if(test_channel<BT_CHANNEL_MAX){
	    UARTCmd_Response(ENTER_SINGLE_TONE_MODE_CMD, UART_TEST_SUCCESS, NULL, 0);
	    switch_to_test_mode(SINGLE_TONE_MODE);
	}
	else{		
		UARTCmd_Response(ENTER_SINGLE_TONE_MODE_CMD, UART_TEST_ERROR, NULL, 0);
	}
}

/**
  * @brief Get DUT information command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_get_dut_info(void *p_data)
{
#if MP_TEST_MODE_SUPPORT_AUTO_K_RF
    bool result = false;
    uint8_t golden_info[32];
    uint8_t dut_info[32];

    memset(dut_info, 0, 32);
    memcpy(golden_info, (uint8_t *)p_data + 3, 32);
    result = dut_info_get(golden_info, dut_info);

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                    "[uart_test_get_dut_info] result is %d", 1, result);

    if (result == true)
    {
        UARTCmd_Response(GET_DUT_INFO_CMD, UART_TEST_SUCCESS, dut_info, 32);
    }
    else
    {
        UARTCmd_Response(GET_DUT_INFO_CMD, UART_TEST_ERROR, NULL, 0);
    }
#else
    UARTCmd_Response(GET_DUT_INFO_CMD, UART_TEST_ERROR, NULL, 0);
#endif
}

/**
  * @brief Direct K RF frequency command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_direct_k_rf_freq(void *p_data)
{
#if MP_TEST_MODE_SUPPORT_AUTO_K_RF
    uint8_t cal_xtal_result;
    uint8_t rx_channel = *((uint8_t *)p_data + 3);
    uint8_t drift_threshold = *((uint8_t *)p_data + 4);

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                    "[uart_test_direct_k_rf_freq] rx_channel is %d, drift_threshold is %d",
                    2, rx_channel, drift_threshold);

    if (Auto_K_RF(rx_channel, drift_threshold, &cal_xtal_result))
    {
        UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_direct_k_rf_freq] Cal RF freqency failed", 0);
        UARTCmd_Response(DIRECT_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
    }
    else
    {
        bool write_xtal_ret;

#ifdef WRITE_XTAL_TO_EFUSE
        write_xtal_ret = WriteXtalToEfuse(cal_xtal_result);
#else
        write_xtal_ret = WriteXtalToConfig(cal_xtal_result);
#endif
        if (write_xtal_ret == true)
        {
            UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_direct_k_rf_freq] Cal RF freqency successed",
                            0);
            UARTCmd_Response(DIRECT_K_RF_FREQ_CMD, UART_TEST_SUCCESS, &cal_xtal_result, 1);
        }
        else
        {
            UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_direct_k_rf_freq] Cal RF freqency failed", 0);
            UARTCmd_Response(DIRECT_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
        }
    }
#endif
}

/**
  * @brief Get DUT information command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_auto_k_rf_freq(void *p_data)
{
#if MP_TEST_MODE_SUPPORT_AUTO_K_RF
    bool result = false;
    uint8_t result_info[16];
    uint8_t cal_xtal_result;

    uint8_t rx_channel = *((uint8_t *)p_data + 19);
    uint8_t drift_threshold = *((uint8_t *)p_data + 20);

    memcpy(result_info, (uint8_t *)p_data + 3, 16);
    result = check_verify_result(result_info);

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                    "[uart_test_auto_k_rf_freq] rx_channel is %d, drift_threshold is %d, check_result is %d",
                    3, rx_channel, drift_threshold, result);

    if (result == true)
    {
        if (Auto_K_RF(rx_channel, drift_threshold, &cal_xtal_result))
        {
            UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_auto_k_rf_freq] Cal RF freqency failed", 0);
            UARTCmd_Response(AUTO_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
        }
        else
        {
            bool write_xtal_ret;

#ifdef WRITE_XTAL_TO_EFUSE
            write_xtal_ret = WriteXtalToEfuse(cal_xtal_result);
#else
            write_xtal_ret = WriteXtalToConfig(cal_xtal_result);
#endif
            if (write_xtal_ret == true)
            {
                UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_auto_k_rf_freq] Cal RF freqency successed", 0);
                UARTCmd_Response(AUTO_K_RF_FREQ_CMD, UART_TEST_SUCCESS, &cal_xtal_result, 1);
            }
            else
            {
                UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_auto_k_rf_freq] Cal RF freqency failed", 0);
                UARTCmd_Response(AUTO_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
            }
        }
    }
    else
    {
        UARTCmd_Response(AUTO_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
    }
#else
    UARTCmd_Response(AUTO_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
#endif
}

/**
  * @brief  Find device type command.
  * @param  p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_find_device_type(void *p_data)
{
    uint8_t dev_type = TEST_DEVICE_TYPE;

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,  "[uart_test_find_device_type] dev_type is %d", 1,
                    dev_type);
    UARTCmd_Response(FIND_DEVICE_TYPE_CMD, UART_TEST_SUCCESS, &dev_type, 1);
}

/**
  * @brief  Manual K RF frequency command.
  * @param  p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_manual_k_rf_freq(void *p_data)
{
    uint8_t xtal = *((uint8_t *)p_data + 3);

    if (WriteXtalToConfig(xtal) == true)
    {
        UARTCmd_Response(MANUAL_K_RF_FREQ_CMD, UART_TEST_SUCCESS, NULL, 0);
    }
    else
    {
        UARTCmd_Response(MANUAL_K_RF_FREQ_CMD, UART_TEST_ERROR, NULL, 0);
    }
}

/**
  * @brief Enter HCI uart test mode command.
  * @param p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_enter_hci_download_mode(void *p_data)
{
    DBG_DIRECT("[uart_test_enter_hci_download_mode] Switch to HCI UART download mode!");

    flash_set_block_protect_locked(0);
    flash_erase_locked(FLASH_ERASE_SECTOR, get_header_addr_by_img_id(OTA));

    UARTCmd_Response(ENTER_HCI_DOWNLOAD_MODE_CMD, UART_TEST_SUCCESS, NULL, 0);

    WDG_SystemReset(RESET_ALL, UART_CMD_RESET);
}
#include "mesh_beacon.h"
#include "gap_scheduler.h"
static void uart_test_enter_ampere_test(void *p_data)
{
    DBG_DIRECT("[uart_test_enter_ampere_test] messure current,close scan beacon wakeup!");

	beacon_stop();  
	gap_sched_scan(false);   // gap layer scan
	// it will not sleep before the production test

    UARTCmd_Response(ENTER_AMPERE_TEST_CMD, UART_TEST_SUCCESS, NULL, 0);
}

#include "data_uart.h"
static void uart_test_change_baudrate(void *p_data)
{
    uint8_t index = *((uint8_t *)p_data+3);
	data_uart_baudrate_t baudrate[3]={DATA_UART_BAUDRATE_115200, DATA_UART_BAUDRATE_1000000, DATA_UART_BAUDRATE_2000000};
	
	if(index<3){
		data_uart_baudrate(baudrate[index]);
   		UARTCmd_Response(CHANGE_BAUDRATE_CMD, UART_TEST_SUCCESS, NULL, 0);
	}else{
   		UARTCmd_Response(CHANGE_BAUDRATE_CMD, UART_TEST_ERROR, NULL, 0);
	}

}

static void uart_test_verify_dut_info(void *p_data)
{
#if MP_TEST_MODE_SUPPORT_AUTO_K_RF
    bool result = false;
    uint8_t verify[16];

    memset(verify, 0, 16);
    result = check_verify_result(verify);

    UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                    "[uart_test_verify_dut_info] result is %d", 1, result);

    if (result == true)
    {
        UARTCmd_Response(VERIFY_DUT_INFO_CMD, UART_TEST_SUCCESS, verify, 16);
    }
    else
    {
        UARTCmd_Response(VERIFY_DUT_INFO_CMD, UART_TEST_ERROR, NULL, 0);
    }
#else
    UARTCmd_Response(VERIFY_DUT_INFO_CMD, UART_TEST_ERROR, NULL, 0);
#endif
}


/**
  * @brief  Get the spefied uart command.
  * @param  p_data: point to UART packet struct.
  * @retval None.
  */
void uart_test_get_cmd_func(UART_PacketTypeDef *p_data)
{
    uint16_t opcode = (p_data->Buf[2] << 8) + p_data->Buf[1];

    UART_DBG_BUFFER(MODULE_UART, LEVEL_INFO, "[uart_test_get_cmd_func] opcode is 0x%04X", 1, opcode);

    /* uart test command */
    if ((opcode >= UART_FN_BEGIN) && (opcode < UART_FN_END))
    {
        if (uart_test_func_map[opcode & UART_FN_MASK].fncb != NULL)
        {
            uart_test_func_map[opcode & UART_FN_MASK].fncb(p_data);
        }
        else
        {
            UART_DBG_BUFFER(MODULE_APP, LEVEL_ERROR, "No uart test cmd function!", 0);
        }
    }
    else
    {
        //Other command
    }
}

/*============================================================================*
 *                              Global Functions
 *============================================================================*/
/**
  * @brief initialize uart test function.
  * @param None.
  * @retval None.
  */
void uart_test_init(void)
{
    //UART_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "[uart_test_init] initialize uart test mode", 0);
	  DBG_DIRECT("uart_test_init!");
    UartTransport_Init();
    uart_test_is_dlps_allowed = false;
}

/**
  * @brief Uart Test Check DLPS allowed or not.
  * @param voide.
  * @retval true or false.
  */
bool uart_test_check_dlps_allowed(void)
{
    return uart_test_is_dlps_allowed;
}

/**
 * @brief handle UART message
 * @param io_driver_msg_recv - recieved io message
 * @return none
 * @retval void
 */
void uart_test_handle_uart_msg(T_IO_MSG io_driver_msg_recv)
{
    UART_PacketTypeDef *pUartTestPacket = (UART_PacketTypeDef *)(io_driver_msg_recv.u.buf);

    if (Packet_Decode(pUartTestPacket))
    {
        uart_test_get_cmd_func(pUartTestPacket);
    }
}

#endif

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

