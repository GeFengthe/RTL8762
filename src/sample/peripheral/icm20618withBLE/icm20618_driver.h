/*
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     icm20618_driver.h
* @brief    icm20618 driver header file
* @details  none
* @author   yuan
* @date     2017-11-6
* @version  v0.0
*********************************************************************************************************
*/
#ifndef __ICM20618_DRIVER_H
#define __ICM20618_DRIVER_H
#include "rtl876x_i2c.h"

#define DIGITAL_INTERDACE     0x01
#define DIGITAL_INTERDACE_I2C 0x01
#define DIGITAL_INTERDACE_SPI 0x02

#define ICM20618_I2C_NUM     I2C0
#define ICM20618_I2C_SPEED   400000U
#define ICM20618_I2C_ADDR    0x68//b1101000 or b1101001
//ICM20618 ID
#define ICM20618_ID  0xA0

//ICM20618 User bank enable
#define USER_BANK_0_EN 1
#define USER_BANK_1_EN 1
#define USER_BANK_2_EN 1
#define USER_BANK_3_EN 0

//ICM20618 User bank define
#define USER_BANK_0  0x00
#define USER_BANK_1  0x01
#define USER_BANK_2  0x02
#define USER_BANK_3  0x03

/*ICM20618 Register map*/
#define REG_BANK_SEL 0x7F

#if USER_BANK_0_EN == 1
#define REG_WHO_AM_I     0x00
#define REG_USER_CTRL    0x03

#define REG_LP_CONFIG   0x05
#define REG_PWR_MGMT_1   0X06
#define REG_PWR_MGMT_2   0X07

#define REG_INT_PIN_CFG     0x0F
#define REG_INT_ENABLE      0x10
#define REG_INT_ENABLE_1    0x11
#define REG_INT_ENABLE_2    0x12
#define REG_INT_ENABLE_3    0x13

#define REG_INT_STATUS      0x19
#define REG_INT_STATUS_1    0x1A
#define REG_INT_STATUS_2    0x1B
#define REG_INT_STATUS_3    0x1C

#define REG_ACCEL_XOUT_H    0x2D
#define REG_ACCEL_XOUT_L    0x2E
#define REG_ACCEL_YOUT_H    0x2F
#define REG_ACCEL_YOUT_L    0x30
#define REG_ACCEL_ZOUT_H    0x31
#define REG_ACCEL_ZOUT_L    0x32

#define REG_GYRO_XOUT_H     0x33
#define REG_GYRO_XOUT_L     0x34
#define REG_GYRO_YOUT_H     0x35
#define REG_GYRO_YOUT_L     0x36
#define REG_GYRO_ZOUT_H     0x37
#define REG_GYRO_ZOUT_L     0x38

#define REG_TEMP_OUT_H      0x39
#define REG_TEMP_OUT_L      0x3A

#define REG_FIFO_EN_2       0x67
#define REG_FIFO_RST        0x68
#define REG_FIFO_MODE       0x69
#define REG_FIFO_COUNTH     0x70
#define REG_FIFO_COUNTL     0x71
#define REG_FIFO_R_W        0x72
#define REG_FIFO_CFG        0x76

#define REG_DATA_RDY_STATUS 0x74

#endif

#if USER_BANK_1_EN==1
#define REG_SELF_TEST_X_GYRO     0x02
#define REG_SELF_TEST_Y_GYRO     0x03
#define REG_SELF_TEST_Z_GYRO     0x04

#define REG_SELF_TEST_X_ACCEL    0X0E
#define REG_SELF_TEST_Y_ACCEL    0X0F
#define REG_SELF_TEST_Z_ACCEL    0x10

#define REG_XA_OFFS_H            0x14
#define REG_XA_OFFS_L            0x15
#define REG_YA_OFFS_H            0x17
#define REG_YA_OFFS_L            0x18
#define REG_ZA_OFFS_H            0x1A
#define REG_ZA_OFFS_L            0x1B
#define REG_TIMEBASE_CORRECTION_PLL    0x28

#endif

