/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     
* @brief    This file provides demo code of I2C master mode.
* @details
* @author   
* @date     
* @version  v1.0
*********************************************************************************************************
*/
#include <trace.h>
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_I2C.h"
#include "rtl876x_nvic.h"
#include "rtl876x_gpio.h"

#include "os_sched.h"
#include "skyIICdevice.h"

#define SKYDISPLAY_PRINTF   DBG_DIRECT 

/*
本驱动、接口设计时未考虑互斥关系。单线程上操作OK。跨线程操作时需加互斥关系。
*/

#define I2C0_SCL_PIN                P2_7        // 
#define I2C0_SDA_PIN                P2_6        // 
// 7bit addr
#define BL55072A_SLAVE_ADDR         0x3E    // DISPLAY 0x7C>>1
#define SHCT3_SLAVE_ADDR            0x70    // SENOR  


/**
 * @brief    pinmux configuration
 * @return   void
 */
static void PINMUX_Configuration(void)
{
    Pinmux_Config(I2C0_SCL_PIN, I2C0_CLK);
    Pinmux_Config(I2C0_SDA_PIN, I2C0_DAT);
    return;
}
/**
 * @brief    pad configuration
 * @return   void
 */
static void PAD_Configuration(void)
{
    Pad_Config(I2C0_SCL_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(I2C0_SDA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    return;
}

/**
 * @brief    rcc configuration
 * @return   void
 */
static void RCC_Configuration(void)
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
static void I2C0_Configuration(void)
{
    /* Initialize I2C */
    I2C_InitTypeDef  I2C_InitStructure;
    I2C_StructInit(&I2C_InitStructure);
    I2C_InitStructure.I2C_ClockSpeed   = 100000;
    I2C_InitStructure.I2C_DeviveMode   = I2C_DeviveMode_Master;
    I2C_InitStructure.I2C_AddressMode  = I2C_AddressMode_7BIT;
    I2C_InitStructure.I2C_SlaveAddress = SHCT3_SLAVE_ADDR; // BL55072A_SLAVE_ADDR;
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
void RTL8762_IIC_Init(void)
{
	RCC_Configuration();
	PAD_Configuration();
	PINMUX_Configuration();

	I2C0_Configuration();
}

static void I2C0_Switch_SlaveAddr(uint8_t addr)
{
	I2C_SetSlaveAddress(I2C0, addr);
}

//==========================================================================================================//
void BL55072A_Init(uint8_t* data, uint8_t len)
{
	uint8_t initdata[30]={0xFF, 0xC8, 0xEA, 0xBE, 0xE8, 0x00};
	if(len <= 18){
		I2C0_Switch_SlaveAddr(BL55072A_SLAVE_ADDR);
		memset( &initdata[6], 0, 18);
		if(len){
			memcpy( &initdata[6], data, len);
			I2C_MasterWrite(I2C0, initdata, 6+len);
		}else{
			I2C_MasterWrite(I2C0, initdata, 24);
		}
	}
}

void BL55072A_DisplayOn(void)
{
	uint8_t senddata[4]={0xBE, 0xF0, 0xFC, 0xC8};

	I2C0_Switch_SlaveAddr(BL55072A_SLAVE_ADDR);
	I2C_MasterWrite(I2C0, senddata, 4);	
}

void BL55072A_DisplayOff(void)
{
	uint8_t senddata[1]={0xC0};

	I2C0_Switch_SlaveAddr(BL55072A_SLAVE_ADDR);
	I2C_MasterWrite(I2C0, senddata, 1);	
}

void BL55072A_WriteDisplay(uint8_t* data, uint8_t len)
{
	uint8_t senddata[30]={0xBE, 0xF0, 0xFC, 0xC8, 0xE8, 0x00};
	if(len <= 18){
		I2C0_Switch_SlaveAddr(BL55072A_SLAVE_ADDR);
		memset( &senddata[6], 0, 18);
		if(len){
			memcpy( &senddata[6], data, len);	
		}	
		I2C_MasterWrite(I2C0, senddata, 6+len);	
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
        uint8_t B0: 1; // 
        uint8_t B1: 1; // 
        uint8_t B2: 1; // 
        uint8_t B3: 1; // 
        uint8_t B4: 1; // 
        uint8_t B5: 1; // 
        uint8_t B6: 1; // 
        uint8_t B7: 1; //   
    } bit;
}UNION_BYTE2BIT_T;
typedef  struct
{
	char    chara;
	uint8_t val;	
}DisContent_t; 
#define TOTAL_DISCONTENT_NUM 12
const DisContent_t DisContent[TOTAL_DISCONTENT_NUM+1] = {
	{'0',0x5F}, {'1',0x06}, {'2',0x3D}, {'3',0x2F}, {'4',0x66}, {'5',0x6B},
	{'6',0x7B}, {'7',0x0E}, {'8',0x7F}, {'9',0x6F}, {' ',0x00}, {'*',0x9F},
	{' ',0x00} // when not find
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
void SkyIot_Lcd_Display(uint32_t humidity, int temperature, uint8_t rssi, uint8_t battery)
{
	UNION_BYTE2BIT_T disdata[18];
	UNION_BYTE2BIT_T data1, data2, tmp;
	uint8_t tmprssi=0, tmpbattery=0;
	
	data1.byte = 0;
	data2.byte = 0;
	
	tmprssi    = (rssi+12) / 25;
	tmpbattery = (battery+10) / 20;
	
	if(tmprssi > 4){
		data2.bit.B3 = 1; // T9
	}
	data2.bit.B1 = 1; // T6
	data2.bit.B2 = 1; // T5
	if(tmpbattery>=5){
		data2.bit.B0 = 1; // R5
		data2.bit.B4 = 1; // R4
		data2.bit.B5 = 1; // R3
		data2.bit.B6 = 1; // R2
		data2.bit.B7 = 1; // R1		
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
	disdata[0].byte = Find_Display_character(0x30+humidity/100    , true); // T12
	disdata[1].byte = Find_Display_character(0x30+(humidity/10)%10, true); // T2
	disdata[2].byte = Find_Display_character(0x30+humidity%10     , true); // 与上2个不一样	
	tmp.byte = disdata[2].byte;
	disdata[2].bit.B7 = tmp.bit.B6;
	disdata[2].bit.B6 = tmp.bit.B5;
	disdata[2].bit.B5 = tmp.bit.B4;
	disdata[2].bit.B4 = tmp.bit.B7; // T1
	
	if(tmprssi > 0){		
		data1.bit.B7 = 1; // T6		
	}
	data1.bit.B5 = 1;  // T11
	data1.bit.B4 = 1;  // T3
	
	if(temperature < 0){
		data1.bit.B6 = 1; // T10
		temperature *= (-1);
	}
	
	//	
	disdata[3].byte = data1.byte;
	tmp.byte = Find_Display_character(0x30+temperature/100    , (tmprssi>=2)); // T7
	disdata[3].half.L = tmp.half.H;
	disdata[4].half.H = tmp.half.L;
	tmp.byte = Find_Display_character(0x30+(temperature/10)%10, (tmprssi>=3)); // T8
	disdata[4].half.L = tmp.half.H;
	disdata[5].half.H = tmp.half.L;
	tmp.byte = Find_Display_character(0x30+temperature%10     , true); // 与上2个不一样
	// disdata[5].half.L = tmp.half.H;	
	disdata[5].bit.B3 = tmp.bit.B6;	
	disdata[5].bit.B2 = tmp.bit.B5;	
	disdata[5].bit.B1 = tmp.bit.B4;	
	disdata[5].bit.B0 = tmp.bit.B7; // T4	
	disdata[6].half.H = tmp.half.L;
		
	disdata[6].half.L = data2.half.H;	
	disdata[7].half.H = data2.half.L;
	
	
	BL55072A_WriteDisplay((uint8_t*)disdata, 18);
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
	uint8_t readid[2]={0,0};
	
	I2C0_Switch_SlaveAddr(SHCT3_SLAVE_ADDR);
	
	initdata[0] = ((SHTC3_WAKE_UP_CMD>>8) & 0xFF);
	initdata[1] = (SHTC3_WAKE_UP_CMD & 0xFF);	
	I2C_MasterWrite(I2C0, initdata, 2);	
	os_delay(5); //  唤醒后需延时
	
	initdata[0] = ((SHTC3_SOFT_RESET_CMD>>8) & 0xFF);
	initdata[1] = (SHTC3_SOFT_RESET_CMD & 0xFF);	
	I2C_MasterWrite(I2C0, initdata, 2);	
	os_delay(10); //  软复位后需延时
	
	initdata[0] = ((SHTC3_READ_ID_CMD>>8) & 0xFF);
	initdata[1] = (SHTC3_READ_ID_CMD & 0xFF);	
	I2C_RepeatRead(I2C0, initdata, 2, readid, 2);
	os_delay(5);	
}

uint8_t SHTC3_Read_Temp_Hum(int *gettemp, uint32_t *gethum)
{
	uint8_t setdata[2];
	uint8_t readdata[6]={0,0,0,0,0,0};
	uint8_t crc_temp=0, crc_hum=0;
	uint16_t tmp_temp=0, tmp_hum=0;
	
	int res_temp=0;
	uint32_t res_hum=0;
	I2C_Status tmpres = I2C_ERR_TIMEOUT;
	
	uint8_t ret = 0;
	uint8_t timoutcnt = 0;
	
	I2C0_Switch_SlaveAddr(SHCT3_SLAVE_ADDR);
	
	setdata[0] = ((SHTC3_WAKE_UP_CMD>>8) & 0xFF);
	setdata[1] = (SHTC3_WAKE_UP_CMD & 0xFF);	
	I2C_MasterWrite(I2C0, setdata, 2);
	os_delay(5); //  唤醒后需延时
	
	setdata[0] = ((SHTC3_NOR_READ_TEMP_FIR_DIS_CMD>>8) & 0xFF);
	setdata[1] = (SHTC3_NOR_READ_TEMP_FIR_DIS_CMD & 0xFF);	
	I2C_MasterWrite(I2C0, setdata, 2);	
	// os_delay(10);
	
	do{
		os_delay(7); //  启动采集后需延时,理论需要11~20ms
		tmpres = I2C_MasterRead(I2C0, readdata, 6);	
		if(++timoutcnt >= 10){
			ret = 2;
			goto EXIT_LAB;
		}
	}while(tmpres!=I2C_Success);
	
	//SHTC3检测完成，开始读取数据，连CRC总共6个byte  
	tmp_temp = readdata[0];               // 温度数值高8位
	tmp_temp = (tmp_temp<<8)|readdata[1]; // 温度数值低8位
	crc_temp = readdata[2];               // 温度CRC校验
	
	tmp_hum = readdata[3];
	tmp_hum = (tmp_hum<<8)|readdata[4];
	crc_hum = readdata[5];
	
	if(SHTC3_crc_Check(tmp_temp, crc_temp) == 0){
		res_temp = tmp_temp;
		res_temp = ((res_temp*175*10)>>16) - 450; // 保留一位小数点*10
		*gettemp = res_temp;
		if(res_temp >= 0){ // 正温度		
			SKYDISPLAY_PRINTF("sample temperatrue is:%d\n",res_temp); 	
		}else{
			SKYDISPLAY_PRINTF("sample temperatrue is:- %d\n",res_temp); 
		}	
	}else{
		ret = 1;
	}
	
	if(SHTC3_crc_Check(tmp_hum, crc_hum) == 0){
		res_hum = tmp_hum;
		res_hum = ((res_hum*1000) >> 16); // 保留一位小数点*10
		
		*gethum = res_hum;		
		SKYDISPLAY_PRINTF("sample humidity is:%d\n",res_hum);    	
	}else{
		ret = 1;
	}
	
EXIT_LAB:	
	setdata[0] = ((SHTC3_SLEEP_CMD>>8) & 0xFF);
	setdata[1] = (SHTC3_SLEEP_CMD & 0xFF);	
	I2C_MasterWrite(I2C0, setdata, 2);
	
	return ret;
}


//=================================================测试代码=========================================================//
// 初始化操作
/*
	RTL8762_IIC_Init();
	os_delay(5);
	memset(data, 0xF0, 18);
	BL55072A_Init( data, 0);
	os_delay(5);
	BL55072A_DisplayOn();
	os_delay(5);	
	SHTC3_Init();
*/

// 更新应用层调用
void displaymain(void)
{
	uint32_t humidity;
	int temperature;
	uint8_t rssi;
	uint8_t battery;
	
	static uint8_t tmp=0;
	if(++tmp >= 10){
		tmp = 0;
	}
//	humidity = 111*tmp;
//	if(tmp&0x01){	
//		temperature = -111*tmp;
//	}else{
//		temperature = 111*tmp;
//	}
	rssi = 10*tmp;
	battery = 10*tmp;
	
	
	SHTC3_Read_Temp_Hum(&temperature , &humidity);
	os_delay(5);

	SkyIot_Lcd_Display(humidity, temperature, rssi, battery);
	
}

void Single_blink(void)
{
    uint8_t senddata[30]={0xBE, 0xF3, 0xFC, 0xC8};      //0xF3 闪烁控制命令(500ms)
    senddata[4] = 0xFF;
    I2C0_Switch_SlaveAddr(BL55072A_SLAVE_ADDR);
    I2C_MasterWrite(I2C0,senddata,4+18);
}

void HAL_Sky_I2C_Dlps(bool allowenter)
{
    if(allowenter)
    {
        Pad_Config(I2C0_SCL_PIN,PAD_SW_MODE,PAD_IS_PWRON,PAD_PULL_UP,PAD_OUT_ENABLE,PAD_OUT_HIGH);
        Pad_Config(I2C0_SDA_PIN,PAD_SW_MODE,PAD_IS_PWRON,PAD_PULL_UP,PAD_OUT_ENABLE,PAD_OUT_HIGH);
    }else{
        Pad_Config(I2C0_SCL_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
        Pad_Config(I2C0_SDA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    }
}
