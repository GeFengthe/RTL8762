
#ifndef _LED_H_
#define _LED_H_

#include "stdint.h"
#include "board.h"

/**
*   @brief user guide for led driver
*   The driver support multi prio, multi event, and multi led.
*   If you want to control a led with a event as you designed, there is two way to
*   achieve this, modify the exist event and redefine a new event.
*
*   The led driver is drived by software timer, the timer peroid is 50ms by default.
*
*   #define  LED_PERIOD   50 //you an change the value according your requirement
*
*   For the whole led driver system, there is 5 place you might modify.
*
*   1. Define led num and led pin, which is in file board.h;
*      For example, define 2 leds as followed:
*      #define  LED_NUM_MAX   0x02
*      #define  LED_INDEX(n)   (n<<8)

*      //uint16_t, first byte led index, last byte led pin
*      #define  LED_1         (LED_INDEX(0) | P2_2)
*      #define  LED_2         (LED_INDEX(1) | P0_2)
*
*   2. Define led type index, which is in file led_driver.h, these types must be
*      defined in sequence.
*      For example,
*      #define  LED_TYPE_BLINK_OTA_UPDATE       (8)
*
*   3. Define led loop bits num, in file led_driver.h
*      //max value 32, min value 1, it indicate that there is how many bits to loop
*      //from LSB
*      #define LED_LOOP_BITS_OTA_UPDATE          (15)
*
*   4. Define led event bit map, in file led_driver.h
*      // it must be cooperated with led loop bits num
*      #define  LED_BIT_MAP_OTA_UPDATE       (0x4210)//on 50ms, off 200ms, period 50ms
*
*   5. Update led event table, led_event_arr[LED_TYPE_MAX], in file led_driver.c
*      Before you use the event you define ,you need to add led type, led loop bit num,
*      and led event bit map into event table.
*      const led_event_stg led_event_arr[LED_TYPE_MAX] =
*      {
*                ··· ···
*        {LED_TYPE_BLINK_OTA_UPDATE, LED_LOOP_BITS_OTA_UPDATE,  LED_BIT_MAP_OTA_UPDATE},
*      };
*
*   There are three interfaces for Led driver, as follow.
*
*   void led_module_init(void); // called when system boot;
*   LED_RET_CAUSE led_blink_start(uint16_t index, LED_TYPE type, uint8_t cnt);
*   LED_RET_CAUSE led_blink_exit(uint16_t index, LED_TYPE type);
*/

/*software timer period*/
#define  LED_PERIOD   50  /*50ms*/
#define  LED_DEBUG     0

/*led event types, the less value, the higher prio of the event*/
typedef   uint32_t      LED_TYPE   ;
#define   LED_TYPE_IDLE                            0
#define   LED_TYPE_BLINK_LOW_POWER                (1)
#define   LED_TYPE_BLINK_IR_LEARN_SUCCESS         (2)
#define   LED_TYPE_BLINK_IR_LEARN_WAITING         (3)
#define   LED_TYPE_BLINK_IR_LEARN_MODE            (4)
#define   LED_TYPE_BLINK_OTA_FAIL                 (5)
#define   LED_TYPE_BLINK_PAIR_SUCCESS             (6)
#define   LED_TYPE_ON                             (7)
#define   LED_TYPE_MAX                             8

/*led loop bit num for each event, max value 32*/
#define   LED_LOOP_BITS_IDLE                     (32)
#define   LED_LOOP_BITS_LOW_POWER                (32)
#define   LED_LOOP_BITS_IR_LEARN_SUCCESS         (32)
#define   LED_LOOP_BITS_IR_LEARN_WAITING         (32)
#define   LED_LOOP_BITS_IR_LEARN_MODE            (32)
#define   LED_LOOP_BITS_OTA_FAIL                 (32)
#define   LED_LOOP_BITS_PAIR_SUCCESS             (32)
#define   LED_LOOP_BITS_ON                       (32)

/*led bit map 32bits, High bits(low priority) ---  Low bits(high priority) */
#define LED_BIT_MAP_INVALID               (0x00000000)
#define LED_BIT_MAP_LOW_POWER             (0x00000001)
#define LED_BIT_MAP_IR_LEARN_SUCCESS      (0x0f0f0f0f)
#define LED_BIT_MAP_IR_LEARN_WAITING      (0xffffffff)
#define LED_BIT_MAP_IR_LEARN_MODE         (0x00FF00FF)
#define LED_BIT_MAP_OTA_FAIL              (0x00050005)
#define LED_BIT_MAP_PAIR_SUCCESS          (0x03030303)
#define LED_BIT_MAP_ON                    (0xffffffff)

typedef struct
{
    uint8_t   led_type_index;
    uint8_t   led_loop_cnt;
    uint32_t  led_bit_map;
} led_event_stg;

/*struct support for led blink count*/
typedef struct
{
    uint8_t led_loop_cnt;
    uint8_t cur_tick_cnt;
} led_cnt_stg;

/*support for each led data*/
typedef struct
{
    uint8_t led_index;
    uint8_t led_pin;
    led_cnt_stg led_cnt_arr[LED_TYPE_MAX];
    uint32_t led_map;
} led_data_stg;

/*led return code*/
typedef enum
{
    LED_SUCCESS                      = 0,
    LED_ERR_TYPE                     = 1,
    LED_ERR_INDEX                    = 2,
    LED_ERR_CODE_MAX
} LED_RET_CAUSE;

/*============================================================================*
 *                        <Led Module Interface>
 *============================================================================*/
extern void led_module_init(void);
extern LED_RET_CAUSE led_blink_start(uint16_t index, LED_TYPE type, uint8_t cnt);
extern LED_RET_CAUSE led_blink_exit(uint16_t index, LED_TYPE type);

#if LED_EN
#define LED_ON(index)                   led_blink_start(index, LED_TYPE_ON, 0)
#define LED_OFF(index)                  led_blink_exit(index, LED_TYPE_ON)
#define LED_BLINK(index, type, n)       led_blink_start(index, type, n)
#define LED_BLINK_EXIT(index, type)     led_blink_exit(index, type)
#else
#define LED_ON(index)
#define LED_OFF(index)
#define LED_BLINK(index, type, n)
#define LED_BLINK_EXIT(index, type)
#endif

#endif