#if USER_BANK_2_EN==1
#define REG_GYRO_SMPLRT_DIV     0x00
#define REG_GYRO_CONFIG_1       0x01
#define REG_GYRO_CONFIG_2       0x02

#define REG_XG_OFFS_USRH        0X03
#define REG_XG_OFFS_USRL        0X04
#define REG_YG_OFFS_USRH        0x05
#define REG_YG_OFFS_USRL        0x06
#define REG_ZG_OFFS_USRH        0x07
#define REG_ZG_OFFS_USRL        0x08

#define REG_ACCEL_SMPLRT_DIV_1  0x10
#define REG_ACCEL_SMPLRT_DIV_2  0x11
#define REG_ACCEL_INTEL_CTRL    0x12
#define REG_ACCEL_WOM_THR       0x13
#define REG_ACCEL_CONFIG        0x14
#define REG_ACCEL_CONFIG_2      0x15

#define REG_FSYNC_CONFIG        0x52
#define REG_TEMP_CONFIG         0x53
#define REG_MOD_CTRL_USR        0x54

#endif

#if USER_BANK_3_EN==1

#endif

#define ICM20618_DMP_ENABLE            0x80
#define ICM20618_FIFO_ENABLE           0x40
#define ICM20618_I2C_MST_ENABLE        0x20
#define ICM20618_I2C_IF_DISABLE        0x10
#define ICM20618_DMP_RESET             0x08
#define ICM20618_SRAM_RESET            0x04
//#define ICM20618_I2C_MST_RESET         0x02

//#define ICM20618_I2C_MST_CYCLE_MODE    0x40
#define ICM20618_ACCEL_MODE_CYCLE       0x20
#define ICM20618_ACCEL_MODE_NORMAL      0x00
#define ICM20618_GYRO_MODE_CYCLE        0x10
#define ICM20618_GYRO_MODE_NORMAL       0x00

#define ICM20618_DEVICE_RESET          0x80
#define ICM20618_GOTO_SLEEP            0x40
//#define ICM20618_WAKK_UP               0x00
#define ICM20618_LP_ENABLE             0x20
#define ICM20618_TEMP_DISABLE          0x08
//#define ICM20618_CLK_SOURCE            0x01

//#define ICM20618_ACCEL_ENABLE          0x00
#define ICM20618_ACCEL_DISABLE         0x38
//#define ICM20618_GYRO_ENABLE           0x00
#define ICM20618_GYRO_DISABLE          0x07

#define ICM20618_INT_PIN_LEVEL_LOW              0x80
#define ICM20618_INT_PIN_LEVEL_HIGH             0x00
#define ICM20618_INT_PIN_OUT_MODE_OD            0x40
#define ICM20618_INT_PIN_OUT_MODE_PP            0x00
#define ICM20618_INT_PIN_LATCH_MODE_UTIL        0x20
#define ICM20618_INT_PIN_LATCH_MODE_50US        0x00
#define ICM20618_INT_PIN_CLEAR_READ_ANY         0x10
#define ICM20618_INT_PIN_CLEAR_READ_REG         0x00
//#define ICM20618_FSYNC_PIN_INT_LEVEL   0x08
//#define ICM20618_FSYNC_PIN_INT_MODE    0x04
//#define ICM20618_I2C_MASTER_BYPASS_EN  0x02

//#define ICM20618_INT_REG_WOF           0x80
#define ICM20618_INT_WOM               0x08
#define ICM20618_INT_PLL_RDY           0x04
#define ICM20618_INT_DMP               0x02
//#define ICM20618_INT_I2C_MST           0x01
#define ICM20618_INT_RAW_DATA_0_RDY    0x11
#define ICM20618_INT_FIFO_OVERFLOW     0x1F
#define ICM20618_INT_FIFO_WM           0x3F

//#define ICM20618_GET_INT(INT)    (((INT) == ICM20618_INT_REG_WOF) || ((INT) == ICM20618_INT_WOF) || \
//                             ((INT) == ICM20618_INT_PLL_RDY) || ((INT) == ICM20618_INT_DMP) || \
//                             ((INT) == ICM20618_INT_I2C_MST) || ((INT) == ICM20618_INT_RAW_DATA_0_RDY) || \
//                             ((INT) == ICM20618_INT_FIFO_OVERFLOW) || ((INT) == ICM20618_INT_FIFO_WM))

