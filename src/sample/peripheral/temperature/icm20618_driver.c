#include "icm20618_driver.h"
#include <trace.h>
uint8_t ACCEL_Full_Scale;
uint8_t GYRO_Full_Scale;

uint8_t ICM20618_Init(void)
{

    uint8_t device_id;
    /*config bank 0*/
    ICM20618_User_Bank_Config(USER_BANK_0);
    /*software reset is required using the PWR_MGMT_1 register, prior to initialization*/
    ICM20618_Device_Reset();//Reset
    /*wake up all anolog from sleep mode*/
    ICM20618_Analog_WakeUp();

    device_id = ICM20618_Read_Single_Reg(REG_WHO_AM_I);

    if (ICM20618_ID != device_id)
    {
        return 0;
    }

//      /*ACCEL config*/
//      ICM20618_ACCEL_LPMode_Config(ACCEL_ODR_1_1,ACCEL_3DBBW_114_6,ACCEL_FULL_SCALE_2G,ACCEL_AVERAGE_32x);
//
//      /*GYRO config*/
//    ICM20618_GYRO_LPMode_Config(GYRO_ODR_70_3,GYRO_3DBBW_196_6,GYRO_FULL_SCALE_2000,GYRO_AVERAGE_1x);
//
//      /*Temp config*/
//      ICM20618_Temp_Disable();
    ICM20618_Temp_LPMode_Config(0x01);

    /*DMP config*/
//      ICM20618_DMP_Config(DISABLE);


    /*FIFO config*/
//    ICM20618_FIFO_Config(ICM20618_FIFO_GYRO_X_EN,ICM20618_FIFO_MODE_STREAM);

    /*INT config*/
//    reg_value=ICM20618_INT_PIN_LEVEL_LOW|ICM20618_INT_PIN_OUT_MODE_OD|ICM20618_INT_PIN_LATCH_MODE_UTIL|ICM20618_INT_PIN_CLEAR_READ_REG;
//    ICM20618_Write_Single_Reg(REG_INT_PIN_CFG,reg_value);

//    reg_value=ICM20618_INT_WOM|ICM20618_INT_PLL_RDY|ICM20618_INT_DMP;
//    ICM20618_INT_Config(reg_value,ENABLE);

//    reg_value=ICM20618_INT_RAW_DATA_0_RDY;
//    ICM20618_INT_Config(reg_value,ENABLE);
//
//    reg_value=ICM20618_INT_FIFO_OVERFLOW;
//    ICM20618_INT_Config(reg_value,ENABLE);
//
//    reg_value=ICM20618_INT_FIFO_WM;
//    ICM20618_INT_Config(reg_value,ENABLE);

    return 1;
}

uint8_t ICM20618_Get_INT_Status(uint8_t vINT_Type)
{
    uint8_t int_status;
    uint8_t icm20618_reg;

    assert_param(ICM20618_GET_INT(ICM20618_INT));

    switch (vINT_Type)
    {
//        case 0x01:
    case 0x02:
    case 0x04:
    case 0x08:
        icm20618_reg = REG_INT_STATUS;
        ICM20618_Receive_Data(&icm20618_reg, &int_status, 1);
        int_status &= vINT_Type;
        break;
    case 0x11:
        icm20618_reg = REG_INT_STATUS_1;
        ICM20618_Receive_Data(&icm20618_reg, &int_status, 1);
        int_status &= (vINT_Type & 0x0F);
        break;
    case 0x1F:
        icm20618_reg = REG_INT_STATUS_2;
        ICM20618_Receive_Data(&icm20618_reg, &int_status, 1);
        int_status &= (vINT_Type & 0x1F);
        break;
    case 0x3F:
        icm20618_reg = REG_INT_STATUS_3;
        ICM20618_Receive_Data(&icm20618_reg, &int_status, 1);
        int_status &= (vINT_Type & 0x1F);
        break;
    default:
        int_status = 0xFF;
        break;

    }

    return int_status;

}

