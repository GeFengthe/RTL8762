/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     main.c
* @brief    This file provides demo code of LCD display in auto mode.
* @details
* @author   yuan
* @date     2019-01-15
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
#define GREEN                           ((uint32_t)0x07E0)
#define BLUE                            ((uint32_t)0x001F)

/* Picture size */
#define PICTURE_FRAME_SIZE              ((uint32_t)(240*160*2))

/* GDMA defines ---------------------------------------*/
#define GDMA_SOURCE_DATA_WIDTH          ((uint32_t)4)
#define GDMA_FRAME_SIZE                 4000

#define LCD_GDMA_Channel_NUM            0
#define LCD_GDMA_Channel                GDMA_Channel0
#define LCD_GDMA_Channel_IRQn           GDMA0_Channel0_IRQn
#define LCD_GDMA_Channel_Handler        GDMA0_Channel0_Handler


/* Globals -------------------------------------------------------------*/
RAM_DATAON_BSS_SECTION GDMA_LLIDef GDMA_LLIStruct[10];
RAM_DATAON_BSS_SECTION uint32_t LCD_Color;
//volatile uint8_t GDMA_INT_Flag = 0;

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
    LCD_InitStruct.LCD_GuardTime        = LCD_GUARD_TIME_2T;
    LCD_InitStruct.LCD_8BitSwap         = LCD_8BitSwap_ENABLE;
    LCD_InitStruct.LCD_16BitSwap        = LCD_16BitSwap_DISABLE;
    LCD_InitStruct.LCD_TxThr            = 10;
    LCD_InitStruct.LCD_TxDMACmd         = LCD_TX_DMA_DISABLE;
    LCD_Init(&LCD_InitStruct);
    LCD_Cmd(ENABLE);
}

/**
  * @brief  Config GDMA LLIStruct.
  * @param  No parameter.
  * @return void
  */
void GDMA_LLIStruct_Config(uint32_t addr, GDMA_InitTypeDef *GDMA_InitStruct)
{
    /* Configure LLI struct */
    uint32_t i = 0;
    uint32_t blk_count = PICTURE_FRAME_SIZE / (GDMA_FRAME_SIZE * GDMA_SOURCE_DATA_WIDTH);
    uint32_t remainer = PICTURE_FRAME_SIZE % (GDMA_FRAME_SIZE * GDMA_SOURCE_DATA_WIDTH) /
                        (GDMA_SOURCE_DATA_WIDTH);
    DBG_DIRECT("blk_count = %d, remainer = %d", blk_count, remainer);
    /* No need Multi-block */
    if (blk_count == 0)
    {
        GDMA_InitStruct->GDMA_BufferSize = remainer;
        GDMA_InitStruct->GDMA_Multi_Block_En = 0;
    }
    else
    {
        /* Check LLI is overflow or not */
        if (remainer > 0)
        {
            if ((sizeof(GDMA_LLIStruct) / sizeof(GDMA_LLIStruct[0])) <= blk_count)
            {
//                DBG_DIRECT("sizeof(GDMA_LLIStruct) / sizeof(GDMA_LLIStruct[0])) <= blk_count");
                return;
            }
        }
        else
        {
            if ((sizeof(GDMA_LLIStruct) / sizeof(GDMA_LLIStruct[0])) < blk_count)
            {
                return;
            }
        }

        /* Configure LLI */
        for (i = 0; i < blk_count; i++)
        {
            GDMA_LLIStruct[i].SAR = (uint32_t)(addr);
            GDMA_LLIStruct[i].DAR = (uint32_t)(&(LCD->FIFO));
            GDMA_LLIStruct[i].LLP = (uint32_t)&GDMA_LLIStruct[i + 1];
            /* configure low 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct->GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct->GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct->GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct->GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct->GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct->GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct->GDMA_DIR << 20)
                                        | (GDMA_InitStruct->GDMA_Multi_Block_Mode & LLP_SELECTED_BIT);
            /* configure high 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_HIGH = GDMA_FRAME_SIZE;
        }

        if (remainer > 0)
        {
            GDMA_LLIStruct[i].LLP = 0;
            GDMA_LLIStruct[i].SAR = (uint32_t)(addr);
            GDMA_LLIStruct[i].DAR = (uint32_t)(&(LCD->FIFO));
            /* configure low 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_LOW = BIT(0)
                                        | (GDMA_InitStruct->GDMA_DestinationDataSize << 1)
                                        | (GDMA_InitStruct->GDMA_SourceDataSize << 4)
                                        | (GDMA_InitStruct->GDMA_DestinationInc << 7)
                                        | (GDMA_InitStruct->GDMA_SourceInc << 9)
                                        | (GDMA_InitStruct->GDMA_DestinationMsize << 11)
                                        | (GDMA_InitStruct->GDMA_SourceMsize << 14)
                                        | (GDMA_InitStruct->GDMA_DIR << 20);
            /* configure high 32 bit of CTL register */
            GDMA_LLIStruct[i].CTL_HIGH = remainer;
        }
        else
        {
            GDMA_LLIStruct[i - 1].LLP = 0;

            /* configure low 32 bit of CTL register */
            GDMA_LLIStruct[i - 1].CTL_LOW = BIT(0)
                                            | (GDMA_InitStruct->GDMA_DestinationDataSize << 1)
                                            | (GDMA_InitStruct->GDMA_SourceDataSize << 4)
                                            | (GDMA_InitStruct->GDMA_DestinationInc << 7)
                                            | (GDMA_InitStruct->GDMA_SourceInc << 9)
                                            | (GDMA_InitStruct->GDMA_DestinationMsize << 11)
                                            | (GDMA_InitStruct->GDMA_SourceMsize << 14)
                                            | (GDMA_InitStruct->GDMA_DIR << 20);
        }
    }
}

