/**
*********************************************************************************************************
*               Copyright(c) 2018, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     switch_io.c
* @brief
* @details
* @author   Elliot Chen
* @date     2018-9-13
* @version  v1.0
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "switch_io.h"
#include "trace.h"

/* Globals ------------------------------------------------------------------*/
SWITCH_STATUS switch_status;

/**
  * @brief  Initialize all switch status.
  * @param  No parameter.
  * @return void
*/
void switch_io_status_init(void)
{
    memset(&switch_status, 0, sizeof(SWITCH_STATUS));
}

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
*/
void board_switch_io_init(void)
{
#if SWITCH_POLARITY_ACTIVE_LOW
    Pad_Config(LEFT_SWITCH_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);
#else
    Pad_Config(LEFT_SWITCH_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);
#endif
    Pad_Config(LEFT_LIGHT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pinmux_Config(LEFT_SWITCH_PIN, DWGPIO);
    Pinmux_Config(LEFT_LIGHT_PIN, DWGPIO);
}

/**
  * @brief  Initialize GPIO peripheral.
  * @param  No parameter.
  * @return void
*/
void driver_switch_io_init(void)
{
    /* Initialize GPIO peripheral */
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin                = GPIO_GetPin(LEFT_SWITCH_PIN);
    GPIO_InitStruct.GPIO_Mode               = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_ITCmd              = ENABLE;
#if 1
    GPIO_InitStruct.GPIO_ITTrigger          = GPIO_INT_Trigger_EDGE;
#if SWITCH_POLARITY_ACTIVE_LOW
    GPIO_InitStruct.GPIO_ITPolarity         = GPIO_INT_POLARITY_ACTIVE_LOW;
#else
    GPIO_InitStruct.GPIO_ITPolarity         = GPIO_INT_POLARITY_ACTIVE_HIGH;
#endif
#else
    GPIO_InitStruct.GPIO_ITTrigger          = GPIO_INT_BOTH_EDGE;
#endif
    GPIO_InitStruct.GPIO_ITDebounce         = GPIO_INT_DEBOUNCE_ENABLE;
    GPIO_InitStruct.GPIO_DebounceTime       = LEFT_SWITCH_PIN_DB_MS;
    GPIO_Init(&GPIO_InitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel         = LEFT_SWITCH_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd      = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    GPIO_ClearINTPendingBit(GPIO_GetPin(LEFT_SWITCH_PIN));
    GPIO_MaskINTConfig(GPIO_GetPin(LEFT_SWITCH_PIN), DISABLE);
    GPIO_INTConfig(GPIO_GetPin(LEFT_SWITCH_PIN), ENABLE);

    /* Initialize GPIO peripheral */
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin                = GPIO_GetPin(LEFT_LIGHT_PIN);
    GPIO_InitStruct.GPIO_Mode               = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_ITCmd              = DISABLE;
    GPIO_Init(&GPIO_InitStruct);
    GPIO_SetBits(GPIO_GetPin(LEFT_LIGHT_PIN));
}

void switch_io_light_enter_dlps_config(void)
{
    if (switch_status.switch_status_bit.left_switch_status_bit)
    {
        Pad_Config(LEFT_LIGHT_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    }
    else
    {
        Pad_Config(LEFT_LIGHT_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    }
}

void switch_io_light_exit_dlps_config(void)
{
    if (switch_status.switch_status_bit.left_switch_status_bit)
    {
        Pad_Config(LEFT_LIGHT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
                   PAD_OUT_LOW);
    }
    else
    {
        Pad_Config(LEFT_LIGHT_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
                   PAD_OUT_HIGH);
    }
}

void switch_io_enter_dlps_config(void)
{
    /* @note: no key is pressed, use PAD wake up function with debounce,
    but pad debunce time should be smaller than ble connect interval */
    System_WakeUpDebounceTime(0x08);
#if SWITCH_POLARITY_ACTIVE_LOW
    Pad_Config(LEFT_SWITCH_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);
    System_WakeUpPinEnable(LEFT_SWITCH_PIN, PAD_WAKEUP_POL_LOW, PAD_WK_DEBOUNCE_ENABLE);
#else
    Pad_Config(LEFT_SWITCH_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);
    System_WakeUpPinEnable(LEFT_SWITCH_PIN, PAD_WAKEUP_POL_HIGH, PAD_WK_DEBOUNCE_ENABLE);
#endif

    switch_io_light_enter_dlps_config();
}

void switch_io_exit_dlps_config(void)
{
#if SWITCH_POLARITY_ACTIVE_LOW
    Pad_Config(LEFT_SWITCH_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);
#else
    Pad_Config(LEFT_SWITCH_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);
#endif

    switch_io_light_exit_dlps_config();
}

void switch_io_handle_msg_exit_dlps(void)
{
#if SWITCH_POLARITY_ACTIVE_LOW
    if (GPIO_ReadInputDataBit(GPIO_GetPin(LEFT_SWITCH_PIN)) == RESET)
    {
        /* press */
        GPIO->INTPOLARITY |= GPIO_GetPin(LEFT_SWITCH_PIN);
        switch_io_ctrl_dlps(false);
    }
#else
    if (GPIO_ReadInputDataBit(GPIO_GetPin(LEFT_SWITCH_PIN)) == SET)
    {
        /* press */
        GPIO->INTPOLARITY &= ~(GPIO_GetPin(LEFT_SWITCH_PIN));
        switch_io_ctrl_dlps(false);
    }
#endif
}

static void switch_swap_status(uint32_t gpio_pin)
{
    if (switch_status.switch_status_bit.left_switch_status_bit)
    {
        GPIO_WriteBit(gpio_pin, Bit_SET);
        switch_status.switch_status_bit.left_switch_status_bit = 0;
    }
    else
    {
        GPIO_WriteBit(gpio_pin, Bit_RESET);
        switch_status.switch_status_bit.left_switch_status_bit = 1;
    }
}

void switch_light_cmd(bool is_on)
{
    if (is_on)
    {
        GPIO_WriteBit(GPIO_GetPin(LEFT_LIGHT_PIN), Bit_RESET);
        switch_status.switch_status_bit.left_switch_status_bit = 1;
    }
    else
    {
        GPIO_WriteBit(GPIO_GetPin(LEFT_LIGHT_PIN), Bit_SET);
        switch_status.switch_status_bit.left_switch_status_bit = 0;
    }
}

SWITCH_STATUS *switch_get_status(void)
{
    return (&switch_status);
}

void switch_handle_io_msg(T_IO_MSG *io_msg)
{
    switch (io_msg->subtype)
    {
    case LEFT_SWITCH_PRESS:
        {
            switch_io_ctrl_dlps(false);
            break;
        }
    case LEFT_SWITCH_RELEASE:
        {
            switch_swap_status(GPIO_GetPin(LEFT_LIGHT_PIN));
            switch_io_ctrl_dlps(true);
            break;
        }
    default:
        {
            break;
        }
    }
}

/**
* @brief  GPIO interrupt trigger by left switch is handled in this function.
* @param  none
* @return  void
*/
void Left_Switch_Handler(void)
{
    GPIO_INTConfig(GPIO_GetPin(LEFT_SWITCH_PIN), DISABLE);
    GPIO_MaskINTConfig(GPIO_GetPin(LEFT_SWITCH_PIN), ENABLE);

    APP_PRINT_INFO0("Enter GPIO Interrupt");
    switch_io_ctrl_dlps(false);
    T_IO_MSG switch_msg;
    switch_msg.type = IO_MSG_TYPE_GPIO;

#if SWITCH_POLARITY_ACTIVE_LOW
    if (GPIO_ReadInputDataBit(GPIO_GetPin(LEFT_SWITCH_PIN)))
    {
        GPIO->INTPOLARITY &= ~(GPIO_GetPin(LEFT_SWITCH_PIN));
        switch_msg.subtype = LEFT_SWITCH_RELEASE;
    }
    else
    {
        GPIO->INTPOLARITY |= GPIO_GetPin(LEFT_SWITCH_PIN);
        switch_msg.subtype = LEFT_SWITCH_PRESS;
    }
#else
    if (GPIO_ReadInputDataBit(GPIO_GetPin(LEFT_SWITCH_PIN)))
    {
        GPIO->INTPOLARITY &= ~(GPIO_GetPin(LEFT_SWITCH_PIN));
        switch_msg.subtype = LEFT_SWITCH_PRESS;
    }
    else
    {
        GPIO->INTPOLARITY |= GPIO_GetPin(LEFT_SWITCH_PIN);
        switch_msg.subtype = LEFT_SWITCH_RELEASE;
    }
#endif
    app_send_msg_to_apptask(&switch_msg);

    GPIO_ClearINTPendingBit(GPIO_GetPin(LEFT_SWITCH_PIN));
    GPIO_MaskINTConfig(GPIO_GetPin(LEFT_SWITCH_PIN), DISABLE);
    GPIO_INTConfig(GPIO_GetPin(LEFT_SWITCH_PIN), ENABLE);
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
