/**
*********************************************************************************************************
*               Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_pinmux.h
* @brief
* @details
* @author    Chuanguo Xue
* @date      2015-3-27
* @version   v0.1
* *********************************************************************************************************
*/


#ifndef _RTL876X_PINMUX_H_
#define _RTL876X_PINMUX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rtl876x.h"
#include "rtl876x_bitfields.h"



/** @addtogroup PINMUX PINMUX
  * @brief Pinmux driver module
  * @{
  */




/** @defgroup PINMUX_Exported_Constants PINMUX Exported Constants
  * @{
  */

/** @defgroup Pin_Function_Number Pin Function Number
  * @{
  */

#define IDLE_MODE              0
#define HCI_UART_TX            1
#define HCI_UART_RX            2
#define HCI_UART_CTS           3
#define HCI_UART_RTS           4
#define I2C0_CLK               5
#define I2C0_DAT               6
#define I2C1_CLK               7
#define I2C1_DAT               8
#define PWM2_P                 9
#define PWM2_N                 10
#define PWM3_P                 11
#define PWM3_N                 12
#define timer_pwm0             13
#define timer_pwm1             14
#define timer_pwm2             15
#define timer_pwm3             16
#define timer_pwm4             17
#define timer_pwm5             18
#define timer_pwm6             19
#define timer_pwm7             20
#define qdec_phase_a_x         21
#define qdec_phase_b_x         22
#define qdec_phase_a_y         23
#define qdec_phase_b_y         24
#define qdec_phase_a_z         25
#define qdec_phase_b_z         26
#define UART2_TX               27
#define UART2_RX               28
#define UART1_TX               29
#define UART1_RX               30
#define UART1_CTS              31
#define UART1_RTS              32
#define IRDA_TX                33
#define IRDA_RX                34
#define UART0_TX               35
#define UART0_RX               36
#define UART0_CTS              37
#define UART0_RTS              38
#define SPI1_SS_N_0_MASTER     39
#define SPI1_SS_N_1_MASTER     40
#define SPI1_SS_N_2_MASTER     41
#define SPI1_CLK_MASTER        42
#define SPI1_MO_MASTER         43
#define SPI1_MI_MASTER         44
#define SPI0_SS_N_0_SLAVE      45
#define SPI0_CLK_SLAVE         46
#define SPI0_SO_SLAVE          47
#define SPI0_SI_SLAVE          48
#define SPI0_SS_N_0_MASTER     49
#define SPI0_CLK_MASTER        50
#define SPI0_MO_MASTER         51
#define SPI0_MI_MASTER         52
#define SPI2W_DATA             53
#define SPI2W_CLK              54
#define SPI2W_CS               55
#define SWD_CLK                56
#define SWD_DIO                57
#define KEY_COL_0              58
#define KEY_COL_1              59
#define KEY_COL_2              60
#define KEY_COL_3              61
#define KEY_COL_4              62
#define KEY_COL_5              63
#define KEY_COL_6              64
#define KEY_COL_7              65
#define KEY_COL_8              66
#define KEY_COL_9              67
#define KEY_COL_10             68
#define KEY_COL_11             69
#define KEY_COL_12             70
#define KEY_COL_13             71
#define KEY_COL_14             72
#define KEY_COL_15             73
#define KEY_COL_16             74
#define KEY_COL_17             75
#define KEY_COL_18             76
#define KEY_COL_19             77
#define KEY_ROW_0              78
#define KEY_ROW_1              79
#define KEY_ROW_2              80
#define KEY_ROW_3              81
#define KEY_ROW_4              82
#define KEY_ROW_5              83
#define KEY_ROW_6              84
#define KEY_ROW_7              85
#define KEY_ROW_8              86
#define KEY_ROW_9              87
#define KEY_ROW_10             88
#define KEY_ROW_11             89
#define DWGPIO                 90
#define LRC_SPORT1             91
#define BCLK_SPORT1            92
#define ADCDAT_SPORT1          93
#define DACDAT_SPORT1          94
#define SPDIF_TX               95
#define DMIC1_CLK              96
#define DMIC1_DAT              97
#define LRC_I_CODEC_SLAVE      98
#define BCLK_I_CODEC_SLAVE     99
#define SDI_CODEC_SLAVE        100
#define SDO_CODEC_SLAVE        101
#define LRC_I_PCM              102
#define BCLK_I_PCM             103
#define UART2_CTS              104
#define UART2_RTS              105
#define BT_COEX_I_0            106
#define BT_COEX_I_1            107
#define BT_COEX_I_2            108
#define BT_COEX_I_3            109
#define BT_COEX_O_0            110
#define BT_COEX_O_1            111
#define BT_COEX_O_2            112
#define BT_COEX_O_3            113
#define PTA_I2C_CLK_SLAVE      114
#define PTA_I2C_DAT_SLAVE      115
#define PTA_I2C_INT_OUT        116
#define DSP_GPIO_OUT           117
#define DSP_JTCK               118
#define DSP_JTDI               119
#define DSP_JTDO               120
#define DSP_JTMS               121
#define DSP_JTRST              122
#define LRC_SPORT0             123
#define BCLK_SPORT0            124
#define ADCDAT_SPORT0          125
#define DACDAT_SPORT0          126
#define MCLK                   127
/**
  * @}
  */