#define ICM20618_FIFO_RESET       0x1F

#define ICM20618_FIFO_ACCEL_EN    0x10
#define ICM20618_FIFO_GYRO_X_EN   0x08
#define ICM20618_FIFO_GYRO_Y_EN   0x04
#define ICM20618_FIFO_GYRO_Z_EN   0x02
//#define ICM20618_FIFO_TEMP_EN   0x01

#define ICM20618_FIFO_MODE_STREAM 0x00
#define ICM20618_FIFO_MODE_SNAP   0x01

typedef enum
{
    MEMS_ACCEL_X,
    MEMS_ACCEL_Y,
    MEMS_ACCEL_Z,
    MEMS_GYRO_X,
    MEMS_GYRO_Y,
    MEMS_GYRO_Z,
    MEMS_TEMP
} MEMS_Type_TypeDef;

//typedef struct
//{
//    uint16_t accel_xout;
//    uint16_t accel_yout;
//    uint16_t accel_zout;
//    uint16_t gyro_xout;
//    uint16_t gyro_yout;
//    uint16_t gyro_zout;
//    uint16_t temp_out;
//
//}MEMS_Data_TypeDef;

#define ACCEL_INTEL_EN         0x02
#define ACCEL_INTEL_MODE_INT   0x01
#define ACCEL_MOM_THRESHOLD_0MG      0x00
#define ACCEL_MOM_THRESHOLD_4MG      0x01
#define ACCEL_MOM_THRESHOLD_8MG      0x02
#define ACCEL_MOM_THRESHOLD_12MG     0x03
#define ACCEL_MOM_THRESHOLD_16MG     0x04
#define ACCEL_MOM_THRESHOLD_20MG     0x05
#define ACCEL_MOM_THRESHOLD_1020MG   0xFF

#define ACCEL_ODR_0_27         0x0FFF
#define ACCEL_ODR_0_55         0x07FC
#define ACCEL_ODR_1_1          0x03FE
#define ACCEL_ODR_2_19         0x0201
#define ACCEL_ODR_4_4          0x00FF
#define ACCEL_ODR_8_79         0x007F
#define ACCEL_ODR_17_6         0x003F
#define ACCEL_ODR_35_2         0x001F
#define ACCEL_ODR_70_3         0x000F
#define ACCEL_ODR_102          0x000A
#define ACCEL_ODR_141          0x0007
#define ACCEL_ODR_281          0x0003
#define ACCEL_ODR_563          0x0001

#define ACCEL_3DBBW_246         0x00
#define ACCEL_3DBBW_246_0       0x08
#define ACCEL_3DBBW_114_6       0x10
#define ACCEL_3DBBW_50_4        0x18
#define ACCEL_3DBBW_23_9        0x20
#define ACCEL_3DBBW_11_5        0x28
#define ACCEL_3DBBW_5_7         0x30
#define ACCEL_3DBBW_473         0x38

#define ACCEL_FULL_SCALE_2G     0x00
#define ACCEL_FULL_SCALE_4G     0x02
#define ACCEL_FULL_SCALE_8G     0x04
#define ACCEL_FULL_SCALE_16G    0x06

#define ACCEL_AVERAGE_1x        0x00
#define ACCEL_AVERAGE_4x        0x00
#define ACCEL_AVERAGE_8x        0x01
#define ACCEL_AVERAGE_16x       0x02
#define ACCEL_AVERAGE_32x       0x03

#define GYRO_ODR_4_4          0xFF
#define GYRO_ODR_17_3         0x40
#define GYRO_ODR_17_6         0x3F
#define GYRO_ODR_35_2         0x1F
#define GYRO_ODR_66_2         0x10
#define GYRO_ODR_70_3         0x0F
#define GYRO_ODR_102_3        0x0A
#define GYRO_ODR_140_6        0x07
#define GYRO_ODR_187_5        0x05
#define GYRO_ODR_225          0x04
#define GYRO_ODR_281          0x03
#define GYRO_ODR_275          0x02
#define GYRO_ODR_563          0x01

