/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     battery_driver.c
* @brief    rcu vbat adc sample.
* @details
* @author   Yuyin_zhang
* @date     2018-05-16
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#ifdef BAT_EN
#include <string.h>
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_adc.h"
#include "rtl876x_nvic.h"
#include "rtl876x_lpc.h"
#include "os_timer.h"
#include "trace.h"
#include "battery_driver.h"
#include "bas.h"
#include "app_msg.h"
#include "rcu_swtimer.h"
#if BAT_LOW_POWER_INDICATE
#include "led_driver.h"
#endif

/* Globals ------------------------------------------------------------------*/
float  vbat_rate  = 0;
float  vbat_const = 0;
vbat_stg vbat_data = {0};
bool is_adc_efuse_existed = false;

extern bool app_send_msg_to_apptask(T_IO_MSG *p_msg);

/**
  * @brief   check whether battery module is working.
  * @param   No parameter.
  * @return  bool true allow dlps, or not.
  */
bool is_vbat_working(void)
{
    return true;
}

/**
  * @brief   Initialize battery adc peripheral.
  * @param   No parameter.
  * @return  void
  */
static void battery_driver_init(void)
{
    ADC_DeInit(ADC);
    RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

    ADC_InitTypeDef adcInitStruct;
    ADC_StructInit(&adcInitStruct);
    adcInitStruct.schIndex[0]         = INTERNAL_VBAT_MODE;
    adcInitStruct.schIndex[1]         = INTERNAL_VBAT_MODE;
    adcInitStruct.schIndex[2]         = INTERNAL_VBAT_MODE;
    adcInitStruct.schIndex[3]         = INTERNAL_VBAT_MODE;
    adcInitStruct.bitmap              = 0x0F;
    adcInitStruct.adcSamplePeriod     = 128;
    /* TIM7 for timing voltage acqusition(fix) */
    //adcInitStruct.timerTriggerEn      = ENABLE;
    ADC_Init(ADC, &adcInitStruct);
    //ADC_INTConfig(ADC, ADC_INT_ONE_SHOT_DONE, ENABLE);

    is_adc_efuse_existed = ADC_CalibrationInit();
}

/**
  * @brief   set battery calibration parameter through read eFuse.
  * @param   No parameter.
  * @return  bool  set success.
  */
static bool set_battery_para(void)
{
    /*read battery calibration rate*/
    vbat_rate = 0.8037;
    /*read battery calibration constant*/
    vbat_const = 0;

    return true;
}

/**
* @brief  lpc interrupt handler function.
* @param   No parameter.
* @return  void
*/
void battery_lpc_handler(void)
{
    LPC_INTConfig(LPC_INT_VOLTAGE_COMP, DISABLE);
    APP_PRINT_ERROR0("[BAT] LPC lower power triggled!");

    T_IO_MSG bee_io_msg = {0};
    bee_io_msg.type = IO_MSG_TYPE_BAT_LPC;
    if (false == app_send_msg_to_apptask(&bee_io_msg))
    {
        APP_PRINT_ERROR0("[BAT] LPC send IO_MSG_TYPE_LPC message failed!");
        return;
    }
}

/**
  * @brief   lpc driver init.
  * @param   void.
  * @return  void.
  */
static void lpc_driver_init(void)
{
    LPC_InitTypeDef LPC_InitStruct;
    LPC_StructInit(&LPC_InitStruct);
    LPC_InitStruct.LPC_Channel   = LPC_CHANNEL_VBAT;
    LPC_InitStruct.LPC_Edge      = LPC_Vin_Below_Vth;
    LPC_InitStruct.LPC_Threshold = BAT_LPC_CONFIG;
    LPC_Init(&LPC_InitStruct);
    LPC_Cmd(ENABLE);
    /* Enable voltage detection interrupt.If Vin<Vth, cause this interrupt */
    //LPC_INTConfig(LPC_INT_VOLTAGE_COMP, ENABLE);

    RamVectorTableUpdate(LPCOMP_VECTORn, battery_lpc_handler);
}