float ICM20618_MEMS_Data_Transform(MEMS_Type_TypeDef vMEMS_Type, int vMEMS_Data)
{
    float mems_value;
    float sensitivity = 0;

    switch (vMEMS_Type)
    {
    case MEMS_ACCEL_X:
    case MEMS_ACCEL_Y:
    case MEMS_ACCEL_Z:
        switch (ACCEL_Full_Scale)
        {
        case ACCEL_FULL_SCALE_2G:
            sensitivity = 16384.0;
            break;
        case ACCEL_FULL_SCALE_4G:
            sensitivity = 8192.0;
            break;
        case ACCEL_FULL_SCALE_8G:
            sensitivity = 4096.0;
            break;
        case ACCEL_FULL_SCALE_16G:
            sensitivity = 2048.0;
            break;
        default:
            break;
        }
        mems_value = (float)vMEMS_Data / sensitivity;
        break;

    case MEMS_GYRO_X:
    case MEMS_GYRO_Y:
    case MEMS_GYRO_Z:
        switch (GYRO_Full_Scale)
        {
        case GYRO_FULL_SCALE_250:
            sensitivity = 131.0;
            break;
        case GYRO_FULL_SCALE_500:
            sensitivity = 65.5;
            break;
        case GYRO_FULL_SCALE_1000:
            sensitivity = 32.8;
            break;
        case GYRO_FULL_SCALE_2000:
            sensitivity = 16.4;
            break;
        default:
            break;
        }
        mems_value = (float)vMEMS_Data / sensitivity;
        break;

    case MEMS_TEMP:
        mems_value = (float)vMEMS_Data / 333.87f + 21.0f;
        break;

    default:
        return 0;
//            break;

    }
    return mems_value;

}

int16_t ICM20618_Get_MEMS_Data(MEMS_Type_TypeDef vMEMS_Type)
{
    uint8_t mems_data[2];
    uint8_t reg_addr;
    int16_t mems_value;

    switch (vMEMS_Type)
    {
    case MEMS_ACCEL_X:
        reg_addr = REG_ACCEL_XOUT_H;
        break;

    case MEMS_ACCEL_Y:
        reg_addr = REG_ACCEL_YOUT_H;
        break;

    case MEMS_ACCEL_Z:
        reg_addr = REG_ACCEL_ZOUT_H;
        break;

    case MEMS_GYRO_X:
        reg_addr = REG_GYRO_XOUT_H;
        break;

    case MEMS_GYRO_Y:
        reg_addr = REG_GYRO_YOUT_H;
        break;

    case MEMS_GYRO_Z:
        reg_addr = REG_GYRO_ZOUT_H;
        break;

    case MEMS_TEMP:
        reg_addr = REG_TEMP_OUT_H;
        break;


    default:
        return 0;

    }

    while (ICM20618_Receive_Data(&reg_addr, mems_data, 2) == false)
    {
        mems_data[0] = 0;
        mems_data[1] = 0;
    }
    mems_value = (((int16_t)mems_data[0]) << 8) | mems_data[1];
    return mems_value;
}
void ICM20618_Acc_Read(int16_t *ACC_DATA)
{
    /*read Acc data*/

    ACC_DATA[0] = ICM20618_Get_MEMS_Data(MEMS_ACCEL_X);
    ACC_DATA[1] = ICM20618_Get_MEMS_Data(MEMS_ACCEL_Y);
    ACC_DATA[2] = ICM20618_Get_MEMS_Data(MEMS_ACCEL_Z);
}

void ICM20618_GYRO_Read(int16_t *GYRO_DATA)
{
    /*read Gyro data*/
    GYRO_DATA[0] = ICM20618_Get_MEMS_Data(MEMS_GYRO_X);         //
    GYRO_DATA[1] = ICM20618_Get_MEMS_Data(MEMS_GYRO_Y);
    GYRO_DATA[2] = ICM20618_Get_MEMS_Data(MEMS_GYRO_Z);
}
uint16_t ICM20618_Get_FIFO_Data(uint8_t *pFIFO_Buffer)
{
    uint8_t fifo_cnt[2];
    uint8_t reg_addr;
    uint16_t fifo_count;
    reg_addr = REG_FIFO_COUNTH;
    ICM20618_Receive_Data(&reg_addr, fifo_cnt, 2);
    fifo_count = (((uint16_t)fifo_cnt[0]) << 8) | fifo_cnt[1];

    reg_addr = REG_FIFO_R_W;
    ICM20618_Receive_Data(&reg_addr, pFIFO_Buffer, fifo_count);

    return fifo_count;
}

