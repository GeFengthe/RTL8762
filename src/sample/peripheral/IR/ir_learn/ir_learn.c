/**
*********************************************************************************************************
*               Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* @file     ir_learn.c
* @brief    This file provides ir learn driver.
* @details
* @author   elliot chen
* @date     2017-11-16
* @version  v1.0
*********************************************************************************************************
*/

/* Includes --------------------------------------------------------------------*/
#include "ir_learn.h"
#include "string.h"
#include "board.h"

#include "app_section.h"
#include "mem_config.h"


#if (IR_FUN && IR_LEARN_MODE)
/* Internal define -------------------------------------------------------------*/

/* Filter threshold value. If time interval< 200us(10KHz), treat it as a part of a carrier time */
/* IR learn carrier freqency between 10KHz and 2.5MHz */
#define TIME_HIGHEST_VALUE          (200*IR_LEARN_FREQ/1000)

/* IR data mask */
#define IR_DATA_MSK                 ((uint32_t)0x7FFFFFFFUL)
/* Carrier waveform data type select */
#define IR_CARRIER_DATA_TYPE        ((uint32_t)0x80000000UL)

/* Global variable  ------------------------------------------------------------*/

/**
  * @brief close the IR learn function.
  * @param  none.
  * @retval None
  */
void IR_Learn_DeInit(void)
{
    DataTrans_DeInit();
}

/**
  * @brief  Initializes some peripherals which used to learn waveform.
  * @param  none.
  * @retval None
  */
void IR_Learn_Init(void)
{
    /* Initialize data transmission layer */
    DataTrans_Init();
}

#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
/**
* @brief  learing a specific IR waveform duty cycle.
* @param  pIR_Packet: point to IR packet struct.
* @retval The decoded status.
*/
IR_Learn_Status IR_Learn_DecodeDutycycle(IR_LearnTypeDef *pIR_Packet)
{
    uint16_t i = 0;
    LoopQueue_BufType buf[IR_LEARN_DUTY_CYCLE_SAMPLE_MAX_SIZE] = {0};
    LoopQueue_BufType carrier_high_time = 0;
    LoopQueue_BufType carrier_low_time = 0;

    if (DataTrans_GetFIFOLen() >= IR_LEARN_DUTY_CYCLE_SAMPLE_MAX_SIZE)
    {
        DataTrans_CopyBuf(buf, IR_LEARN_DUTY_CYCLE_SAMPLE_MAX_SIZE);
    }
    else
    {
        return IR_LEARN_DUTY_CYCLE_ENERR_NO_SAMPLE;
    }

    for (i = 0; i < (IR_LEARN_DUTY_CYCLE_SAMPLE_MAX_SIZE - 1); i++)
    {
        if (((buf[i] & IR_DATA_MSK) < TIME_HIGHEST_VALUE) &&
            ((buf[i + 1] & IR_DATA_MSK) < TIME_HIGHEST_VALUE))
        {
            if (buf[i] & IR_CARRIER_DATA_TYPE)
            {
                carrier_high_time = buf[i] & IR_DATA_MSK;
                carrier_low_time = buf[i + 1] & IR_DATA_MSK;
            }
            else
            {
                carrier_high_time = buf[i + 1] & IR_DATA_MSK;
                carrier_low_time = buf[i] & IR_DATA_MSK;
            }
            break;
        }
    }

    if (carrier_high_time && carrier_low_time)
    {
        pIR_Packet->duty_cycle = (carrier_high_time + carrier_low_time) / carrier_high_time;
        pIR_Packet->carrier_time = carrier_high_time + carrier_low_time;
        return IR_LEARN_OK;
    }

    return IR_LEARN_DUTY_CYCLE_ERR_NO_valid_DATA;
}

/**
* @brief  Process IR waveform.
* @param  pIR_Packet: point to IR packet struct.
* @retval The reduction status.
*/
void IR_Learn_DataCompensation(IR_LearnTypeDef *pIR_Packet)
{
    uint16_t i = 0;

    if ((pIR_Packet->freq == 0) || (pIR_Packet->duty_cycle == 0))
    {
        return ;
    }

    for (i = 0; i <= pIR_Packet->buf_index; i++)
    {
        pIR_Packet->ir_buf[i] += pIR_Packet->carrier_time;
    }
}

/**
* @brief  Convert  IR learned waveform data to actual IR data which can be sent.
* @param  pIR_Packet: point to IR packet struct.
* @retval The reduction status.
*/
void IR_Learn_ConvertData(IR_LearnTypeDef *pIR_Packet)
{
    uint16_t i = 0;
    if ((pIR_Packet->freq == 0) || (pIR_Packet->duty_cycle == 0))
    {
        return ;
    }
    //float data_array[IR_LEARN_WAVE_MAX_SIZE];

    for (i = 0; i <= pIR_Packet->buf_index; i++)
    {
        //data_array[i] = (pIR_Packet->ir_buf[i]&IR_DATA_MSK)*pIR_Packet->freq/IR_LEARN_FREQ;

        if (pIR_Packet->ir_buf[i] & IR_CARRIER_DATA_TYPE)
        {
            pIR_Packet->ir_buf[i] = (pIR_Packet->ir_buf[i] & IR_DATA_MSK) * pIR_Packet->freq / IR_LEARN_FREQ;
            pIR_Packet->ir_buf[i] |= IR_CARRIER_DATA_TYPE;
        }
        else
        {
            pIR_Packet->ir_buf[i] = (pIR_Packet->ir_buf[i] & IR_DATA_MSK) * pIR_Packet->freq / IR_LEARN_FREQ;
        }

#ifdef SOFTWARE_ADJUSTMENT
        if ((pIR_Packet->ir_buf[i] & IR_DATA_MSK) > 0)
        {
            pIR_Packet->ir_buf[i] -= 1;
        }
#endif
    }
}