/** @defgroup Internal Macros
  * @{
  */

#define  TOTAL_PIN_NUM             39

/* Pad Functions */
#define Output_Val                 (BIT0)
#define Output_En                  (BIT1)
#define Pull_En                    (BIT2)
#define Pull_Direction             (BIT3)
#define Pull_Resistance            (BIT4)
#define WakeUp_En                  (BIT5)
#define WKPOL                      (BIT6)

#define SHDN                       (BIT0)
#define Pin_Mode                   (BIT1)
#define Pin_Debounce               (BIT2)

/**
  * @}
  */

/**
  * @}
  */

/** @defgroup PINMUX_Exported_Types PINMUX Exported Types
  * @{
  */

/** @defgroup PAD_Pull_Mode PAD Pull Mode
  * @{
  */

typedef enum _PAD_Pull_Mode
{
    PAD_PULL_NONE,
    PAD_PULL_UP,
    PAD_PULL_DOWN
} PAD_Pull_Mode;


/**
  * @}
  */

/** @defgroup PAD_Mode PAD Mode
  * @{
  */

typedef enum _PAD_Mode
{
    PAD_SW_MODE,
    PAD_PINMUX_MODE
} PAD_Mode;

/**
  * @}
  */

/** @defgroup PAD_Power_Mode PAD Power Mode
  * @{
  */

typedef enum _PAD_PWR_Mode
{
    PAD_NOT_PWRON,
    PAD_IS_PWRON = 1
} PAD_PWR_Mode;

/**
  * @}
  */

/** @defgroup PAD_Outout_Config PAD Outout Config
  * @{
  */

typedef enum _PAD_OUTPUT_ENABLE_Mode
{
    PAD_OUT_DISABLE,
    PAD_OUT_ENABLE
} PAD_OUTPUT_ENABLE_Mode;

/**
  * @}
  */

/** @defgroup PAD_Outout_Value PAD Outout Value
  * @{
  */

typedef enum _PAD_OUTPUT_VAL
{
    PAD_OUT_LOW,
    PAD_OUT_HIGH
} PAD_OUTPUT_VAL;

/**
  * @}
  */

/** @defgroup PAD_WakeUp_Polarity_Value PAD WakeUp Polarity
  * @{
  */

typedef enum _PAD_WAKEUP_POL_VAL
{
    PAD_WAKEUP_POL_HIGH,
    PAD_WAKEUP_POL_LOW
} PAD_WAKEUP_POL_VAL;

/**
  * @}
  */

/** @defgroup PAD_WakeUp_Debounce_En PAD WakeUp Debounce enable
  * @{
  */

typedef enum _PAD_WAKEUP_DEBOUNCE_EN
{
    PAD_WK_DEBOUNCE_DISABLE,
    PAD_WK_DEBOUNCE_ENABLE
} PAD_WAKEUP_DEBOUNCE_EN;

/**
  * @}
  */

/** @defgroup PAD Driving Current control value
  * @{
  */

typedef enum _PAD_PULL_CONFIG_VAL
{
    PAD_WEAK_PULL,
    PAD_STRONG_PULL
} PAD_PULL_VAL;
/**
  * @}
  */

/** @defgroup PAD Driving Current control value
  * @{
  */

typedef enum _PAD_DRIVING_CURRENT_CONT
{
    DRIVING_CURRENT_DISABLE,
    DRIVING_CURRENT_E1,
    DRIVING_CURRENT_E2,
    DRIVING_CURRENT_E3,
    DRIVING_CURRENT_E4,
    DRIVING_CURRENT_E5
} PAD_DRIVING_CURRENT_VAL;

