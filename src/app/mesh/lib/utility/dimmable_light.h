/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     dimmable_light.h
  * @brief    Head file for dimmable light.
  * @details  Data structs and external functions declaration.
  * @author   bill
  * @date     2015-11-03
  * @version  v0.1
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _DIMMABLE_LIGHT_H_
#define _DIMMABLE_LIGHT_H_

#include "platform_types.h"
#include "board.h"
#include "rtl876x_tim.h"

BEGIN_DECLS

/**
 * @addtogroup DIMMABLE_LIGHT
 * @{
 */

/**
 * @defgroup Dimmable_Light_Exported_Types Dimmable Light Exported Types
 * @brief
 * @{
 */
typedef enum
{
    LIGHT_FLASH_PARAM_TYPE_POWER_ON_COUNT,
    LIGHT_FLASH_PARAM_TYPE_LIGHT_STATE,
    LIGHT_FLASH_PARAM_TYPE_USER_DATA,
} light_flash_param_type_t;

typedef struct
{
    uint8_t count;
    uint8_t padding[3];
} light_flash_power_on_count_t;

typedef struct
{
    uint16_t state[5];
    uint8_t padding[2];
} light_flash_light_state_t;

typedef struct
{
    light_flash_power_on_count_t power_on_count;
    light_flash_light_state_t light_state;
    /**
     * user can add some fixed size structure here,
     * size must be 4-byte alignment
     **/

    /** user data must at the end, do not modify */
    void *user_data;
} light_flash_param_t;

/* invalid pin */
#define PIN_INVALID       0xff

typedef struct
{
    uint8_t pin_num;
    uint8_t pin_func;
    TIM_TypeDef *tim_id;
    bool pin_high_on;
    uint16_t lightness;
    uint16_t lightness_last;
} light_t;
/** @} */

/**
 * @defgroup Dimmable_Light_Exported_Macros Dimmable Light Exported Macros
 * @brief
 * @{
 */
/**
 * @brief initialize light driver common part
 */
void light_driver_init(void);

/**
 * @brief config light pin
 * @param[in] light: light handle
 */
void light_pin_config(const light_t *light);

/**
 * @brief config light dlps
 * @param[in] light: light handle
 */
void light_pin_dlps_config(const light_t *light);

/**
 * @brief set light lightness
 * @param[in] light: light handle
 * @param[in] lightness: light lightness
 * @note update lightness file only
 */
void light_lighten(light_t *light, uint16_t lightness);

/**
 * @brief set light lightness
 * @param[in] light: light handle
 * @param[in] lightness: light lightness
 * @note update lightness and lightness_last field
 */
void light_set_lightness(light_t *light, uint16_t lightness);

/**
 * @brief blink light
 * @param[in] light: light handle
 * @param[in] hz_numerator: hz numerator
 * @param[in] hz_denominator: hz denominator
 * @param[in] duty: high duty
 * @note frequency = hz_numerator / hz_denominator Hz, range: 1/13 ~ 2.5K
 */
void light_blink_infinite(light_t *light, uint32_t hz_numerator, uint32_t hz_denominator,
                          uint8_t duty);

/**
 * @brief write light parameter to flash
 * @param[in] type: parameter type
 * @param[in] len: parameter length
 * @param[in] pdata: input parameter data
 * @retval TRUE: store success
 * @retval FALSE: store failed
 */
bool light_flash_write(light_flash_param_type_t type, uint16_t len, void *pdata);

/**
 * @brief read light parameter from flash
 * @param[in] type: parameter type
 * @param[in] len: parameter length
 * @param[out] pdata: output parameter data
 * @retval TRUE: restore success
 * @retval FALSE: restore failed
 */
bool light_flash_read(light_flash_param_type_t type, uint16_t len, void *pdata);

/**
 * @brief check light power on count
 * @param[in] count: power on count
 */
bool light_power_on_count_check(uint8_t count);
/** @} */
/** @} */

END_DECLS

#endif /* _DIMMABLE_LIGHT_H_ */
