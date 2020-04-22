#include <stdint.h>
#include <stdbool.h>

extern bool allowedEnterDlps;

typedef enum
{
    MSG_BUTTON_SHORT_PRESS,
    MSG_BUTTON_LONG_PRESS
} T_BUTTON_PRESS_EVENT;

extern uint8_t keystatus;
void GPIO_LPN_Handler(void);
void gpio_config_init(void);
void start_adv_ctl(void);
void sw_timer_init(void);
void start_led_flash(void);
void stop_led_flash(void);
