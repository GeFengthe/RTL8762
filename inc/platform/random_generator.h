#ifndef _RANDOM_GENERATOR_H_
#define _RANDOM_GENERATOR_H_

#include <stdint.h>
#include <stdbool.h>
#include "rtl876x.h"

/** @defgroup RANDOM_GENERATOR    Random Generator
  * @{
  */

/*============================================================================*
  *                                Variables
  *============================================================================*/
/** @defgroup RANDOM_GENERATOR_Exported_Variables Random Generator Exported Variables
    * @brief
    * @{
    */
extern uint32_t platform_seed;
extern uint32_t platform_poly;
/** @} */ /* End of group RANDOM_GENERATOR_Exported_Variables*/
/*============================================================================*
  *                                Functions
  *============================================================================*/
/** @defgroup RANDOM_GENERATOR_Exported_Functions Random Generator Exported Functions
    * @brief
    * @{
    */
static inline void rand_gen_set_clock_enable(bool enable)
{
    RAN_GEN->u.CTL_BITS.rand_gen_en = enable;
}


static inline void rand_gen_set_polynomial(uint32_t poly)
{
    RAN_GEN->POLYNOMIAL = poly;
}

static inline uint32_t rand_gen_get_polynomial(void)
{
    return RAN_GEN->POLYNOMIAL;
}

static inline void rand_gen_set_seed(uint32_t seed)
{
    RAN_GEN->SEED = seed;
}

static inline uint32_t rand_gen_get_seed(void)
{
    return RAN_GEN->SEED;
}

static inline uint32_t rand_gen_get_random_number(void)
{
    return RAN_GEN->RAN_NUM;
}

static inline void rand_gen_update_seed(bool need_update)
{
    RAN_GEN->u.CTL_BITS.seed_upd = need_update;
}

static inline void rand_gen_update_random(void)
{
    RAN_GEN->u.CTL_BITS.seed_upd = 0;
    RAN_GEN->u.CTL_BITS.opt_rand_upd = 1;
}

static inline void rand_gen_request_random(void)
{
    /*
       fix write random_req register bug:
       rand_gen_en should be set to 1 first before write random_req register,
       and rand_gen_en will be cleared to 0 after read rand_num.
     */
    RAN_GEN->u.CTL_BITS.rand_gen_en = 1;
    RAN_GEN->u.CTL_BITS.seed_upd = 0;
    RAN_GEN->u.CTL_BITS.random_req = 1;
}

static inline void rand_gen_reset_generator(bool is_reset)
{
    RAN_GEN->u.CTL_BITS.soft_rst = is_reset;
}

extern void (*rand_gen_init)(uint32_t seed, uint32_t poly, bool update_seed);
extern uint32_t (*rand_gen_read_random_number)(void);
extern void (*rand_gen_reset)(void);
/** @} */ /* End of group RANDOM_GENERATOR_Exported_Functions*/

/** @} */ /* End of group RANDOM_GENERATOR*/

#endif /* _RANDOM_GENERATOR_H_ */

