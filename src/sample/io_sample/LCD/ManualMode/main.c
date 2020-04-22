/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of LCD display.
* @details
* @author   yuan
* @date     2018-06-04
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

/* LCD reset pin and BL pin defines */
#define LCD_RST_PIN                     P2_4
#define LCD_BL_PIN                      P2_5

/* LCD color */
#define WHITE                           ((uint32_t)0xFFFF)
#define BLACK                           ((uint32_t)0x0000)
#define GREEN                           ((uint32_t)0x07E0)
#define BLUE                            ((uint32_t)0x001F)


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
    LCD_InitStruct.LCD_GuardTime        = LCD_GUARD_TIME_1T;
    LCD_InitStruct.LCD_8BitSwap         = LCD_8BitSwap_DISABLE;
    LCD_InitStruct.LCD_16BitSwap        = LCD_16BitSwap_ENABLE;
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
static void write_cmd(uint8_t cmd)
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
static void write_data(uint8_t data)
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
void write_block(uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd)
{
    write_cmd(0x2a);
    write_data(xStart >> 8);
    write_data(xStart & 0xff);
    write_data(xEnd >> 8);
    write_data(xEnd & 0xff);

    write_cmd(0x2b);
    write_data(yStart >> 8);
    write_data(yStart & 0xff);
    write_data(yEnd >> 8);
    write_data(yEnd & 0xff);
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
void write_color_box(uint16_t xStart, uint16_t yStart, uint16_t xWidth, uint16_t yWidth,
                     uint16_t color)
{
    uint32_t temp = 0;

    write_block(xStart, xStart + xWidth - 1, yStart, yStart + yWidth - 1);

    write_cmd(0x2c);
    for (temp = 0; temp < xWidth * yWidth; temp++)
    {
        write_data(color >> 8);
        write_data(color);
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
    write_cmd(0x13);

    /* Tearing effect line on */
    write_cmd(0x35);
    write_data(0x00);

    /* Memory data access */
    write_cmd(0x36);
    write_data(0x48);
    //write_data(0x40);

    /* MTPCTL */
    write_cmd(0xD0);
    write_data(0x00);
    write_data(0x05);

    write_cmd(0xEF);
    write_data(0x07);

    write_cmd(0xF2);
    write_data(0x1B);
    write_data(0x16);
    write_data(0x0F);
    write_data(0x08);
    write_data(0x08);
    write_data(0x08);
    write_data(0x08);
    write_data(0x10);
    write_data(0x00);
    write_data(0x1C);
    write_data(0x16);

    write_cmd(0xF3);
    write_data(0x01);
    write_data(0x41);
    write_data(0x15);
    write_data(0x0D);
    write_data(0x33);
    write_data(0x63);
    write_data(0x46);
    write_data(0x10);


    write_cmd(0xF4);
    write_data(0x5B);
    write_data(0x5B);
    write_data(0x55);
    write_data(0x55);
    write_data(0x44);

    write_cmd(0xF5);
    write_data(0x12);
    write_data(0x11);
    write_data(0x06);
    write_data(0xF0);
    write_data(0x00);
    write_data(0x1F);

    write_cmd(0xF6);
    write_data(0x80);
    write_data(0x10);
    write_data(0x00);

    write_cmd(0xFD);
    write_data(0x11);
    write_data(0x1D);
    write_data(0x00);

    //Positive Gamma Control
    write_cmd(0xF8);
    write_data(0x00);
    write_data(0x15);
    write_data(0x01);
    write_data(0x08);
    write_data(0x15);
    write_data(0x22);
    write_data(0x25);
    write_data(0x28);
    write_data(0x14);
    write_data(0x13);
    write_data(0x10);
    write_data(0x11);
    write_data(0x09);
    write_data(0x24);
    write_data(0x28);
    //Positive Gamma Control
    write_cmd(0xF9);
    write_data(0x00);
    write_data(0x15);
    write_data(0x01);
    write_data(0x08);
    write_data(0x15);
    write_data(0x22);
    write_data(0x25);
    write_data(0x28);
    write_data(0x14);
    write_data(0x13);
    write_data(0x10);
    write_data(0x11);
    write_data(0x09);
    write_data(0x24);
    write_data(0x28);
    //Positive Gamma Control
    write_cmd(0xFC);
    write_data(0x00);
    write_data(0x15);
    write_data(0x01);
    write_data(0x08);
    write_data(0x15);
    write_data(0x22);
    write_data(0x25);
    write_data(0x28);
    write_data(0x14);
    write_data(0x13);
    write_data(0x10);
    write_data(0x11);
    write_data(0x09);
    write_data(0x24);
    write_data(0x28);

    //Memory Data Access Control
    write_cmd(0x36);
    write_data(0x00);
    //SET 65K Color
    write_cmd(0x3A);
    write_data(0x55);
    //Exit Sleep
    write_cmd(0x11);

    //Display on
    write_cmd(0x29);
    /* Pull CS up */
    LCD_SetCS();
    delay(120);

    GPIO_SetBits(GPIO_GetPin(LCD_BL_PIN));
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

    write_color_box(0, 0, 320, 240, BLUE);
    write_color_box(0, 0, 320, 240, GREEN);
}

/**
  * @brief    Entry of app code
  * @return   int(To avoid compile warning)
  */
int main(void)
{
    lcd_demo();

    while (1)
    {
        write_color_box(0, 0, 320, 240, BLUE);
        write_color_box(0, 0, 320, 240, GREEN);
    }
}


/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

