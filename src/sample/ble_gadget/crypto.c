#include "crypto.h"
#include "sha256.h"
#include "string.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CAL_SHA256_MAX_BUFFER           256

void xComputeSHA256Hash(void *pvMessage,
                        size_t ulMessageSize,
                        void *pvHashDigest,
                        size_t *pulDigestSize)
{
    SHA256_CTX ctx;
    uint8_t pulMemBuffer[CAL_SHA256_MAX_BUFFER];
    uint32_t ulConsumed = 0;

    SHA256_Init(&ctx);

    while (ulConsumed < ulMessageSize)
    {
        uint32_t ulToConsume = MIN(ulMessageSize - ulConsumed, CAL_SHA256_MAX_BUFFER);
        memcpy(pulMemBuffer, (uint8_t *)pvMessage + ulConsumed, ulToConsume);
        SHA256_Update(&ctx, pulMemBuffer, ulToConsume);
        ulConsumed += ulToConsume;
    }

    SHA256_Final(&ctx, pvHashDigest);
}

