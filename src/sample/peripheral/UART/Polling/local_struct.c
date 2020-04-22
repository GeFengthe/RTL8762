#include "board.h"
#include "local_struct.h"
#include "string.h"
#include "trace.h"



void InitLoopBuf(LoopBuf_TypeDef *LoopBufStruct, uint8_t *startAddr, uint16_t buf_size)
{
    LoopBufStruct->emptyFlag  = false;
    LoopBufStruct->ImageReceiveOffset = 0;
    LoopBufStruct->ImageSendOffset = 0;
    LoopBufStruct->overFlow = 0;
    LoopBufStruct->readIndex = 0;
    LoopBufStruct->writeIndex = 0;
    LoopBufStruct->maxSize = buf_size;
    LoopBufStruct->pBuf = startAddr;
    memset(startAddr, 0, buf_size);
}
bool PushLoopBuf(LoopBuf_TypeDef *LoopBufStruct, uint16_t length, uint8_t *pValue)
{
    uint16_t tail_len = 0;

    if (IsLoopBufOverFlow(LoopBufStruct, length) == false)
    {
        if (LoopBufStruct->writeIndex + length <= LoopBufStruct->maxSize)
        {
            memcpy(LoopBufStruct->pBuf + LoopBufStruct->writeIndex, pValue, length);
            LoopBufStruct->writeIndex += length;
        }
        else
        {
            tail_len = LoopBufStruct->maxSize - LoopBufStruct->writeIndex;
            memcpy(LoopBufStruct->pBuf + LoopBufStruct->writeIndex, pValue, tail_len);
            LoopBufStruct->writeIndex = 0;
            memcpy(LoopBufStruct->pBuf + LoopBufStruct->writeIndex, pValue + tail_len, length - tail_len);
            LoopBufStruct->writeIndex += (length - tail_len);
        }
        LoopBufStruct->writeIndex &= (LoopBufStruct->maxSize - 1);
        return true;
    }
    else
    {
        /* Discard data */
        APP_PRINT_WARN0("Loop Queue OverFlow!!!!");
        return false;
    }
}
bool PopLoopBufWithoutRemove(LoopBuf_TypeDef *LoopBufStruct, uint16_t length, uint8_t *pValue)
{
    uint16_t remain = 0;
    uint16_t recordReadIndex = LoopBufStruct->readIndex;
    /* Check parameters */
    if ((length == 0) || (pValue == NULL))
    {
        APP_PRINT_WARN0("Loop Buufer read length is zero line = %d!!!!");
        return true;
    }
    if (pValue == NULL)
    {
        return false;
    }
    if (IsLoopBufEmpty(LoopBufStruct, length) == false)
    {
        if (LoopBufStruct->readIndex + length <= LoopBufStruct->maxSize)
        {
            memcpy(pValue, &(LoopBufStruct->pBuf[LoopBufStruct->readIndex]), length);
            LoopBufStruct->readIndex += length;
        }
        else
        {
            remain = LoopBufStruct->maxSize - LoopBufStruct->readIndex;
            memcpy(pValue, &(LoopBufStruct->pBuf[LoopBufStruct->readIndex]), remain);
            LoopBufStruct->readIndex = 0;
            memcpy(pValue + remain, &(LoopBufStruct->pBuf[LoopBufStruct->readIndex]), length - remain);
        }
        LoopBufStruct->readIndex &= (LoopBufStruct->maxSize - 1);
        LoopBufStruct->readIndex = recordReadIndex;
        return true;
    }
    else
    {
        return false;
    }
}
bool PopLoopBuf(LoopBuf_TypeDef *LoopBufStruct, uint16_t length, uint8_t *pValue)
{
    uint16_t remain = 0;

    /* Check parameters */
    if ((length == 0) || (pValue == NULL))
    {
        APP_PRINT_WARN0("Loop Buufer read length is zero!");
        return true;
    }
    if (pValue == NULL)
    {
        return false;
    }
    if (IsLoopBufEmpty(LoopBufStruct, length) == false)
    {
        if (LoopBufStruct->readIndex + length <= LoopBufStruct->maxSize)
        {
            memcpy(pValue, &(LoopBufStruct->pBuf[LoopBufStruct->readIndex]), length);
            LoopBufStruct->readIndex += length;
        }
        else
        {
            remain = LoopBufStruct->maxSize - LoopBufStruct->readIndex;
            memcpy(pValue, &(LoopBufStruct->pBuf[LoopBufStruct->readIndex]), remain);
            LoopBufStruct->readIndex = 0;
            memcpy(pValue + remain, &(LoopBufStruct->pBuf[LoopBufStruct->readIndex]), length - remain);
            LoopBufStruct->readIndex += (length - remain);
        }
        LoopBufStruct->readIndex &= (LoopBufStruct->maxSize - 1);
        return true;
    }
    else
    {
        return false;
    }
}
uint16_t GetLoopBufDataIndex(LoopBuf_TypeDef *LoopBufStruct, uint8_t data, uint8_t find_data_index,
                             uint16_t length)
{
    uint16_t pos = 0;
    if (IsLoopBufEmpty(LoopBufStruct, length) == false)
    {
        while (length--)
        {
            if (data == LoopBufStruct->pBuf[(LoopBufStruct->readIndex + pos++) & (LoopBufStruct->maxSize - 1)])
            {
                find_data_index--;
                if (find_data_index == 0)
                {
                    return pos;
                }
            }
        }
        return false;
    }
    else
    {
        return false;
    }
}
uint16_t GetLoopBufLength(LoopBuf_TypeDef *LoopBufStruct)
{
    if (LoopBufStruct->readIndex > LoopBufStruct->writeIndex)
    {
        return (LoopBufStruct->writeIndex + LoopBufStruct->maxSize - LoopBufStruct->readIndex);
    }
    else
    {
        return (LoopBufStruct->writeIndex - LoopBufStruct->readIndex);
    }
}

bool IsLoopBufEmpty(LoopBuf_TypeDef *LoopBufStruct, uint16_t read_size)
{
    if (LoopBufStruct->readIndex <= LoopBufStruct->writeIndex)
    {
        if ((LoopBufStruct->readIndex + read_size) > LoopBufStruct->writeIndex)
        {
            APP_PRINT_WARN0("Loop Queue has empty!");
            LoopBufStruct->emptyFlag = true;
            return true;
        }
    }
    else
    {
        if ((LoopBufStruct->readIndex + read_size) > (LoopBufStruct->maxSize + LoopBufStruct->writeIndex))
        {
            APP_PRINT_WARN0("Loop Queue has empty!");
            LoopBufStruct->emptyFlag = true;
            return true;
        }
    }
    return false;
}
bool IsLoopBufOverFlow(LoopBuf_TypeDef *LoopBufStruct, uint16_t write_size)
{
    /* Check queue status */
    if (LoopBufStruct->writeIndex >= LoopBufStruct->readIndex)
    {
        if ((LoopBufStruct->writeIndex + write_size) >= (LoopBufStruct->maxSize + LoopBufStruct->readIndex))
        {
            APP_PRINT_WARN0("Loop Queue will OverFlow!");
            LoopBufStruct->overFlow = true;
            return true;
        }
    }
    else
    {
        if ((LoopBufStruct->writeIndex + write_size) >= LoopBufStruct->readIndex)
        {
            APP_PRINT_WARN0("Loop Queue will OverFlow!");
            LoopBufStruct->overFlow = true;
            return true;
        }
    }
    return false;
}




