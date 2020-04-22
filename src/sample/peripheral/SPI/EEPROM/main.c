/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of IPS6404L-SQ SPI/QPI PSRAM mode.
* @details
* @author   lance
* @date     2018-02-05
* @version  v1.0
*********************************************************************************************************
*/
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_spi.h"
#include "trace.h"
#include "board.h"
#include "rtl876x_wdg.h"

/**
 * @brief    pinmux configuration
 * @return   void
 */
void PINMUX_Configuration(void)
{

    Pinmux_Config(SPI0_SCK, SPI0_CLK_MASTER);
    Pinmux_Config(SPI0_MOSI, SPI0_MO_MASTER);
    Pinmux_Config(SPI0_MISO, SPI0_MI_MASTER);
    Pinmux_Config(SPI0_CS, SPI0_SS_N_0_MASTER);
    return;
}
/**
 * @brief    pad configuration
 * @return   void
 */
void PAD_Configuration(void)
{

    Pad_Config(SPI0_SCK, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MOSI, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_MISO, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(SPI0_CS, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    return;
}

/**
 * @brief    rcc configuration
 * @return   void
 */
void RCC_Configuration(void)
{
    /* Enable SPI clock */
    RCC_PeriphClockCmd(APBPeriph_SPI0, APBPeriph_SPI0_CLOCK, ENABLE);
    return;
}

/**
 * @brief    Contains the initialization of pinmux settings and pad settings
 * @note     All the pinmux settings and pad settings shall be initiated in this function,
 *           but if legacy driver is used, the initialization of pinmux setting and pad setting
 *           should be peformed with the IO initializing.
 * @return   void
 */
void board_init(void)
{
    RCC_Configuration();
    PINMUX_Configuration();
    PAD_Configuration();
}

/**
  * @brief  Initialize SPI peripheral.
  * @param   No parameter.
  * @return  void
  */
void SPI_InitConfig(void)
{
    SPI_InitTypeDef  SPI_InitStructure;

    SPI_StructInit(&SPI_InitStructure);
    SPI_InitStructure.SPI_Direction   = SPI_Direction_EEPROM;
    SPI_InitStructure.SPI_Mode        = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize    = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL        = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA        = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_BaudRatePrescaler  = 100;//SPI Clock Speed = clk source/SPI_ClkDIV
    SPI_InitStructure.SPI_FrameFormat = SPI_Frame_Motorola;
    SPI_InitStructure.SPI_NDF         = 3;
    SPI_Init(SPI0, &SPI_InitStructure);

    SPI_Cmd(SPI0, ENABLE);
}
/**
  * @brief   SPI Read PSRAM ID.
  * @param   No parameter.
  * @return  void
  */
void SPI_ReadID(void)
{
    uint8_t sendBuf[16] = {0};
    uint8_t SPI_ReadBuf[16] = {0};
    uint8_t idx = 0;
    uint8_t len = 0;
    uint32_t address = 0;

    /* Configure read command and address */
    sendBuf[0] = 0x9f;
    sendBuf[1] = (address >> 16) & 0xff;
    sendBuf[2] = (address >> 8) & 0xff;
    sendBuf[3] = address & 0xff;
    SPI_SendBuffer(SPI0, sendBuf, 4);

    /*Waiting for SPI data transfer to end*/
    while (SPI_GetFlagState(SPI0, SPI_FLAG_BUSY));

    while (1)
    {
        /*read ID number of PSRAM*/
        len = SPI_GetRxFIFOLen(SPI0);
        for (idx = 0; idx < len; idx++)
        {
            SPI_ReadBuf[idx] = SPI_ReceiveData(SPI0);
            DBG_DIRECT("%d", SPI_ReadBuf[idx]);
        }
    }
}
int main(void)
{
    WDG_Disable();
    __enable_irq();
    board_init();
    SPI_InitConfig();
    SPI_ReadID();
}
