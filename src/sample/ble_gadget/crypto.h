#ifndef __CRYPTO_H
#define __CRYPTO_H

#include <stdint.h>
#include <stddef.h>
//#include "sdk_errors.h"

uint8_t xCryptoInit(void);
uint8_t xCryptoUnInit(void);

void xComputeSHA256Hash(void *pvMessage,
                        size_t ulMessageSize,
                        void *pvHashDigest,
                        size_t *pulDigestSize);

#endif /* ifndef __CRYPTO_H */
