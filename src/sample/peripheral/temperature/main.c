#include <trace.h>
#include <os_sched.h>
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"
#include "rtl876x_gpio.h"
#include "rtl876x_wdg.h"
#include "icm20618_driver.h"
#include "board.h"
#include "platform_utils.h"

/**
 * @brief    pinmux configuration
 * @return   void
 */
void PINMUX_Configuration(void)
{
    Pinmux_Config(I2C0_SCL, I2C0_CLK);
    Pinmux_Config(I2C0_SDA, I2C0_DAT);
    return;
}
/**
 * @brief    pad configuration
 * @return   void
 */
void PAD_Configuration(void)
{

    Pad_Config(I2C0_SCL, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(I2C0_SDA, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_HIGH);
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
  * @brief  Initialize IO peripheral.
  * @param   No parameter.
  * @return  void
  */
void Driver_ICM20618_I2C_Init(void)
{
    /* Initialize I2C */
    I2C_InitTypeDef  I2C_InitStructure;
    I2C_StructInit(&I2C_InitStructure);
    I2C_InitStructure.I2C_ClockSpeed = ICM20618_I2C_SPEED;
    I2C_InitStructure.I2C_DeviveMode = I2C_DeviveMode_Master;
    I2C_InitStructure.I2C_AddressMode = I2C_AddressMode_7BIT;
    I2C_InitStructure.I2C_SlaveAddress = ICM20618_I2C_ADDR;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_Init(I2C0, &I2C_InitStructure);
    I2C_Cmd(I2C0, ENABLE);
}
int main(void)
{
    int16_t TEMP = 0;
    WDG_Disable();
    __enable_irq();
    board_init();
    Driver_ICM20618_I2C_Init();
    while (!ICM20618_Init());
    platform_delay_ms(500);
    TEMP = ICM20618_Get_MEMS_Data(MEMS_TEMP);
    float value = ICM20618_MEMS_Data_Transform(MEMS_TEMP, TEMP);
    while (1)
    {
        __nop();
        __nop();
        __nop();
        __nop();
        __nop();
    }

}