void bat_nvic_config(void)
{
    /* Config LPC interrupt */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = LPCOMP_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    LPC_INTConfig(LPC_INT_VOLTAGE_COMP, ENABLE);
}

/**
  * @brief   Initialize rcu battery module.
  * @param   No parameter.
  * @return  void
  */
void bat_module_init(void)
{
    lpc_driver_init();
    battery_driver_init();
    /*set calibration param*/
    set_battery_para();
    vbat_data.vbat_low_power_threshold = (uint16_t)BAT_LOW_POWER_THRESHOLD;
    vbat_data.vbat_mode = NORMAL_MODE;

    /*enable adc*/
    //ADC_Cmd(ADC, ADC_One_Shot_Mode, ENABLE);
    //RamVectorTableUpdate(ADC_VECTORn, battery_adc_handler);
}

/**
  * @brief   get battery mode.
  * @param   No parameter.
  * @return  NORMAL_MODE or LOW_POWER_MODE.
  */
BAT_MODE get_bat_mode(void)
{
    return (BAT_MODE)vbat_data.vbat_mode;
}

/**
  * @brief      calculation battery calibration voltage.
  * @param[in]  data: data read from ADC.
  * @return     ADC voltage whose unit is mv.
  */
uint16_t vbat_calibrate_voltage(uint16_t data)
{
    float adc_voltage = 0;
    ADC_ErrorStatus error_status = NO_ERROR;
    if (true == is_adc_efuse_existed)
    {
        adc_voltage = ADC_GetVoltage(DIVIDE_SINGLE_MODE, (int32_t)data, &error_status);
        if (error_status < 0)
        {
            APP_PRINT_WARN1("ADC parameter or efuse data error %d!", error_status);
            adc_voltage = (-620) * data * data / 100000000 + (0.98399) * data - 125.6;
        }
    }
    else
    {
        adc_voltage = (-620) * data * data / 100000000 + (0.98399) * data - 125.6;
    }
    return (uint16_t)adc_voltage;
}

/**
  * @brief   update battery value, level and mode.
  * @param   src_dat: adc sample source data.
  * @return  void.
  */
void update_vbat_value(uint16_t src_data)
{
    vbat_data.vbat_value = (uint16_t)vbat_calibrate_voltage(src_data);

    /*calculate bat level according to bat value*/
    if (vbat_data.vbat_value >= 3100)/*>3.1V, 100%*/
    {
        vbat_data.vbat_level = 100;
    }
    else if (vbat_data.vbat_value >= 3000) /*3.1~3.0V, 100%~80%*/
    {
        vbat_data.vbat_level = 80 + (vbat_data.vbat_value - 3000) * (100 - 80) / (3100 - 3000);
    }
    else if (vbat_data.vbat_value >= 2500) /*3.0~2.5V, 80%~30%*/
    {
        vbat_data.vbat_level = 30 + (vbat_data.vbat_value - 2500) * (80 - 30) / (3000 - 2500);
    }
    else if (vbat_data.vbat_value >= 220) /*2.5~2.2V, 30%~15%*/
    {
        vbat_data.vbat_level = 15 + (vbat_data.vbat_value - 2200) * (30 - 15) / (2500 - 2200);
    }
    else/*<2.2V, <15%*/
    {
        vbat_data.vbat_level = (vbat_data.vbat_value) * (15) / (2200);
    }

    APP_PRINT_ERROR2("[BAT] update_vbat_value, value: %d, level: %d!", vbat_data.vbat_value,
                     vbat_data.vbat_level);

    /*check battery mode*/
    if (vbat_data.vbat_value < vbat_data.vbat_low_power_threshold)
    {
        vbat_data.vbat_mode = LOW_POWER_MODE;
    }
    else
    {
        vbat_data.vbat_mode = NORMAL_MODE;
    }

}
/**
  * @brief   get battery value.
  * @param   p_level  battery level, p_value battery value.
  * @return  none.
  */
