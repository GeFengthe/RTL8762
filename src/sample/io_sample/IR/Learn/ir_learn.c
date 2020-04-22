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

#if (IR_FUN_EN && IR_FUN_LEARN_EN)

/**
  * @brief  Initialization of pinmux settings and pad settings.
  * @param  No parameter.
  * @return void
  */
void board_ir_learn_init(void)
{
    board_ir_rx_init();
}

/**
  * @brief  Close the IR learn function.
  * @param  none.
  * @retval None
  */
void ir_learn_deinit(void)
{
    ir_trans_rx_deinit();
}

/**
  * @brief  Initializes some peripherals which used to learn waveform.
  * @param  none.
  * @retval None
  */
void ir_learn_init(void)
{
    ir_trans_rx_init();
}

#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
/**
  * @brief  Learn a specific IR waveform duty cycle.
  * @param  pIR_Packet: point to IR packet struct.
  * @retval The decoded status.
  */
static IR_Learn_Status ir_learn_dutycycle(IR_Learn_TypeDef *pIR_Packet)
{
    uint16_t i = 0;
    uint32_t buf[IR_LEARN_DUTY_CYCLE_SAMPLE_SIZE_MAX] = {0};
    uint32_t carrier_high_time = 0;
    uint32_t carrier_low_time = 0;

    /* Replicate data used to calculate duty cycle from the IR receiving data queue. */
    if (ir_trans_rx_get_data_len() >= IR_LEARN_DUTY_CYCLE_SAMPLE_SIZE_MAX)
    {
        ir_trans_rx_copy_data(buf, IR_LEARN_DUTY_CYCLE_SAMPLE_SIZE_MAX);
    }
    else
    {
        return IR_LEARN_DUTY_CYCLE_ERR_NO_SAMPLE;
    }

    for (i = 0; i < (IR_LEARN_DUTY_CYCLE_SAMPLE_SIZE_MAX - 1); i++)
    {
        if (((buf[i] & IR_DATA_MSK) < TIME_HIGHEST_VALUE) &&
            ((buf[i + 1] & IR_DATA_MSK) < TIME_HIGHEST_VALUE))
        {
            /* Check if there is a carrier waveform. */
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
        pIR_Packet->carrier_time = carrier_high_time + carrier_low_time;
        pIR_Packet->duty_cycle = (float)carrier_high_time / ((float)carrier_high_time +
                                                             (float)carrier_low_time);
        return IR_LEARN_OK;
    }

    return IR_LEARN_DUTY_CYCLE_ERR_NO_VALID_DATA;
}

/**
  * @brief  IR data compensation.
  * @param  pIR_Packet: point to ir packet struct.
  * @retval void
  */
void ir_learn_data_compensation(IR_Learn_TypeDef *pIR_Packet)
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

#endif


/**
  * @brief  Learn a specific ir waveform freqency.
  * @param  pIR_Packet: point to ir packet struct.
  * @retval The decoded status.
  */
IR_Learn_Status ir_learn_freq(IR_Learn_TypeDef *pIR_Packet)
{
    uint16_t i = 0;
    uint16_t j = 0;
    float freq_sum = 0;
#if IR_LEARN_FREQ_FILTER_EN
    float max_freq = 0;
    float min_freq = 1000.0;
    float temp = 0;
#endif

    for (i = 0, j = 0; (i <= pIR_Packet->buf_index) && (j <= pIR_Packet->carrier_info_idx); i++)
    {
        if (pIR_Packet->ir_buf[i] & IR_CARRIER_DATA_TYPE)
        {
            temp = (pIR_Packet->carrier_info_buf[j++] * (IR_LEARN_FREQ * 0.5)) / \
                   (pIR_Packet->ir_buf[i] & IR_DATA_MSK);

            freq_sum += temp;

#if IR_LEARN_FREQ_FILTER_EN
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

#if IR_LEARN_FREQ_FILTER_EN
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
  * @brief  Convert ir learned waveform data to actual ir data which can be sent.
  * @param  pIR_Packet: point to ir packet struct.
  * @retval void
  */
void ir_learn_data_convert(IR_Learn_TypeDef *pIR_Packet)
{
    uint16_t i = 0;

#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
    if ((pIR_Packet->freq == 0) || (pIR_Packet->duty_cycle == 0))
    {
        return ;
    }
#else
    if (pIR_Packet->freq == 0)
    {
        return ;
    }
#endif

    for (i = 0; i <= pIR_Packet->buf_index; i++)
    {
        if (pIR_Packet->ir_buf[i] & IR_CARRIER_DATA_TYPE)
        {
            pIR_Packet->ir_buf[i] = (pIR_Packet->ir_buf[i] & IR_DATA_MSK) * pIR_Packet->freq / IR_LEARN_FREQ;
            pIR_Packet->ir_buf[i] |= IR_CARRIER_DATA_TYPE;
        }
        else
        {
            pIR_Packet->ir_buf[i] = (pIR_Packet->ir_buf[i] & IR_DATA_MSK) * pIR_Packet->freq / IR_LEARN_FREQ;
        }

#if IR_LEARN_SOFTWARE_ADJUST_EN
        if ((pIR_Packet->ir_buf[i] & IR_DATA_MSK) > 0)
        {
            pIR_Packet->ir_buf[i] -= 1;
        }
#endif
    }
}

/**
  * @brief  IR learn data decode
  * @param  pIR_Packet: point to IR packet struct.
  * @retval The decoded status.
  */
IR_Learn_Status ir_learn_decode(IR_Learn_TypeDef *pIR_Packet)
{
    uint32_t time_interval = 0;
    IR_Learn_Status status = IR_LEARN_OK;

#ifdef IR_LEARN_DUTY_CYCLE_SUPPORT
    if (!pIR_Packet->duty_cycle)
    {
        status = ir_learn_dutycycle(pIR_Packet);
    }
#endif

    while (ir_trans_rx_get_data_len())
    {
        /* Extract data */
        ir_trans_rx_read_data(&time_interval, 1);
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
                    IR_RX_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "Warning: IR carrier data compensation handle error!",
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
                    IR_RX_DBG_BUFFER(MODULE_APP, LEVEL_INFO,
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
                IR_RX_DBG_BUFFER(MODULE_APP, LEVEL_INFO, "Warning: exceed maximum stop signal value!", 0);
                status = IR_LEARN_WAVEFORM_ERR;
                break;
            }
        }

        /* Check the maximum number of learning data */
        if (pIR_Packet->buf_index >= IR_LEARN_WAVEFORM_SIZE_MAX)
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

#endif /* (IR_FUN_EN && IR_FUN_LEARN_EN) */

/******************* (C) COPYRIGHT 2017 Realtek Semiconductor Corporation *****END OF FILE****/

