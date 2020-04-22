#ifndef _AUTO_K_RF_H_
#define _AUTO_K_RF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define FLT_MAX 40000

#define K_RF_SUCCESS               0
#define K_RF_READ_DRIFT_FAIL       1
#define K_RF_K_FAIL                2
#define K_RF_WRITE_CONFIG_FAIL     3

uint8_t Auto_K_RF(uint8_t rx_channel, uint32_t freq_drift_threshold, uint8_t *cal_xtal_result);

#ifdef __cplusplus
}
#endif

#endif /* _AUTO_K_RF_H_ */