void ICM20618_ACCEL_LPMode_Config(uint16_t vSample_Rate, uint8_t v3dB_BW, uint8_t vFull_Scale,
                                  uint8_t vAveraging)
{
    uint8_t reg_value[2];
    ACCEL_Full_Scale = vFull_Scale;
    ICM20618_User_Bank_Config(USER_BANK_2);
    reg_value[0] = REG_ACCEL_SMPLRT_DIV_1;
    reg_value[1] = (uint8_t)(vSample_Rate >> 8);
    ICM20618_Send_Data(&reg_value[0], reg_value, 2);
    reg_value[0] = REG_ACCEL_SMPLRT_DIV_2;
    reg_value[1] = (uint8_t)(vSample_Rate);
    ICM20618_Send_Data(&reg_value[0], reg_value, 2);

    reg_value[0] = REG_ACCEL_CONFIG;
    reg_value[1] = v3dB_BW | vFull_Scale | 0x01;
    ICM20618_Send_Data(&reg_value[0], reg_value, 2);
    reg_value[0] = REG_ACCEL_CONFIG_2;
    reg_value[1] = vAveraging & 0x03;
    ICM20618_Send_Data(&reg_value[0], reg_value, 2);

    ICM20618_User_Bank_Config(USER_BANK_0);
//    ICM20618_Config_Single_Reg(REG_LP_CONFIG,ICM20618_ACCEL_MODE_CYCLE,ENABLE);
    ICM20618_Config_Single_Reg(REG_PWR_MGMT_1, ICM20618_LP_ENABLE, DISABLE);

}

void ICM20618_GYRO_LPMode_Config(uint8_t vSample_Rate, uint8_t v3dB_BW, uint8_t vFull_Scale,
                                 uint8_t vAveraging)
{
    uint8_t reg_value[2];

    GYRO_Full_Scale = vFull_Scale;

    ICM20618_User_Bank_Config(USER_BANK_2);

    reg_value[0] = REG_GYRO_SMPLRT_DIV;
//    reg_value[1]=(uint8_t)(1125.0/vSample_Rate - 1);//125Hz
    reg_value[1] = vSample_Rate;
    ICM20618_Send_Data(&reg_value[0], reg_value, 2);
    reg_value[0] = REG_GYRO_CONFIG_1;
    reg_value[1] = v3dB_BW | vFull_Scale | 0x01;
    ICM20618_Send_Data(&reg_value[0], reg_value, 2);
    reg_value[0] = REG_GYRO_CONFIG_2;
    reg_value[1] = vAveraging & 0x07;
    ICM20618_Send_Data(&reg_value[0], reg_value, 2);

    ICM20618_User_Bank_Config(USER_BANK_0);
//    ICM20618_Config_Single_Reg(REG_LP_CONFIG,ICM20618_GYRO_MODE_CYCLE,ENABLE);
    ICM20618_Config_Single_Reg(REG_PWR_MGMT_1, ICM20618_LP_ENABLE, DISABLE);
}