void battery_get_value(uint16_t *p_level, uint16_t *p_value)
{
    uint32_t sum = 0;
    uint16_t adc_arr[4] = {0};
    uint16_t min = 0xffff, max = 0;

    ADC_INTConfig(ADC, ADC_INT_ONE_SHOT_DONE, ENABLE);
    ADC_Cmd(ADC, ADC_One_Shot_Mode, ENABLE);

    uint32_t delay = 0;

    /*5000 timeout: 1ms at 40M Clock*/
    while ((ADC_GetIntFlagStatus(ADC, ADC_INT_ONE_SHOT_DONE) != SET)
           && ((delay++ < 5000)));

    ADC_ClearINTPendingBit(ADC, ADC_INT_ONE_SHOT_DONE);

    for (uint8_t index = 0; index < 4; index ++)
    {
        adc_arr[index] = ADC_ReadByScheduleIndex(ADC, (ADC_SCHEDULE_INDEX + index));

        sum += adc_arr[index];

        if (min > adc_arr[index])
        {
            min = adc_arr[index];
        }
        if (max < adc_arr[index])
        {
            max = adc_arr[index];
        }
    }

    sum = (sum - min - max) >> 1;

    update_vbat_value((uint16_t)sum);

    if ((p_value != NULL) && (p_level != NULL))
    {
        *p_value = vbat_data.vbat_value;
        *p_level = (uint16_t)vbat_data.vbat_level;
    }

    APP_PRINT_ERROR1("[BAT] battery_get_value, new value: %d!", vbat_data.vbat_value);
}

/**
* @brief  ADC interrupt handler function.
* @param   No parameter.
* @return  void
*/
/*void battery_adc_handler(void)
{
    if (ADC_GetIntFlagStatus(ADC, ADC_INT_ONE_SHOT_DONE) == SET)
    {
        uint16_t bat_src_data = 0;
        ADC_ClearINTPendingBit(ADC, ADC_INT_ONE_SHOT_DONE);
        bat_src_data = ADC_ReadByScheduleIndex(ADC, ADC_SCHEDULE_INDEX);

        vbat_data.is_working = false;
        update_vbat_value(bat_src_data);
    }
}
*/

#if BAT_LOW_POWER_INDICATE
void battery_low_power_indicate(void)
{
    LED_BLINK(LED_1, LED_TYPE_BLINK_LOW_POWER, 5);
}
#endif

/**
* @brief   bat message handle.
* @param   msg_sub_type the msg type to handle.
* @return  void
*/
void battery_msg_handle(uint16_t msg_sub_type)
{
    APP_PRINT_ERROR1("[BAT] battery_msg_handle, type = %d!", msg_sub_type);
    switch (msg_sub_type)
    {
    case IO_MSG_TYPE_BAT_LPC:
        {
            uint16_t value, level = 0;
            battery_get_value(&level, &value);
            if (value < BAT_ADC_DETECT_THRESHOLD)
            {
                vbat_data.vbat_mode = LOW_POWER_MODE;
            }
        }
        break;
    case IO_MSG_TYPE_BAT_DETECT:
        {
            uint16_t value, level = 0;
#if BAT_LOW_POWER_INDICATE
            battery_low_power_indicate();
#endif
            battery_get_value(&level, &value);
            if (value >= BAT_ADC_DETECT_THRESHOLD)
            {
                vbat_data.vbat_mode = NORMAL_MODE;
                APP_PRINT_INFO0("[BAT] exit low power mode.");
                /*rework lpc function*/
                LPC_INTConfig(LPC_INT_VOLTAGE_COMP, ENABLE);
            }
            else
            {
                APP_PRINT_INFO0("[BAT] low power mode, keyscan not work.");
            }
        }
        break;
    default:
        break;
    }
}

#endif
/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

