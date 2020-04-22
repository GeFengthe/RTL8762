/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     lcd_auto_write_by_mcu_demo.c
* @brief    This file provides demo code of LCD display in auto mode.
* @details
* @author   yuan
* @date     2018-06-06
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "rtl876x_gdma.h"
#include "rtl876x_gpio.h"
#include "rtl876x_lcd.h"
#include "rtl876x_nvic.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"

#include "app_section.h"
#include "flash_device.h"
#include "trace.h"

/* Defines -------------------------------------------------------------------*/
/* LCD reset pin and BL pin defines -----------------------*/
#define LCD_RST_PIN                     P2_4
#define LCD_BL_PIN                      P2_5

/* LCD color */
#define WHITE                           ((uint32_t)0xFFFF)
#define BLACK                           ((uint32_t)0x0000)
#define RED                             ((uint32_t)0x1F00)
#define GREEN                           ((uint32_t)0x07E0)
#define BLUE                            ((uint32_t)0x001F)

/* Picture size */
#define PICTURE_FRAME_SIZE              ((uint32_t)(240*240*2))

/* Globals -------------------------------------------------------------*/
RAM_DATAON_BSS_SECTION uint32_t LCD_Color;

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_gpio_init(void)
{
    Pad_Config(LCD_RST_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(LCD_BL_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);

    Pinmux_Config(LCD_RST_PIN, DWGPIO);
    Pinmux_Config(LCD_BL_PIN, DWGPIO);
}

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_lcd_init(void)
{
#if 0
    Pad_Config(P0_2, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P0_4, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P1_3, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P1_4, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P4_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P4_1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P4_2, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P4_3, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);

    Pad_Config(P3_3, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P3_4, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P3_2, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P3_5, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);

    /* 8080 interface: D0~D7 */
    Pinmux_Config(P0_2, IDLE_MODE);
    Pinmux_Config(P0_4, IDLE_MODE);
    Pinmux_Config(P1_3, IDLE_MODE);
    Pinmux_Config(P1_4, IDLE_MODE);
    Pinmux_Config(P4_0, IDLE_MODE);
    Pinmux_Config(P4_1, IDLE_MODE);
    Pinmux_Config(P4_2, IDLE_MODE);
    Pinmux_Config(P4_3, IDLE_MODE);

    /* CS */
    Pinmux_Config(P3_3, IDLE_MODE);
    /* DCX */
    Pinmux_Config(P3_4, IDLE_MODE);
    /* RD */
    Pinmux_Config(P3_2, IDLE_MODE);
    /* WR */
    Pinmux_Config(P3_5, IDLE_MODE);

#else
    Pad_Config(P3_5, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P0_1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P0_2, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P0_4, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P4_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P4_1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P4_2, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P4_3, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);

    Pad_Config(P2_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P3_2, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P3_3, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pad_Config(P3_4, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);

    /* 8080 interface: D0~D7 */
    Pinmux_Config(P3_5, IDLE_MODE);
    Pinmux_Config(P0_1, IDLE_MODE);
    Pinmux_Config(P0_2, IDLE_MODE);
    Pinmux_Config(P0_4, IDLE_MODE);
    Pinmux_Config(P4_0, IDLE_MODE);
    Pinmux_Config(P4_1, IDLE_MODE);
    Pinmux_Config(P4_2, IDLE_MODE);
    Pinmux_Config(P4_3, IDLE_MODE);

    /* CS */
    Pinmux_Config(P3_3, IDLE_MODE);
    /* DCX */
    Pinmux_Config(P3_4, IDLE_MODE);
    /* RD */
    Pinmux_Config(P2_0, IDLE_MODE);
    /* WR */
    Pinmux_Config(P3_2, IDLE_MODE);

#endif
}

/**
  * @brief  Initialize gpio peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_gpio_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_ControlMode = GPIO_SOFTWARE_MODE;
    GPIO_InitStruct.GPIO_Pin  = GPIO_GetPin(LCD_RST_PIN) | GPIO_GetPin(LCD_BL_PIN);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
}

/**
  * @brief  Initialize LCD controller peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_lcd_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_LCD, APBPeriph_LCD_CLOCK, ENABLE);

    LCD_PinGroupConfig(LCD_PinGroup_2);
    LCD_InitTypeDef LCD_InitStruct;
    LCD_StructInit(&LCD_InitStruct);
    LCD_InitStruct.LCD_ClockDiv         = LCD_CLOCK_DIV_2;
    LCD_InitStruct.LCD_Mode             = LCD_MODE_MANUAL;
    LCD_InitStruct.LCD_GuardTimeCmd     = LCD_GUARD_TIME_DISABLE;
    LCD_InitStruct.LCD_GuardTime        = LCD_GUARD_TIME_2T;
    LCD_InitStruct.LCD_8BitSwap         = LCD_8BitSwap_ENABLE;
    LCD_InitStruct.LCD_16BitSwap        = LCD_16BitSwap_DISABLE;
    LCD_InitStruct.LCD_TxThr            = 10;
    LCD_InitStruct.LCD_TxDMACmd         = LCD_TX_DMA_DISABLE;
    LCD_Init(&LCD_InitStruct);
    LCD_Cmd(ENABLE);
}

/**
  * @brief  Send command.
  * @param  cmd: command index.
  * @return void
*/
void WriteComm(uint8_t cmd)
{
    LCD_SetCS();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    LCD_ResetCS();
    LCD_SendCommand(cmd);
}

/**
  * @brief  Send data.
  * @param  data: data to be sent.
  * @return void
*/
void WriteData(uint8_t data)
{
    LCD_SendData(&data, 1);
}

/**
  * @brief  Configure parameter of block write.
  * @param  xStart: start position of X axis.
  * @param  xEnd: end position of X axis.
  * @param  yStart: start position of Y axis.
  * @param  yEnd: end position of Y axis.
  * @return void
*/
void WriteBlock(uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd)
{
    WriteComm(0x2a);
    WriteData(xStart >> 8);
    WriteData(xStart & 0xff);
    WriteData(xEnd >> 8);
    WriteData(xEnd & 0xff);

    WriteComm(0x2b);
    WriteData(yStart >> 8);
    WriteData(yStart & 0xff);
    WriteData(yEnd >> 8);
    WriteData(yEnd & 0xff);
    LCD_SetCS();
}

/**
  * @brief  Configure parameter of block write.
  * @param  xStart: start position of X axis.
  * @param  yStart: start position of Y axis.
  * @param  xWidth: width of X axis.
  * @param  yWidth: width of Y axis.
  * @param  color: color data.
  * @return void
*/
void WriteColorBox(uint16_t xStart, uint16_t yStart, uint16_t xWidth, uint16_t yWidth,
                   uint16_t color)
{
    uint32_t temp = 0;

    WriteBlock(xStart, xStart + xWidth - 1, yStart, yStart + yWidth - 1);

    WriteComm(0x2c);
    for (temp = 0; temp < xWidth * yWidth; temp++)
    {
        WriteData(color >> 8);
        WriteData(color);
    }
    LCD_SetCS();
}

/**
  * @brief  Software delay.
  * @param  nCount: delay count.
  * @return void
*/
volatile static void delay(uint32_t nCount)
{
    volatile int i;
    for (i = 0; i < 7200; i++)
        for (; nCount != 0; nCount--);
}

/**
  * @brief  Initialize external LCD.
  * @param  No parameter.
  * @return void
*/
void external_lcd_init(void)
{
    GPIO_ResetBits(GPIO_GetPin(LCD_RST_PIN));
    delay(10000);
    GPIO_SetBits(GPIO_GetPin(LCD_RST_PIN));
    delay(10000);

    /* Initialize S6D05A13 */

    /* Normal display mode on */
    WriteComm(0x13);

    /* Tearing effect line on */
    WriteComm(0x35);
    WriteData(0x00);

    /* Memory data access */
    WriteComm(0x36);
    WriteData(0x48);
    //WriteData(0x40);

    /* MTPCTL */
    WriteComm(0xD0);
    WriteData(0x00);
    WriteData(0x05);

    WriteComm(0xEF);
    WriteData(0x07);

    WriteComm(0xF2);
    WriteData(0x1B);
    WriteData(0x16);
    WriteData(0x0F);
    WriteData(0x08);
    WriteData(0x08);
    WriteData(0x08);
    WriteData(0x08);
    WriteData(0x10);
    WriteData(0x00);
    WriteData(0x1C);
    WriteData(0x16);

    WriteComm(0xF3);
    WriteData(0x01);
    WriteData(0x41);
    WriteData(0x15);
    WriteData(0x0D);
    WriteData(0x33);
    WriteData(0x63);
    WriteData(0x46);
    WriteData(0x10);


    WriteComm(0xF4);
    WriteData(0x5B);
    WriteData(0x5B);
    WriteData(0x55);
    WriteData(0x55);
    WriteData(0x44);

    WriteComm(0xF5);
    WriteData(0x12);
    WriteData(0x11);
    WriteData(0x06);
    WriteData(0xF0);
    WriteData(0x00);
    WriteData(0x1F);

    WriteComm(0xF6);
    WriteData(0x80);
    WriteData(0x10);
    WriteData(0x00);

    WriteComm(0xFD);
    WriteData(0x11);
    WriteData(0x1D);
    WriteData(0x00);

    //Positive Gamma Control
    WriteComm(0xF8);
    WriteData(0x00);
    WriteData(0x15);
    WriteData(0x01);
    WriteData(0x08);
    WriteData(0x15);
    WriteData(0x22);
    WriteData(0x25);
    WriteData(0x28);
    WriteData(0x14);
    WriteData(0x13);
    WriteData(0x10);
    WriteData(0x11);
    WriteData(0x09);
    WriteData(0x24);
    WriteData(0x28);
    //Positive Gamma Control
    WriteComm(0xF9);
    WriteData(0x00);
    WriteData(0x15);
    WriteData(0x01);
    WriteData(0x08);
    WriteData(0x15);
    WriteData(0x22);
    WriteData(0x25);
    WriteData(0x28);
    WriteData(0x14);
    WriteData(0x13);
    WriteData(0x10);
    WriteData(0x11);
    WriteData(0x09);
    WriteData(0x24);
    WriteData(0x28);
    //Positive Gamma Control
    WriteComm(0xFC);
    WriteData(0x00);
    WriteData(0x15);
    WriteData(0x01);
    WriteData(0x08);
    WriteData(0x15);
    WriteData(0x22);
    WriteData(0x25);
    WriteData(0x28);
    WriteData(0x14);
    WriteData(0x13);
    WriteData(0x10);
    WriteData(0x11);
    WriteData(0x09);
    WriteData(0x24);
    WriteData(0x28);

    //Memory Data Access Control
    WriteComm(0x36);
    WriteData(0x00);
    //SET 65K Color
    WriteComm(0x3A);
    WriteData(0x55);
    //Exit Sleep
    WriteComm(0x11);

    //Display on
    WriteComm(0x29);
    /* Pull CS up */
    LCD_SetCS();
    delay(120);

    GPIO_SetBits(GPIO_GetPin(LCD_BL_PIN));
}

/**
  * @brief Write FIFO in auto mode.
  * @param  data: color data.
  * @param  len: data length.
  * @return  void
  */
void LCD_WriteInAutoMode(uint32_t data, uint32_t len)
{
    uint32_t temp = 0;

    for (temp = 0; temp < len; temp++)
    {
        LCD_WriteFIFO(data);
        while (LCD_GetFlagStatus(LCD_FLAG_TF_FULL));
    }
}

/**
  * @brief  Write data by auto mode.
  * @param  void.
  * @return  void
  */
void LCD_AutoWrite(uint8_t cmd, uint32_t data, uint32_t pixel_num)
{
    LCD_ClearDataCounter();

    /* Stop LCD parallel interface */
    LCD_SetCS();
    LCD_Cmd(DISABLE);

    /* Enable Auto mode */
    LCD_SwitchMode(LCD_MODE_AUTO);

    /* Configure command */
    LCD_SetCmdSequence(&cmd, 1);

    /* Enable GDMA TX */
    LCD_GDMACmd(ENABLE);

    /* Configure pixel number */
    LCD_SetTxDataLen(pixel_num);

    /* Start output */
    LCD_Cmd(ENABLE);

    /* Write data */
    LCD_WriteInAutoMode(data, pixel_num / 4);
}

/**
  * @brief  Demo code of LCD controller communication.
  * @param  No parameter.
  * @return void
*/
void lcd_demo(void)
{
    /* Initialize LCD peripheral */
    board_gpio_init();
    board_lcd_init();
    driver_gpio_init();
    driver_lcd_init();

    external_lcd_init();
    WriteColorBox(0, 0, 320, 240, BLACK);

    while (1)
    {
        /* Stop LCD parallel interface */
        LCD_Cmd(DISABLE);
        /* Enable manual mode */
        LCD_SwitchMode(LCD_MODE_MANUAL);
        LCD_Cmd(ENABLE);
        WriteBlock(80, 320 - 1, 0, 240 - 1);

        /* Auto mode operation */
//        LCD_Color = (BLUE << 16) + BLUE;
        LCD_Color = RED + BLUE;
        LCD_AutoWrite(0x2C, LCD_Color, (uint32_t)(PICTURE_FRAME_SIZE));
        delay(100000);

        /* Stop LCD parallel interface */
        LCD_Cmd(DISABLE);
        /* Enable manual mode */
        LCD_SwitchMode(LCD_MODE_MANUAL);
        LCD_Cmd(ENABLE);
        WriteBlock(0, 240 - 1, 0, 240 - 1);

        /* Auto mode operation */
        LCD_Color = (GREEN << 16) + GREEN;
        LCD_AutoWrite(0x2C, LCD_Color, (uint32_t)(PICTURE_FRAME_SIZE));
        delay(100000);
    }
}

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