/**
  * @}
  */

/** @defgroup PAD Driving Current E2 value
  * @{
  */

typedef enum _PAD_DRIVING_CURRENT_E2_LEVEL
{
    LEVEL_E2_0,
    LEVEL_E2_1
} PAD_DRIVING_CURRENT_E2_LEVEL;

/**
  * @}
  */

/** @defgroup PAD Driving Current E3 value
  * @{
  */

typedef enum _PAD_DRIVING_CURRENT_E3_LEVEL
{
    LEVEL_E3_0,
    LEVEL_E3_1
} PAD_DRIVING_CURRENT_E3_LEVEL;

/**
  * @}
  */

/** @defgroup PAD Function config value
  * @{
  */

typedef enum _PAD_FUNCTION_CONFIG_VALUE
{
    AON_GPIO,
    LED0,
    LED1,
    LED2,
    CLK_REQ,
} PAD_FUNCTION_CONFIG_VAL;

/**
  * @}
  */

/** @defgroup PAD_WakeUp_Polarity_Value PAD WakeUp Polarity
  * @{
  */

typedef enum _PAD_LDO_TYPE
{
    PAD_LDOAUX1,
    PAD_LDOAUX2
} PAD_LDO_TYPE;

/**
  * @}
  */

/** End of group PINMUX_Exported_Types
  * @}
  */



/** @defgroup PINMUX_Exported_Functions PINMUX Exported Functions
  * @{
  */

extern void Pinmux_Reset(void);
extern void Pinmux_Deinit(uint8_t Pin_Num);
extern void Pinmux_Config(uint8_t Pin_Num, uint8_t Pin_Func);
extern void Pad_Config(uint8_t Pin_Num,
                       PAD_Mode AON_PAD_Mode,
                       PAD_PWR_Mode AON_PAD_PwrOn,
                       PAD_Pull_Mode AON_PAD_Pull,
                       PAD_OUTPUT_ENABLE_Mode AON_PAD_E,
                       PAD_OUTPUT_VAL AON_PAD_O);
extern void Pad_AllConfigDefault(void);
extern void System_WakeUpPinEnable(uint8_t Pin_Num, uint8_t Polarity, uint8_t DebounceEn);
extern void System_WakeUpPinDisable(uint8_t Pin_Num);
extern uint8_t System_WakeUpInterruptValue(uint8_t Pin_Num);
extern uint8_t Pad_WakeupInterruptValue(uint8_t Pin_Num);
extern void Pad_OutputEnableValue(uint8_t Pin_Num, uint8_t value);
extern void Pad_PullEnableValue(uint8_t Pin_Num, uint8_t value);
extern void Pad_PullUpOrDownValue(uint8_t Pin_Num, uint8_t value);
extern void Pad_PullConfigValue(uint8_t Pin_Num, uint8_t value);
extern void Pad_ControlSelectValue(uint8_t Pin_Num, uint8_t value);
extern void Pad_OutputControlValue(uint8_t Pin_Num, uint8_t value);
extern void Pad_WakeupEnableValue(uint8_t Pin_Num, uint8_t value);
/* PAD wakeup polarity. 0: high level. 1: low level */
extern void Pad_WakeupPolarityValue(uint8_t Pin_Num, uint8_t value);
extern void Pad_WKDebounceConfig(uint8_t Pin_Num, uint8_t value);
extern void Pad_ClearWakeupINTPendingBit(uint8_t Pin_Num);
extern void Pad_ClearAllWakeupINT(void);
extern void Pad_PowerOrShutDownValue(uint8_t Pin_Num, uint8_t value);
extern void Pad_PullConfigValue(uint8_t Pin_Num, uint8_t value);
extern void Pad_DrivingCurrentControl(uint8_t Pin_Num, uint8_t e2_value, uint8_t e3_value);
extern void Pad_FunctionConfig(uint8_t Pin_Num, PAD_FUNCTION_CONFIG_VAL value);
extern void System_WakeUpDebounceTime(uint8_t time);
/**
  * @}
  */

/**
* @}
*/

#ifdef __cplusplus
}
#endif

#endif /* _RTL876X_PINMUX_H_ */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2015 Realtek Semiconductor *****END OF FILE****/