/**
  * @brief  Initialize gdma peripheral.
  * @param  No parameter.
  * @return void
  */
void driver_gdma_init(uint32_t addr)
{
    /* Enable GDMA clock */
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    /* Initialize GDMA peripheral */
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = LCD_GDMA_Channel_NUM;
    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToPeripheral;
    GDMA_InitStruct.GDMA_BufferSize          = GDMA_FRAME_SIZE;
    GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
    GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_1;
    GDMA_InitStruct.GDMA_SourceAddr          = addr;
    GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)(&(LCD->FIFO));
    GDMA_InitStruct.GDMA_SourceHandshake     = GDMA_Handshake_SPIC_RX;
    GDMA_InitStruct.GDMA_DestHandshake       = GDMA_Handshake_LCD;
    GDMA_InitStruct.GDMA_Multi_Block_Mode    = LLI_TRANSFER;
    GDMA_InitStruct.GDMA_Multi_Block_En      = 1;
    GDMA_InitStruct.GDMA_Multi_Block_Struct  = (uint32_t)GDMA_LLIStruct;

    GDMA_LLIStruct_Config(addr, &GDMA_InitStruct);

    GDMA_Init(LCD_GDMA_Channel, &GDMA_InitStruct);

    GDMA_INTConfig(LCD_GDMA_Channel_NUM, GDMA_INT_Transfer, ENABLE);

    /* Enable GDMA IRQ */
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = LCD_GDMA_Channel_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = (FunctionalState)ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
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
  * @brief  Write data by auto mode.
  * @param  void.
  * @return void
  */
void lcd_auto_write(uint8_t cmd, uint32_t pixel_num)
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
    /* Enable GDMA */
    GDMA_Cmd(LCD_GDMA_Channel_NUM, ENABLE);
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
    write_color_box(0, 0, 320, 240, WHITE);

    while (1)
    {
        /* Stop LCD parallel interface */
        LCD_Cmd(DISABLE);
        /* Enable Manual mode */
        LCD_SwitchMode(LCD_MODE_MANUAL);
        LCD_Cmd(ENABLE);
        write_block(40, 280 - 1, 40, 200 - 1);

        /* Auto mode operation */
        LCD_Color = (BLUE << 16) + BLUE;
        driver_gdma_init((uint32_t)(&LCD_Color));
        lcd_auto_write(0x2C, (uint32_t)(PICTURE_FRAME_SIZE));
        while (GDMA_GetChannelStatus(LCD_GDMA_Channel_NUM) == SET);
        delay(10000000);

        /* Stop LCD parallel interface */
        LCD_Cmd(DISABLE);
        /* Enable Auto mode */
        LCD_SwitchMode(LCD_MODE_MANUAL);
        LCD_Cmd(ENABLE);
        write_block(40, 280 - 1, 40, 200 - 1);

        /* Auto mode operation */
        LCD_Color = (GREEN << 16) + GREEN;
        driver_gdma_init((uint32_t)(&LCD_Color));
        lcd_auto_write(0x2C, (uint32_t)(PICTURE_FRAME_SIZE));
        while (GDMA_GetChannelStatus(LCD_GDMA_Channel_NUM) == SET);
        delay(10000000);
    }
}

/**
  * @brief    Entry of app code
  * @return   int(To avoid compile warning)
  */
int main(void)
{
    __enable_irq();

    lcd_demo();

    while (1)
    {
    }
}

/**
  * @brief  GDMA interrupt handler function.
  * @param  No parameter.
  * @return void
*/
void GDMA0_Channel0_Handler(void)
{
    GDMA_ClearINTPendingBit(LCD_GDMA_Channel_NUM, GDMA_INT_Transfer);
    DBG_DIRECT("GDMA0_Channel0_Handler!");

}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