void ICM20618_INT_Config(uint8_t vINT_Type, FunctionalState NewState)
{
    uint8_t reg_addr, reg_value;

//    /* Check the parameters */
//    assert_param(ICM20618_GET_INT(ICM20618_INT));
//    assert_param(IS_FUNCTIONAL_STATE(NewState));

    ICM20618_Write_Single_Reg(REG_FIFO_CFG, 0x01);

    if (vINT_Type < 0x10)
    {
        reg_addr = REG_INT_ENABLE;
        reg_value = vINT_Type;
    }
    else
    {
        switch (vINT_Type)
        {

        case 0x11:
            reg_addr = REG_INT_ENABLE_1;
            reg_value = vINT_Type & 0x0F;
            break;
        case 0x1F:
            reg_addr = REG_INT_ENABLE_2;
            reg_value = vINT_Type & 0x1F;
            break;
        case 0x3F:
            reg_addr = REG_INT_ENABLE_3;
            reg_value = vINT_Type & 0x1F;
            break;
        default:
            break;

        }

    }
    ICM20618_Config_Single_Reg(reg_addr, reg_value, NewState);

    if ((vINT_Type & ICM20618_INT_WOM) == ICM20618_INT_WOM)
    {
        ICM20618_User_Bank_Config(USER_BANK_2);
        ICM20618_Write_Single_Reg(REG_ACCEL_INTEL_CTRL, ACCEL_INTEL_EN | ACCEL_INTEL_MODE_INT);
        ICM20618_Write_Single_Reg(REG_ACCEL_WOM_THR, ACCEL_MOM_THRESHOLD_4MG);
        ICM20618_User_Bank_Config(USER_BANK_0);
    }

}

void ICM20618_FIFO_Config(uint8_t vFIFO_Source, uint8_t vFIFO_Mode)
{

    ICM20618_Config_Single_Reg(REG_USER_CTRL, ICM20618_FIFO_ENABLE, ENABLE);

    ICM20618_Write_Single_Reg(REG_FIFO_RST, ICM20618_FIFO_RESET);

    ICM20618_Write_Single_Reg(REG_FIFO_RST, 0x00);

    ICM20618_Write_Single_Reg(REG_FIFO_EN_2, vFIFO_Source);

    ICM20618_Write_Single_Reg(REG_FIFO_MODE, vFIFO_Mode);

}

uint8_t ICM20618_User_Bank_Config(uint8_t vBank_Num)
{
    uint8_t reg_value[2];

    if ((vBank_Num > 3)) { return 0; }

    reg_value[0] = REG_BANK_SEL;
    reg_value[1] = (vBank_Num << 4) & 0xFF;

    ICM20618_Send_Data(&reg_value[0], reg_value, 2);

    return 1;
}

uint8_t ICM20618_Read_Single_Reg(uint8_t vReg_Addr)
{
    uint8_t reg_addr, reg_value = 0;
    reg_addr = vReg_Addr;

    ICM20618_Receive_Data(&reg_addr, &reg_value, 1);

    return reg_value;

}

void ICM20618_Write_Single_Reg(uint8_t vReg_Addr, uint8_t vReg_Value)
{
    uint8_t config_data[2];
    config_data[0] = vReg_Addr;
    config_data[1] = vReg_Value;

    ICM20618_Send_Data(&config_data[0], config_data, 2);

}

void ICM20618_Config_Single_Reg(uint8_t vReg_Addr, uint8_t vReg_Value, FunctionalState NewState)
{
    uint8_t reg_value[2];
    uint8_t get_reg_value;
    reg_value[0] = vReg_Addr;
    ICM20618_Receive_Data(&reg_value[0], &get_reg_value, 1);

    if (NewState != DISABLE)
    {
        reg_value[1] = get_reg_value | vReg_Value;
    }
    else
    {
        reg_value[1] = get_reg_value & (~vReg_Value);
    }

    ICM20618_Send_Data(&reg_value[0], reg_value, 2);

}



void ICM20618_Send_Data(uint8_t *vReg_Addr, uint8_t *pData, uint16_t vLenth)
{

    I2C_MasterWrite(ICM20618_I2C_NUM, pData, vLenth);
}

bool ICM20618_Receive_Data(uint8_t *vReg_Addr, uint8_t *pData, uint16_t vLenth)
{

    if (I2C_Success == I2C_RepeatRead(ICM20618_I2C_NUM, vReg_Addr, 1, pData, vLenth))
    {
        return true;
    }
    else
    {
        return false;
    }
}