#define GYRO_3DBBW_196_6       0x00
#define GYRO_3DBBW_151_8       0x08
#define GYRO_3DBBW_119_5       0x10
#define GYRO_3DBBW_51_2        0x18
#define GYRO_3DBBW_23_9        0x20
#define GYRO_3DBBW_11_6        0x28
#define GYRO_3DBBW_5_7         0x30
#define GYRO_3DBBW_361_4       0x38

#define GYRO_FULL_SCALE_250    0x00
#define GYRO_FULL_SCALE_500    0x02
#define GYRO_FULL_SCALE_1000   0x04
#define GYRO_FULL_SCALE_2000   0x06

#define GYRO_AVERAGE_1x        0x00
#define GYRO_AVERAGE_2x        0x01
#define GYRO_AVERAGE_4x        0x02
#define GYRO_AVERAGE_8x        0x03
#define GYRO_AVERAGE_16x       0x04
#define GYRO_AVERAGE_32x       0x05
#define GYRO_AVERAGE_64x       0x06
#define GYRO_AVERAGE_128x      0x07

#define ICM20618_Device_Reset() ICM20618_Write_Single_Reg(REG_PWR_MGMT_1,ICM20618_DEVICE_RESET);
#define ICM20618_Analog_WakeUp() ICM20618_Config_Single_Reg(REG_PWR_MGMT_1,ICM20618_GOTO_SLEEP,DISABLE);
#define ICM20618_DMP_Config(status) ICM20618_Config_Single_Reg(REG_USER_CTRL,ICM20618_DMP_ENABLE,status);
#define ICM20618_Temp_Disable() ICM20618_Config_Single_Reg(REG_PWR_MGMT_1,ICM20618_TEMP_DISABLE,ENABLE);
#define ICM20618_Temp_LPMode_Config(vSample_Rate) {ICM20618_User_Bank_Config(USER_BANK_2);\
        ICM20618_Write_Single_Reg(REG_TEMP_CONFIG,(vSample_Rate));\
        ICM20618_User_Bank_Config(USER_BANK_0);}

uint8_t ICM20618_Init(void);
uint8_t ICM20618_Get_INT_Status(uint8_t ICM20618_INT);
float ICM20618_MEMS_Data_Transform(MEMS_Type_TypeDef vMEMS_Type, int vMEMS_Data);
int16_t ICM20618_Get_MEMS_Data(MEMS_Type_TypeDef vMEMS_Type);
void ICM20618_GYRO_Read(int16_t *GYRO_DATA);
void ICM20618_Acc_Read(int16_t *ACC_DATA);
//void ICM20618_Get_MEMS_Data(MEMS_Data_TypeDef *pMEMS_Data);
uint16_t ICM20618_Get_FIFO_Data(uint8_t *pFIFO_Buffer);

void ICM20618_FIFO_Config(uint8_t vFIFO_Source, uint8_t vFIFO_Mode);
void ICM20618_ACCEL_LPMode_Config(uint16_t vSample_Rate, uint8_t v3dB_BW, uint8_t vFull_Scale,
                                  uint8_t vAveraging);
void ICM20618_GYRO_LPMode_Config(uint8_t vSample_Rate, uint8_t v3dB_BW, uint8_t vFull_Scale,
                                 uint8_t vAveraging);

void ICM20618_INT_Config(uint8_t vINT_Type, FunctionalState NewState);
uint8_t ICM20618_User_Bank_Config(uint8_t vBank_Num);
void ICM20618_Write_Single_Reg(uint8_t vReg_Addr, uint8_t vReg_Value);
uint8_t ICM20618_Read_Single_Reg(uint8_t vReg_Addr);
void ICM20618_Config_Single_Reg(uint8_t vReg_Addr, uint8_t vReg_Value, FunctionalState NewState);
void ICM20618_Send_Data(uint8_t *vReg_Addr, uint8_t *pData, uint16_t vLenth);
bool ICM20618_Receive_Data(uint8_t *vReg_Addr, uint8_t *pData, uint16_t vLenth);

#endif

