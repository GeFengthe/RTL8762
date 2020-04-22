#ifndef __LOCAL_STRUCT_H__
#define __LOCAL_STRUCT_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "board.h"
#include "rtl876x_pinmux.h"
typedef struct
{
    volatile uint16_t   readIndex;      /* index of read queue */
    volatile uint16_t   writeIndex;     /* index of write queue */
    volatile bool       overFlow;           /* loop queue overflow or not */
    volatile bool       emptyFlag;
    volatile uint16_t       maxSize;
    uint8_t *pBuf;      /* Buffer for loop queue */
    uint32_t ImageReceiveOffset;
    uint32_t ImageSendOffset;
} LoopBuf_TypeDef;


void InitLoopBuf(LoopBuf_TypeDef *LoopBufStruct, uint8_t *startAddr, uint16_t buf_size);
bool PushLoopBuf(LoopBuf_TypeDef *LoopBufStruct, uint16_t length, uint8_t *pValue);
bool PopLoopBuf(LoopBuf_TypeDef *LoopBufStruct, uint16_t length, uint8_t *pValue);
bool PopLoopBufWithoutRemove(LoopBuf_TypeDef *LoopBufStruct, uint16_t length, uint8_t *pValue);
uint16_t GetLoopBufLength(LoopBuf_TypeDef *LoopBufStruct);
bool IsLoopBufOverFlow(LoopBuf_TypeDef *LoopBufStruct, uint16_t write_size);
bool IsLoopBufEmpty(LoopBuf_TypeDef *LoopBufStruct, uint16_t read_size);
uint16_t GetLoopBufDataIndex(LoopBuf_TypeDef *LoopBufStruct, uint8_t data, uint8_t find_data_index,
                             uint16_t length);




#ifdef __cplusplus
}
#endif



#endif

