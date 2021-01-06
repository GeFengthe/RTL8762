/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of I2C master mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
#include <trace.h>
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_I2C.h"
#include "rtl876x_nvic.h"
#include "rtl876x_gpio.h"

#include "skydisplay.h"

#define SKYDISPLAY_PRINTF   DBG_DIRECT 

#define I2C0_SCL_PIN                P2_7        //C9: 0
#define I2C0_SDA_PIN                P2_6 // P5_0        //C9: 1

#define BL55072A_SLAVE_ADDR         0x7C    // DISPLAY
#define SHCT3_SLAVE_ADDR            0xE0    // SENOR  0X70<<1



/**
 * @brief    pinmux configuration
 * @return   void
 */
void PINMUX_Configuration(void)
{
    Pinmux_Config(I2C0_SCL_PIN, I2C0_CLK);
    Pinmux_Config(I2C0_SDA_PIN, I2C0_DAT);
    return;
}
/**
 * @brief    pad configuration
 * @return   void
 */
void PAD_Configuration(void)
{
    Pad_Config(I2C0_SCL_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(I2C0_SDA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    return;
}

/**
 * @brief    rcc configuration
 * @return   void
 */
void RCC_Configuration(void)
{
    /* Enable I2C clock */
    RCC_PeriphClockCmd(APBPeriph_I2C0, APBPeriph_I2C0_CLOCK, ENABLE);
    return;
}

/**
  * @brief  Initialize IO peripheral.
  * @param   No parameter.
  * @return  void
  */
void I2C0_Configuration(void)
{
    /* Initialize I2C */
    I2C_InitTypeDef  I2C_InitStructure;
    I2C_StructInit(&I2C_InitStructure);
    I2C_InitStructure.I2C_ClockSpeed   = 100000;
    I2C_InitStructure.I2C_DeviveMode   = I2C_DeviveMode_Master;
    I2C_InitStructure.I2C_AddressMode  = I2C_AddressMode_7BIT;
    I2C_InitStructure.I2C_SlaveAddress = (BL55072A_SLAVE_ADDR>>1);
    I2C_InitStructure.I2C_Ack          = I2C_Ack_Enable;
    I2C_Init(I2C0, &I2C_InitStructure);
    I2C_Cmd(I2C0, ENABLE);
}
/**
 * @brief    Contains the initialization of pinmux settings and pad settings
 * @note     All the pinmux settings and pad settings shall be initiated in this function,
 *           but if legacy driver is used, the initialization of pinmux setting and pad setting
 *           should be peformed with the IO initializing.
 * @return   void
 */
void Display_IIC_Init(void)
{
	RCC_Configuration();
	PAD_Configuration();
	PINMUX_Configuration();

	I2C0_Configuration();
}


//==========================================================================================================//
I2C_Status retstatu=I2C_ERR_TIMEOUT;
void BL55072A_Init(uint8_t* data, uint8_t len)
{
	uint8_t initdata[30]={0xFF, 0xC8, 0xEA, 0xBE, 0xE8, 0x00};
	if(len <= 18){
		memcpy( &initdata[6], data, len);	
		retstatu = I2C_MasterWrite(I2C0, initdata, 6+len);	
		retstatu=I2C_ERR_TIMEOUT;
	}
}

void BL55072A_DisplayOn(void)
{
	uint8_t senddata[4]={0xBE, 0xF0, 0xFC, 0xC8};

	retstatu = I2C_MasterWrite(I2C0, senddata, 4);	
	retstatu=I2C_ERR_TIMEOUT;
}

void BL55072A_DisplayOff(void)
{
	uint8_t senddata[1]={0xC0};

	retstatu = I2C_MasterWrite(I2C0, senddata, 1);	
	retstatu=I2C_ERR_TIMEOUT;
}

void BL55072A_WriteDisplay(uint8_t* data, uint8_t len)
{
	uint8_t senddata[30]={0xBE, 0xF0, 0xFC, 0xC8, 0xE8, 0x00};
	if(len <= 18){
		memcpy( &senddata[6], data, len);	
		retstatu = I2C_MasterWrite(I2C0, senddata, 6+len);	
		retstatu=I2C_ERR_TIMEOUT;
	}
}
//----------------------------------------------------------------------------------------------------------//
typedef union
{
    uint8_t byte;
    struct
    {
        uint8_t L: 4; // 
        uint8_t H: 4; //  
    } half;
    struct
    {
        uint32_t B0: 1; // 
        uint32_t B1: 1; // 
        uint32_t B2: 1; // 
        uint32_t B3: 1; // 
        uint32_t B4: 1; // 
        uint32_t B5: 1; // 
        uint32_t B6: 1; // 
        uint32_t B7: 1; //   
    } bit;
}UNION_BYTE2BIT_T;
typedef  struct
{
	char    chara;
	uint8_t val;	
}DisContent_t; 
#define TOTAL_DISCONTENT_NUM 13
const DisContent_t DisContent[TOTAL_DISCONTENT_NUM+1] = {
	{'0',0x3F}, {'1',0x06}, {'2',0x5B}, {'3',0x4F}, {'4',0x66}, {'5',0x6D},
	{'6',0x7D}, {'7',0x07}, {'8',0x7F}, {'9',0x6F}, {' ',0x00}, {'*',0x9F},
	{'-',0x40}, {' ',0x00} // when not find
}; // 码值可以调整
static uint8_t Find_Display_character(char chara, bool showdot)
{
	uint8_t i = 0;

	for(i=0; i<TOTAL_DISCONTENT_NUM; i++){
		if(chara == DisContent[i].chara){
			break;
		}
	}
	if(i == TOTAL_DISCONTENT_NUM){
		SKYDISPLAY_PRINTF("%s err! \n", __func__);
	}
	
	if(showdot == true){
		return DisContent[i].val |0x80 ;
	} else{
		return DisContent[i].val ;
	}
}
/*
	humidity 放大10倍传入
	temperature 放大10倍传入
	rssi 信号强度%
	battery 电池电量%	
*/
void SkyIot_Display(uint32_t humidity, int temperature, uint8_t rssi, uint8_t battery)
{
	UNION_BYTE2BIT_T disdata[18];
	UNION_BYTE2BIT_T data1, data2, tmp;
	uint8_t tmprssi=0, tmpbattery=0;
	
	data1.byte = 0;
	data2.byte = 0;
	
	tmprssi    = (rssi+12) / 25;
	tmpbattery = (battery+10) / 20;
	
	if(tmprssi > 4){
		data2.bit.B0 = 1;
	}
	data2.bit.B1 = 1;
	data2.bit.B2 = 1;
	if(tmpbattery>=5){
		data2.bit.B3 = 1;
		data2.bit.B4 = 1;
		data2.bit.B5 = 1;
		data2.bit.B6 = 1;
		data2.bit.B7 = 1;		
	}else if(tmpbattery==4){
		data2.bit.B4 = 1;
		data2.bit.B5 = 1;
		data2.bit.B6 = 1;
		data2.bit.B7 = 1;	
	}else if(tmpbattery==3){
		data2.bit.B5 = 1;
		data2.bit.B6 = 1;
		data2.bit.B7 = 1;	
	}else if(tmpbattery==2){
		data2.bit.B6 = 1;
		data2.bit.B7 = 1;	
	}else if(tmpbattery==1){
		data2.bit.B7 = 1;	
	}
	
	// 
	disdata[0].byte = Find_Display_character(humidity/100    , true);
	disdata[1].byte = Find_Display_character((humidity/10)%10, true);
	disdata[2].byte = Find_Display_character(humidity%10     , true); // 与上2个不一样
	
	if(tmprssi > 0){		
		data1.bit.B4 = 1;		
	}
	data1.bit.B6 = 1;
	data1.bit.B7 = 1;	
	
	if(temperature < 0){
		data1.bit.B5 = 1;
		temperature *= (-1);
	}
	
	//	
	disdata[3].byte = data1.byte;
	tmp.byte = Find_Display_character(temperature/100    , (tmprssi>=2));
	disdata[3].half.L = tmp.half.H;
	disdata[4].half.H = tmp.half.L;
	tmp.byte = Find_Display_character((temperature/10)%10, (tmprssi>=3));
	disdata[4].half.L = tmp.half.H;
	disdata[5].half.H = tmp.half.L;
	tmp.byte = Find_Display_character(temperature%10     , true); // 与上2个不一样
	disdata[5].half.L = tmp.half.H;	
	disdata[6].half.H = tmp.half.L;
		
	disdata[6].half.L = data2.half.H;	
	disdata[7].half.H = data2.half.L;
	
	memset((uint8_t*)disdata, 0xF0, 18);
	BL55072A_WriteDisplay((uint8_t*)disdata, 18);
}

//==========================================================================================================//

void displaymain(void)
{
   SkyIot_Display(123, 456, 100, 100);

}

//==========================================================================================================//
#define	SHTC3_SOFT_RESET_CMD	        0x805D//软件复位命令
#define	SHTC3_READ_ID_CMD	            0xEFC8//读取SHTC3 ID命令
#define	SHTC3_WAKE_UP_CMD	            0x3517//将芯片从睡眠模式唤醒命令
#define	SHTC3_SLEEP_CMD	                0xB098//使芯片进入休眠命令

//Normal Mode Clock Stretching Enable Measurement Command
#define	SHTC3_NOR_READ_TEMP_FIR_EN_CMD	0x7CA2//
#define	SHTC3_NOR_READ_HUM_FIR_EN_CMD	0x5C24//
//Normal Mode Clock Stretching Disable Measurement Command
#define	SHTC3_NOR_READ_TEMP_FIR_DIS_CMD	0x7866//
#define	SHTC3_NOR_READ_HUM_FIR_DIS_CMD	0x58E0//
//Low Power Mode Clock Stretching Enable Measurement Command
#define	SHTC3_LOW_READ_TEMP_FIR_EN_CMD	0x6458//
#define	SHTC3_LOW_READ_HUM_FIR_EN_CMD	0x44DE//
//Low Power Mode Clock Stretching Disable Measurement Command
#define	SHTC3_LOW_READ_TEMP_FIR_DIS_CMD	0x609C//
#define	SHTC3_LOW_READ_HUM_FIR_DIS_CMD	0x401A//

/*******************************************************************************
函数名称        ：SHTC3_CRC_CHECK
函数参数        ：校验参数和校验码
函数返回值      ：0：校验正确   1：校验错误
函数说明        ：CRC校验：
                    CRC多项式为：x^8+x^5+x^4+1，即0x131
*******************************************************************************/
uint8_t SHTC3_crc_Check(uint16_t data, uint8_t crcres)
{
    uint8_t i,t,temp;
    uint8_t crccal = 0xFF;  
	
    temp = (data>>8) & 0xFF;        
    for(t = 0;t < 2;t ++){
        crccal ^= temp;
        for(i = 0;i < 8;i ++){
            if(crccal & 0x80) {
                crccal <<= 1;                
                crccal ^= 0x31;    
            }else {
                crccal <<= 1;    
            }
        }        
        if(t == 0){
            temp = data & 0xFF; 
        }
    }
	
    temp = (crccal==crcres ? 0:1);
    
    return temp;
}

void SHTC3_Init(void)
{
	uint8_t initdata[2];
	uint8_t readid[2];
	
	initdata[0] = ((SHTC3_WAKE_UP_CMD>>8) & 0xFF);
	initdata[1] = (SHTC3_WAKE_UP_CMD & 0xFF);	
	I2C_MasterWrite(I2C0, initdata, 2);	
		
	initdata[0] = ((SHTC3_SOFT_RESET_CMD>>8) & 0xFF);
	initdata[1] = (SHTC3_SOFT_RESET_CMD & 0xFF);	
	I2C_MasterWrite(I2C0, initdata, 2);	
	
	initdata[0] = ((SHTC3_READ_ID_CMD>>8) & 0xFF);
	initdata[1] = (SHTC3_READ_ID_CMD & 0xFF);	
	I2C_RepeatRead(I2C0, initdata, 2, readid, 2);	
}

void SHTC3_Read_Temp_Hum(void)
{
	uint8_t setdata[2];
	uint8_t readdata[6];
	uint8_t crc_temp=0, crc_hum=0;
	uint16_t tmp_temp=0, tmp_hum=0;
	int res_temp=0;
	uint32_t res_hum=0;
	
	setdata[0] = ((SHTC3_WAKE_UP_CMD>>8) & 0xFF);
	setdata[1] = (SHTC3_WAKE_UP_CMD & 0xFF);	
	I2C_MasterWrite(I2C0, setdata, 2);
	
	setdata[0] = ((SHTC3_NOR_READ_TEMP_FIR_DIS_CMD>>8) & 0xFF);
	setdata[1] = (SHTC3_NOR_READ_TEMP_FIR_DIS_CMD & 0xFF);	
	I2C_MasterWrite(I2C0, setdata, 2);	
	
	
	I2C_MasterRead(I2C0, readdata, 6);	
	//SHTC3检测完成，开始读取数据，连CRC总共6个byte  
	tmp_temp = readdata[0];               // 温度数值高8位
	tmp_temp = (tmp_temp<<8)|readdata[1]; // 温度数值低8位
	crc_temp = readdata[2];               // 温度CRC校验
	
	tmp_hum = readdata[3];
	tmp_hum = (tmp_hum<<8)|readdata[4];
	crc_hum = readdata[5];
	
	if(SHTC3_crc_Check(tmp_temp, crc_temp) == 0){
		res_temp = tmp_temp*175*10; // 保留一位小数点*10
		res_temp = (res_temp >> 16);
		res_temp -= 450;
		
		if(res_temp >= 0){ // 正温度		
			SKYDISPLAY_PRINTF("sample temperatrue is:%d\n",res_temp); 	
		}else{
			SKYDISPLAY_PRINTF("sample temperatrue is:- %d\n",res_temp); 
		}	
	}
	
	if(SHTC3_crc_Check(tmp_hum, crc_hum) == 0){
		res_hum = tmp_hum*100;
		res_hum = (res_hum >> 16);
		
		SKYDISPLAY_PRINTF("sample humidity is:%d\n",res_hum);    	
	}
	
	setdata[0] = ((SHTC3_SLEEP_CMD>>8) & 0xFF);
	setdata[1] = (SHTC3_SLEEP_CMD & 0xFF);	
	I2C_MasterWrite(I2C0, setdata, 2);
}


