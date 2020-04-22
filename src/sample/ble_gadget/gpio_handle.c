#include <board.h>
#include <rtl876x_pinmux.h>
#include <rtl876x_gpio.h>
#include <rtl876x_rcc.h>
#include <rtl876x_nvic.h>
#include <app_msg.h>
#include <app_task.h>
#include <gpio_handle.h>
#include <os_timer.h>
#include <trace.h>

uint8_t keystatus = 0;
static void *xTimerLongPress;
static void *xTimerAdvCtl;
static void *xTimerFlash;

static bool is_long_press = false;

void gpio_driver_init(void)
{
    RCC_PeriphClockCmd(APBPeriph_GPIO, APBPeriph_GPIO_CLOCK, ENABLE);
}

void gpio_config_init(void)
{
    gpio_driver_init();
    Pad_Config(LPN_BUTTON, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);
    Pad_Config(LED_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    Pinmux_Config(LPN_BUTTON, DWGPIO);
    Pinmux_Config(LED_PIN, DWGPIO);

    GPIO_InitTypeDef GPIO_struct_init;
    GPIO_StructInit(&GPIO_struct_init);

    GPIO_struct_init.GPIO_Pin = GPIO_LPN_BUTTON;
    GPIO_struct_init.GPIO_Mode = GPIO_Mode_IN;
    GPIO_struct_init.GPIO_DebounceTime = 20;
    GPIO_struct_init.GPIO_ITTrigger = GPIO_INT_Trigger_EDGE;
    GPIO_struct_init.GPIO_ITCmd = ENABLE;
    GPIO_struct_init.GPIO_ITDebounce = GPIO_INT_DEBOUNCE_ENABLE;
    GPIO_struct_init.GPIO_ITPolarity = GPIO_INT_POLARITY_ACTIVE_LOW;
    GPIO_Init(&GPIO_struct_init);
    keystatus = GPIO_ReadInputDataBit(GPIO_LPN_BUTTON);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = GPIO_LPN_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    GPIO_MaskINTConfig(GPIO_LPN_BUTTON, DISABLE);
    GPIO_INTConfig(GPIO_LPN_BUTTON, ENABLE);

    GPIO_StructInit(&GPIO_struct_init);
    GPIO_struct_init.GPIO_Pin = GPIO_LED_PIN;
    GPIO_struct_init.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_struct_init.GPIO_ITCmd = DISABLE;
    GPIO_Init(&GPIO_struct_init);

    GPIO_ResetBits(GPIO_LED_PIN);
}

void GPIO_LPN_Handler(void)
{
    //GPIO_INTConfig(GPIO_LPN_BUTTON, DISABLE);
    GPIO_MaskINTConfig(GPIO_LPN_BUTTON, ENABLE);

    keystatus = GPIO_ReadInputDataBit(GPIO_LPN_BUTTON);
    T_IO_MSG button_msg;
    if (keystatus == 0)
    {
        is_long_press = false;
        GPIO->INTPOLARITY |= GPIO_LPN_BUTTON;
        os_timer_start(&xTimerLongPress);
    }
    else
    {
        GPIO->INTPOLARITY &= ~GPIO_LPN_BUTTON;
        if (!is_long_press)
        {
            os_timer_stop(&xTimerLongPress);
            button_msg.type = IO_MSG_TYPE_GPIO;
            button_msg.subtype = MSG_BUTTON_SHORT_PRESS;
            app_send_msg_to_apptask(&button_msg);
        }
        is_long_press = false;
    }

    GPIO_ClearINTPendingBit(GPIO_LPN_BUTTON);
    GPIO_MaskINTConfig(GPIO_LPN_BUTTON, DISABLE);
    //GPIO_INTConfig(GPIO_LPN_BUTTON, ENABLE);
}


void pTimerLongPressCallback(void *pTimer)
{
    T_IO_MSG button_msg;
    is_long_press = true;
    button_msg.type = IO_MSG_TYPE_GPIO;
    button_msg.subtype = MSG_BUTTON_LONG_PRESS;
    app_send_msg_to_apptask(&button_msg);
}

void pTimerAdvCtlCallback(void *pTimer)
{
    T_IO_MSG adv_msg;
    adv_msg.type = IO_MSG_TYPE_TIMER;
    app_send_msg_to_apptask(&adv_msg);
}

bool invert_led = true;
void pTimerFlashCallback(void *pTimer)
{
    if (invert_led)
    {
        GPIO_SetBits(GPIO_LED_PIN);
        invert_led = false;
    }
    else
    {
        GPIO_ResetBits(GPIO_LED_PIN);
        invert_led = true;
    }
}

void sw_timer_init(void)
{
    uint8_t ret;
    ret = os_timer_create(&xTimerLongPress, "xTimerLongPress", 0, 2000, false,
                          pTimerLongPressCallback);
    if (!ret)
    {
        APP_PRINT_ERROR0("xTimerLongPress create fail.");
    }

    ret = os_timer_create(&xTimerAdvCtl, "xTimerAdvCtl", 0, 60000, false,
                          pTimerAdvCtlCallback);
    if (!ret)
    {
        APP_PRINT_ERROR0("xTimerAdvCtl create fail.");
    }

    ret = os_timer_create(&xTimerFlash, "xTimerFlash", 0, 1000, true, pTimerFlashCallback);
    if (!ret)
    {
        APP_PRINT_ERROR0("xTimerFlash create fail.");
    }
}

void start_adv_ctl(void)
{
    APP_PRINT_INFO0("ADV timer started");
    os_timer_start(&xTimerAdvCtl);
}

void start_led_flash(void)
{
    allowedEnterDlps = false;
    os_timer_start(&xTimerFlash);
}

void stop_led_flash(void)
{
    allowedEnterDlps = true;
    os_timer_stop(&xTimerFlash);
    GPIO_ResetBits(GPIO_LED_PIN);
}
