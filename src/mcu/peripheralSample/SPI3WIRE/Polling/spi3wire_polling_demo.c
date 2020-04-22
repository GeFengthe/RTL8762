/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     spi3wire_polling_demo.c
* @brief    This file provides demo code of three wire SPI comunication.
* @details
* @author   yuan
* @date     2018-05-29
* @version  v0.1
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rtl876x_3wire_spi.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "trace.h"

/* Defines -------------------------------------------------------------------*/
//#define SENSOR_SPI              SPI3WIRE
#define SPI_3WIRE_CLK_PIN       P3_2
#define SPI_3WIRE_DATA_PIN      P3_3

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return Void
  */
void board_3wire_spi_init(void)
{
    Pad_Config(SPI_3WIRE_CLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(SPI_3WIRE_DATA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pinmux_Config(SPI_3WIRE_CLK_PIN, SPI2W_CLK);
    Pinmux_Config(SPI_3WIRE_DATA_PIN, SPI2W_DATA);
}

/**
  * @brief  Initialize SPI3WIRE peripheral.
  * @param  No parameter.
  * @return Void
  */
void driver_3wire_spi_init(void)
{
    /* Enable 20M clock source */
    SYSBLKCTRL->u_200.SYS_CLK_SEL |= 0x3 << 13;
    //SYSBLKCTRL->u_218.PERI_FUNC0_EN |= 1<<24;
    //SYSBLKCTRL->u_238.PERI_CLK_CTRL1 |=0x03<<16;

    /* Enable SPI3WIRE clock */
    RCC_PeriphClockCmd(APBPeriph_SPI2W, APBPeriph_SPI2W_CLOCK, ENABLE);

    /* Initialize IR */
    SPI3WIRE_InitTypeDef SPI3WIRE_InitStruct;
    SPI3WIRE_StructInit(&SPI3WIRE_InitStruct);

    SPI3WIRE_InitStruct.SPI3WIRE_SysClock       = 20000000;
    SPI3WIRE_InitStruct.SPI3WIRE_Speed          = 800000;
    SPI3WIRE_InitStruct.SPI3WIRE_Mode           = SPI3WIRE_2WIRE_MODE;
    /* delay time = (SPI3WIRE_ReadDelay +1)/(2*SPI3WIRE_Speed). The delay time from the end of address phase to the start of read data phase */
    //delay time = (0x06 + 3)/(2 * speed) = 3us
    SPI3WIRE_InitStruct.SPI3WIRE_ReadDelay      = 0x3;
    SPI3WIRE_InitStruct.SPI3WIRE_OutputDelay    = SPI3WIRE_OE_DELAY_NONE;
    SPI3WIRE_InitStruct.SPI3WIRE_ExtMode        = SPI3WIRE_NORMAL_MODE;
    SPI3WIRE_Init(&SPI3WIRE_InitStruct);
}

/**
  * @brief  Read one byte through 3wire SPI perpherial .
  * @param  address: address of register which need to read .
  * @return Value of register.
*/
uint8_t spi3wire_readbyte(uint8_t address)
{
    uint8_t reg_value = 0;
    uint32_t timeout = 0;

    /* Check SPI busy or not */
    while (SPI3WIRE_GetFlagStatus(SPI3WIRE_FLAG_BUSY) == SET)
    {
        timeout++;
        if (timeout > 0x1ffff)
        {
            break;
        }
    }

    /* Clear Receive data length */
    SPI3WIRE_ClearRxDataLen();

    SPI3WIRE_StartRead(address, 1);

    timeout = 0;
    while (SPI3WIRE_GetFlagStatus(SPI3WIRE_FLAG_BUSY) == SET)
    {
        timeout++;
        if (timeout > 0x1ffff)
        {
            break;
        }
    }

    /* Get the length of received data */
    while (SPI3WIRE_GetRxDataLen() == 0);
    /* Read data */
    SPI3WIRE_ReadBuf(&reg_value, 1);

    return reg_value;
}

/**
  * @brief  Write one byte.
  * @param  address: address of register which need to write data.
  * @param  data: data which need to write.
  * @return TRUE: write success, FALSE: write failure.
*/
bool spi3wire_writebyte(uint8_t address, uint8_t data)
{
    uint32_t timeout = 0;

    /* Check SPI busy or not */
    while (SPI3WIRE_GetFlagStatus(SPI3WIRE_FLAG_BUSY) == SET)
    {
        timeout++;
        if (timeout > 0x1ffff)
        {
            return false;
        }
    }
    /* Write data */
    SPI3WIRE_StartWrite(address, data);

    timeout = 0;
    while (SPI3WIRE_GetFlagStatus(SPI3WIRE_FLAG_BUSY) == SET)
    {
        timeout++;
        if (timeout > 0x1ffff)
        {
            return false;
        }
    }
    return true;
}

/**
  * @brief  Reset mouse.
  * @param  None.
  * @return Void.
*/
void mouse_reset(void)
{
    uint32_t delay = 0;

    spi3wire_writebyte(0x06, (1 << 7));

    /* Delay 0.28ms */
    for (delay = 0; delay < 0x1fff; delay++) {;}
}

/**
  * @brief  Read mouse product ID.
  * @param  p_id, --pointer to production id buffer,buffer length should more than two.
  * @return TRUE.
*/
bool mouse_getproductid(uint8_t *p_id)
{

    /* Read product ID number high byte */
    *p_id++ = spi3wire_readbyte(0x20);

    /* Read product ID number low byte */
    *p_id = spi3wire_readbyte(0x21);

    return true;
}

/**
  * @brief  Demo code of SPI3WIRE communication.
  * @param  No parameter.
  * @return Void
  */
void spi3wire_demo(void)
{
    uint8_t id[2] = {0, 0};

    board_3wire_spi_init();
    driver_3wire_spi_init();

    /* Send resync time. Resync signal time = 2*1/(2*SPI3WIRE_Speed) = 1us */
    SPI3WIRE_SetResyncTime(2);
    SPI3WIRE_ResyncSignalCmd(ENABLE);
    while (SPI3WIRE_GetFlagStatus(SPI3WIRE_FLAG_RESYNC_BUSY) == SET);
    SPI3WIRE_ResyncSignalCmd(DISABLE);

    /* Enable SPI3WIRE to normal communication */
    SPI3WIRE_Cmd(ENABLE);

    mouse_reset();
    mouse_getproductid(&id[0]);
    if ((0x58 == id[0]) && (0x20 == (id[1] & 0xF0)))
    {
        DBG_DIRECT("SPI3WIRE test OK!");
    }
    if ((0x58 != id[0]) || (0x20 != (id[1] & 0xF0)))
    {
        //Read mouse sensor ID error!
    }
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/