#endif

/**
* @brief  learing a specific IR waveform freqency.
* @param  pIR_Packet: point to IR packet struct.
* @retval The decoded status.
*/
IR_Learn_Status IR_Learn_Freq(IR_LearnTypeDef *pIR_Packet)
{
    uint16_t i = 0;
    uint16_t j = 0;
    float freq_sum = 0;
#ifdef FILTER_IR_LEARN_FREQ
    float max_freq = 0;
    float min_freq = 1000.0;
    float temp = 0;
#endif
    //float freq_array[IR_LEARN_WAVE_MAX_SIZE/2 + 2] = {0};

    for (i = 0, j = 0; (i <= pIR_Packet->buf_index) && (j <= pIR_Packet->carrier_info_idx); i++)
    {
        //freq_array[j] = (pIR_Packet->carrier_info_buf[j] * (IR_LEARN_FREQ*0.5))/(pIR_Packet->ir_buf[i] & IR_DATA_MSK);
        if (pIR_Packet->ir_buf[i] & IR_CARRIER_DATA_TYPE)
        {
            temp = (pIR_Packet->carrier_info_buf[j++] * (IR_LEARN_FREQ * 0.5)) / \
                   (pIR_Packet->ir_buf[i] & IR_DATA_MSK);

            freq_sum += temp;

#ifdef FILTER_IR_LEARN_FREQ
            if (max_freq < temp)
            {
                max_freq = temp;
            }

            if (min_freq > temp)
            {
                min_freq = temp;
            }
#endif
        }
    }

#ifdef FILTER_IR_LEARN_FREQ

    if (j > 2)
    {
        freq_sum -= (max_freq + min_freq);
        j -= 2;
    }
#endif

    if (j)
    {
        pIR_Packet->freq = freq_sum / j;
    }

    return IR_LEARN_OK;
}

/**
* @brief  learing a specific IR waveform.
* @param  pIR_Packet: point to IR packet struct.
* @retval The decoded status.
*/
IR_Learn_Status IR_Learn_Decode(IR_LearnTypeDef *pIR_Packet)
{
    LoopQueue_BufType time_interval = 0;
    IR_Learn_Status status = IR_LEARN_OK;

#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
    if (!pIR_Packet->duty_cycle)
    {
        status = IR_Learn_DecodeDutycycle(pIR_Packet);
    }
#endif

    while (DataTrans_GetFIFOLen())
    {
        /* Extract data */
        DataTrans_ReadBuf(&time_interval, 1);
        time_interval &= IR_DATA_MSK;

        /* Record total time of carrier wave */
        if (time_interval < TIME_HIGHEST_VALUE)
        {
            /* Record carrier waveform time */
            pIR_Packet->ir_buf[pIR_Packet->buf_index] += time_interval;

            /* Record carrier number */
            pIR_Packet->carrier_info_buf[pIR_Packet->carrier_info_idx]++;

            /* Record data type */
            pIR_Packet->is_carrier = true;
#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
            pIR_Packet->last_handle_data = time_interval;
#endif
        }
        else
        {
            if (pIR_Packet->is_carrier == true)
            {
#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
                if (pIR_Packet->duty_cycle)
                {
                    if (pIR_Packet->last_handle_data < TIME_HIGHEST_VALUE)
                    {
                        pIR_Packet->ir_buf[pIR_Packet->buf_index] += pIR_Packet->carrier_time -
                                                                     pIR_Packet->last_handle_data;
                    }
                }
                else
                {
                    DATATRANS_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "Warning: IR carrier data compensation handle error!",
                                         0);
                    status =  IR_LEARN_CARRIRE_DATA_HANDLE_ERR;
                }
#endif
                /* Restore data type */
                pIR_Packet->ir_buf[pIR_Packet->buf_index] |= IR_CARRIER_DATA_TYPE;
                /* pointer to next buffer to store data */
                pIR_Packet->buf_index++;
#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
                if (pIR_Packet->duty_cycle)
                {
                    /* Store value of low waveform */
                    pIR_Packet->ir_buf[pIR_Packet->buf_index++] = time_interval + pIR_Packet->last_handle_data - \
                                                                  pIR_Packet->carrier_time;
                }
                else
                {
                    /* Store value of low waveform */
                    pIR_Packet->ir_buf[pIR_Packet->buf_index++] = time_interval;
                    DATATRANS_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
                                         "Warning: IR no carrier data compensation handle error!", 0);
                    status = IR_LEARN_NO_CARRIRE_DATA_HANDLE_ERR;
                }
#else
                /* Store value of low waveform */
                pIR_Packet->ir_buf[pIR_Packet->buf_index++] = time_interval;
#endif
                /* Record new carrier waveform information */
                pIR_Packet->carrier_info_idx++;
                /* Record data type */
                pIR_Packet->is_carrier = false;
            }
            else
            {
                /* Store value of low waveform */
                //pIR_Packet->ir_buf[pIR_Packet->buf_index] += time_interval;
                DATATRANS_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "Warning: exceed maximum stop signal value!", 0);
                status = IR_LEARN_WAVEFORM_ERR;
                break;
            }
        }

        /* Check the maximum number of learning data */
        if (pIR_Packet->buf_index >= IR_LEARN_WAVE_MAX_SIZE)
        {
            status = IR_LEARN_EXCEED_SIZE;
            break;
        }

        /* Check IR end signal */
        if (time_interval >= IR_LEARN_STOP_TIME)
        {
            status = IR_LEARN_EXIT;
            break;
        }
    }

    return status;
}

#endif /*end Micro @IR_FUN && IR_LEARN_MODE*/

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

