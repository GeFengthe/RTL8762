/**
*********************************************************************************************************
*               Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved.
*********************************************************************************************************
* @file      rtl876x_ir.h
* @brief     header file of IR driver.
* @details
* @author    elliot chen
* @date      2016-12-05
* @version   v1.0
* *********************************************************************************************************
*/


#ifndef _RTL876x_IR_H_
#define _RTL876x_IR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"

/* Peripheral: IR */
/* Description: IR register defines */

/* Register: TX_CONFIG -------------------------------------------------------*/
/* Description: IR TX CONFIG register. Offset: 0x04. Address: 0x40003004. */

/* TX_CONFIG[31] :IR_MODE_SEL. 0x1: RX mode. 0x0: TX mode. */
#define IR_MODE_SEL_Pos                 (31UL)
#define IR_MODE_SEL_Msk                 (0x1UL << IR_MODE_SEL_Pos)
#define IR_MODE_SEL_CLR                 (~IR_MODE_SEL_Msk)
/* TX_CONFIG[30] :IR_TX_START. 0x1: Start TX. 0x0: Stop TX. */
#define IR_TX_START_Pos                 (30UL)
#define IR_TX_START_Msk                 (0x1UL << IR_TX_START_Pos)
#define IR_TX_START_CLR                 (~IR_TX_START_Msk)
/* TX_CONFIG[16] :IR_TX_DUTY_NUM. Duty cycle setting for modulation frequency. */
/* Example : for 1/3 duty cycle, IR_DUTY_NUM = (IR_DIV_NUM+1)/3 -1. */
#define IR_TX_DUTY_NUM_Pos              (16UL)
#define IR_TX_DUTY_NUM_Msk              (0xFFFUL << IR_TX_DUTY_NUM_Pos)
/* TX_CONFIG[14] :IR_OUTPUT_INVERSE. 0x1: Inverse  active output. 0x0: Not inverse active output. */
#define IR_OUTPUT_INVERSE_Pos           (14UL)
#define IR_OUTPUT_INVERSE_Msk           (0x1UL << IR_OUTPUT_INVERSE_Pos)
#define IR_OUTPUT_INVERSE_CLR           (~IR_OUTPUT_INVERSE_Msk)
/* TX_CONFIG[13] :IR_FIFO_INVERSE. 0x1: Inverse FIFO define. 0x0: Not inverse FIFO define. */
#define IR_FIFO_INVERSE_Pos             (13UL)
#define IR_FIFO_INVERSE_Msk             (0x1UL << IR_FIFO_INVERSE_Pos)
#define IR_FIFO_INVERSE_CLR             (~IR_FIFO_INVERSE_Msk)
/* TX_CONFIG[8] :IR_TX_FIFO_THRESHOLD. TX FIFO interrupt threshold. when TX FIFO depth <= threshold value, trigger interrupt. */
#define IR_TX_FIFO_THRESHOLD_Pos        (8UL)
#define IR_TX_FIFO_THRESHOLD_Msk        (0x1fUL << IR_TX_FIFO_THRESHOLD_Pos)
#define IR_TX_FIFO_THRESHOLD_CLR        (~IR_TX_FIFO_THRESHOLD_Msk)
/* TX_CONFIG[6] :IR_TX_IDLE_STATE. TX output State in idle. 0x1: High. 0x0: Low. */
#define IR_TX_IDLE_STATE_Pos            (6UL)
#define IR_TX_IDLE_STATE_Msk            (0x1UL << IR_TX_IDLE_STATE_Pos)
#define IR_TX_IDLE_STATE_CLR            (~IR_TX_IDLE_STATE_Msk)
/* TX_CONFIG[5] :IR_TX_FIFO_OVER_INT_MASK. TX FIFO empty Interrupt. 0x1: Mask. 0x0: Unmask. */
#define IR_TX_FIFO_OVER_INT_MASK_Pos    (5UL)
#define IR_TX_FIFO_OVER_INT_MASK_Msk    (0x1UL << IR_TX_FIFO_OVER_INT_MASK_Pos)
#define IR_TX_FIFO_OVER_INT_MASK_CLR    (~IR_TX_FIFO_OVER_INT_MASK_Msk)
/* TX_CONFIG[4] :IR_TX_FIFO_OVER_INT_EN. TX FIFO overflow Interrupt. 0x1: Enable. 0x0: Disable. */
#define IR_TX_FIFO_OVER_INT_EN_Pos      (4UL)
#define IR_TX_FIFO_OVER_INT_EN_Msk      (0x1UL << IR_TX_FIFO_OVER_INT_EN_Pos)
#define IR_TX_FIFO_OVER_INT_EN_CLR      (~IR_TX_FIFO_OVER_INT_EN_Msk)
#define IR_TX_FIFO_OVER_MSK_TO_EN_Pos   (IR_TX_FIFO_OVER_INT_MASK_Pos - IR_TX_FIFO_OVER_INT_EN_Pos)
/* TX_CONFIG[3] :IR_TX_FIFO_LEVEL_INT_MASK. TX FIFO threshold Interrupt. 0x1: Mask. 0x0: Unmask. */
#define IR_TX_FIFO_LEVEL_INT_MASK_Pos   (3UL)
#define IR_TX_FIFO_LEVEL_INT_MASK_Msk   (0x1UL << IR_TX_FIFO_LEVEL_INT_MASK_Pos)
#define IR_TX_FIFO_LEVEL_INT_MASK_CLR   (~IR_TX_FIFO_LEVEL_INT_MASK_Msk)
/* TX_CONFIG[2] :IR_TX_FIFO_EMPTY_INT_MASK. TX FIFO empty Interrupt. 0x1: Mask. 0x0: Unmask. */
#define IR_TX_FIFO_EMPTY_INT_MASK_Pos   (2UL)
#define IR_TX_FIFO_EMPTY_INT_MASK_Msk   (0x1UL << IR_TX_FIFO_EMPTY_INT_MASK_Pos)
#define IR_TX_FIFO_EMPTY_INT_MASK_CLR   (~IR_TX_FIFO_EMPTY_INT_MASK_Msk)
/* TX_CONFIG[1] :IR_TX_FIFO_LEVEL_INT_EN. TX FIFO threshold Interrupt. 0x1: Enable. 0x0: Disable. */
#define IR_TX_FIFO_LEVEL_INT_EN_Pos     (1UL)
#define IR_TX_FIFO_LEVEL_INT_EN_Msk     (0x1UL << IR_TX_FIFO_LEVEL_INT_EN_Pos)
#define IR_TX_FIFO_LEVEL_INT_EN_CLR     (~IR_TX_FIFO_LEVEL_INT_EN_Msk)
/* TX_CONFIG[0] :IR_TX_FIFO_EMPTY_INT_EN. TX FIFO empty Interrupt. 0x1: Enable. 0x0: Disable. */
#define IR_TX_FIFO_EMPTY_INT_EN_Pos     (0UL)
#define IR_TX_FIFO_EMPTY_INT_EN_Msk     (0x1UL << IR_TX_FIFO_EMPTY_INT_EN_Pos)
#define IR_TX_FIFO_EMPTY_INT_EN_CLR     (~IR_TX_FIFO_EMPTY_INT_EN_Msk)
#define IR_TX_MSK_TO_EN_Pos             (IR_TX_FIFO_EMPTY_INT_MASK_Pos - IR_TX_FIFO_EMPTY_INT_EN_Pos)
#define IR_TX_STATUS_TO_EN_Pos          (IR_TX_FIFO_OVER_INT_EN_Pos - IR_TX_FIFO_EMPTY_INT_MASK_Pos)

/* Register: TX_SR -----------------------------------------------------------*/
/* Description: TX_SR register. Offset: 0x08. Address: 0x40003008. */

/* TX_SR[15] :IR_TX_FIFO_EMPTY. 0x1: empty. 0x0: not empty. */
#define IR_TX_FIFO_EMPTY_Pos            (15UL)
#define IR_TX_FIFO_EMPTY_Msk            (0x1UL << IR_TX_FIFO_EMPTY_Pos)
/* TX_SR[14] :IR_TX_FIFO_FULL. 0x1: full. 0x0: not full. */
#define IR_TX_FIFO_FULL_Pos             (14UL)
#define IR_TX_FIFO_FULL_Msk             (0x1UL << IR_TX_FIFO_FULL_Pos)
/* TX_SR[8] :IR_TX_FIFO_OFFSET. */
#define IR_TX_FIFO_OFFSET_Pos           (8UL)
#define IR_TX_FIFO_OFFSET_Msk           (0x3fUL << IR_TX_FIFO_OFFSET_Pos)
/* TX_SR[4] :IR_TX_STATUS. */
#define IR_TX_STATUS_Pos                (4UL)
#define IR_TX_STATUS_Msk                (0x1UL << IR_TX_STATUS_Pos)
/* TX_SR[2] :IR_TX_FIFO_OVER_INT_STATUS. */
#define IR_TX_FIFO_OVER_INT_STATUS_Pos  (2UL)
#define IR_TX_FIFO_OVER_INT_STATUS_Msk  (0x1UL << IR_TX_FIFO_OVER_INT_STATUS_Pos)
/* TX_SR[1] :IR_TX_FIFO_LEVEL_INT_STATUS. */
#define IR_TX_FIFO_LEVEL_INT_STATUS_Pos (1UL)
#define IR_TX_FIFO_LEVEL_INT_STATUS_Msk (0x1UL << IR_TX_FIFO_LEVEL_INT_STATUS_Pos)
/* TX_SR[0] :IR_TX_FIFO_EMPTY_INT_STATUS. */
#define IR_TX_FIFO_EMPTY_INT_STATUS_Pos (0UL)
#define IR_TX_FIFO_EMPTY_INT_STATUS_Msk (0x1UL << IR_TX_FIFO_EMPTY_INT_STATUS_Pos)

/* Register: TX_INT_CLR -----------------------------------------------------------*/
/* Description: TX_INT_CLR register. Offset: 0x10. Address: 0x40003010. */

/* TX_INT_CLR[3] :IR_TX_FIFO_OVER_INT_CLR. Write 1 clear. */
#define IR_TX_FIFO_OVER_INT_CLR_Pos     (3UL)
#define IR_TX_FIFO_OVER_INT_CLR_Msk     (0x1UL << IR_TX_FIFO_OVER_INT_CLR_Pos)
/* TX_INT_CLR[2] :IR_TX_FIFO_LEVEL_INT_CLR. Write 1 clear. */
#define IR_TX_FIFO_LEVEL_INT_CLR_Pos    (2UL)
#define IR_TX_FIFO_LEVEL_INT_CLR_Msk    (0x1UL << IR_TX_FIFO_LEVEL_INT_CLR_Pos)
/* TX_INT_CLR[1] :IR_TX_FIFO_EMPTY_INT_CLR. Write 1 clear. */
#define IR_TX_FIFO_EMPTY_INT_CLR_Pos    (1UL)
#define IR_TX_FIFO_EMPTY_INT_CLR_Msk    (0x1UL << IR_TX_FIFO_EMPTY_INT_CLR_Pos)
/* TX_INT_CLR[0] :IR_TX_FIFO_CLR. Write 1 clear. */
#define IR_TX_FIFO_CLR_Pos              (0UL)
#define IR_TX_FIFO_CLR_Msk              (0x1UL << IR_TX_FIFO_CLR_Pos)

#define IR_INT_ALL_CLR                  (IR_TX_FIFO_OVER_INT_CLR_Msk | \
                                         IR_TX_FIFO_LEVEL_INT_CLR_Msk | \
                                         IR_TX_FIFO_EMPTY_INT_CLR_Msk)

/* Register: TX_FIFO ---------------------------------------------------------*/
/* Description: TX_FIFO register. Offset: 0x14. Address: 0x40003014. */

/* TX_FIFO[31] :IR_DATA_TYPE. 0x1: active carrier. 0x0: incative carrier. */
#define IR_DATA_TYPE_Pos                (31UL)
#define IR_DATA_TYPE_Msk                (0x1UL << IR_DATA_TYPE_Pos)
#define IR_DATA_TYPE_CLR                (~IR_DATA_TYPE_Msk)
/* TX_FIFO[30] :IR_TX_LAST_PACKEET. 0x1: last packet. 0x0: normal packet. */
#define IR_TX_LAST_PACKEET_Pos          (30UL)
#define IR_TX_LAST_PACKEET_Msk          (0x1UL << IR_TX_LAST_PACKEET_Pos)
#define IR_TX_LAST_PACKEET_CLR          (~IR_TX_LAST_PACKEET_Msk)
/* TX_FIFO[29:28] :IR_LOW_PERIOD_COMPENSATION */
#define IR_LOW_PERIOD_COMPENSATION_Pos  (28UL)
#define IR_LOW_PERIOD_COMPENSATION_Msk  (0x3UL << IR_LOW_PERIOD_COMPENSATION_Pos)
#define IR_LOW_PERIOD_COMPENSATION_CLR  (~IR_LOW_PERIOD_COMPENSATION_Msk)

/* Register: RX_CONFIG ------------------------------------------------------*/
/* Description: IR RX CONFIG register. Offset: 0x18. Address: 0x40003018. */

/* RX_CONFIG[28] :IR_RX_START. 0x1: Run. 0x0: Stop. */
#define IR_RX_START_Pos                 (28UL)
#define IR_RX_START_Msk                 (0x1UL << IR_RX_START_Pos)
#define IR_RX_START_CLR                 (~IR_RX_START_Msk)
/* RX_CONFIG[27] :IR_RX_START_MODE. 0x1: auto mode. 0x0: manual mode. */
#define IR_RX_START_MODE_Pos            (27UL)
#define IR_RX_START_MODE_Msk            (0x1UL << IR_RX_START_MODE_Pos)
#define IR_RX_START_MODE_CLR            (~IR_RX_START_MODE_Msk)
/* RX_CONFIG[26] :IR_RX_MAN_START. 0x1: Start check waveform. */
#define IR_RX_MAN_START_Pos             (26UL)
#define IR_RX_MAN_START_Msk             (0x1UL << IR_RX_MAN_START_Pos)
#define IR_RX_MAN_START_CLR             (~IR_RX_MAN_START_Msk)
/* RX_CONFIG[24] :IR_TRIGGER_MODE. 0x1: Run. */
/* 0x0: high->low  trigger. 0x1: low->high trigger. 0x02: high->low  or low->high trigger. */
#define IR_TRIGGER_MODE_Pos             (24UL)
#define IR_TRIGGER_MODE_Msk             (0x3UL << IR_TRIGGER_MODE_Pos)
#define IR_TRIGGER_MODE_CLR             (~IR_TRIGGER_MODE_Msk)
/* RX_CONFIG[21] :IR_FILTER_TIME. */
#define IR_FILTER_TIME_Pos              (21UL)
#define IR_FILTER_TIME_Msk              (0x3UL << IR_FILTER_TIME_Pos)
#define IR_FILTER_TIME_CLR              (~IR_FILTER_TIME_Msk)
/* RX_CONFIG[19] :IR_RX_FIFO_ERROR_MASK_INT. 0x1: mask. 0x0: unmask.*/
#define IR_RX_FIFO_ERROR_MASK_INT_Pos   (19UL)
#define IR_RX_FIFO_ERROR_MASK_INT_Msk   (0x1UL << IR_RX_FIFO_ERROR_MASK_INT_Pos)
#define IR_RX_FIFO_ERROR_MASK_INT_CLR   (~IR_RX_FIFO_ERROR_MASK_INT_Msk)
/* RX_CONFIG[18] :IR_RX_CNT_THR_MASK_INT. 0x1: enable. 0x0: disable.*/
#define IR_RX_CNT_THR_MASK_INT_Pos      (18UL)
#define IR_RX_CNT_THR_MASK_INT_Msk      (0x1UL << IR_RX_CNT_THR_MASK_INT_Pos)
#define IR_RX_CNT_THR_MASK_INT_CLR      (~IR_RX_CNT_THR_MASK_INT_Msk)
/* RX_CONFIG[17] :IR_RX_FIFO_OF_MASK_INT. 0x1: enable. 0x0: disable.*/
#define IR_RX_FIFO_OF_MASK_INT_Pos      (17UL)
#define IR_RX_FIFO_OF_MASK_INT_Msk      (0x1UL << IR_RX_FIFO_OF_MASK_INT_Pos)
#define IR_RX_FIFO_OF_MASK_INT_CLR      (~IR_RX_FIFO_OF_MASK_INT_Msk)
/* RX_CONFIG[16] :IR_RX_CNT_OF_MASK_INT. 0x1: enable. 0x0: disable.*/
#define IR_RX_CNT_OF_MASK_INT_Pos       (16UL)
#define IR_RX_CNT_OF_MASK_INT_Msk       (0x1UL << IR_RX_CNT_OF_MASK_INT_Pos)
#define IR_RX_CNT_OF_MASK_INT_CLR       (~IR_RX_CNT_OF_MASK_INT_Msk)
/* RX_CONFIG[15] :IR_RX_FIFO_LEVEL_MASK_INT. 0x1: enable. 0x0: disable.*/
#define IR_RX_FIFO_LEVEL_MASK_INT_Pos   (15UL)
#define IR_RX_FIFO_LEVEL_MASK_INT_Msk   (0x1UL << IR_RX_FIFO_LEVEL_MASK_INT_Pos)
#define IR_RX_FIFO_LEVEL_MASK_INT_CLR   (~IR_RX_FIFO_LEVEL_MASK_INT_Msk)
/* RX_CONFIG[14] :IR_RX_FIFO_FULL_MASK_INT. 0x1: enable. 0x0: disable.*/
#define IR_RX_FIFO_FULL_MASK_INT_Pos    (14UL)
#define IR_RX_FIFO_FULL_MASK_INT_Msk    (0x1UL << IR_RX_FIFO_FULL_MASK_INT_Pos)
#define IR_RX_FIFO_FULL_MASK_INT_CLR    (~IR_RX_FIFO_FULL_MASK_INT_Msk)
/* RX_CONFIG[13] :IR_RX_FIFO_DISCARD_SET. 0x1: reject new data send to FIFO. 0x0: discard oldest data in FIFO.*/
#define IR_RX_FIFO_DISCARD_SET_Pos      (13UL)
#define IR_RX_FIFO_DISCARD_SET_Msk      (0x1UL << IR_RX_FIFO_DISCARD_SET_Pos)
#define IR_RX_FIFO_DISCARD_SET_CLR      (~IR_RX_FIFO_DISCARD_SET_Msk)
/* RX_CONFIG[8] :IR_RX_FIFO_LEVE. */
#define IR_RX_FIFO_LEVEL_Pos            (8UL)
#define IR_RX_FIFO_LEVEL_Msk            (0x1fUL << IR_RX_FIFO_LEVEL_Pos)
#define IR_RX_FIFO_LEVEL_CLR            (~IR_RX_FIFO_LEVEL_Msk)
/* RX_CONFIG[5] :IR_RX_FIFO_ERROR_INT. 0x1: enable. 0x0: disable.*/
#define IR_RX_FIFO_ERROR_INT_Pos        (5UL)
#define IR_RX_FIFO_ERROR_INT_Msk        (0x1UL << IR_RX_FIFO_ERROR_INT_Pos)
#define IR_RX_FIFO_ERROR_INT_CLR        (~IR_RX_FIFO_ERROR_INT_Msk)
/* RX_CONFIG[4] :IR_RX_CNT_THR_INT. 0x1: enable. 0x0: disable.*/
#define IR_RX_CNT_THR_INT_Pos           (4UL)
#define IR_RX_CNT_THR_INT_Msk           (0x1UL << IR_RX_CNT_THR_INT_Pos)
#define IR_RX_CNT_THR_INT_CLR           (~IR_RX_CNT_THR_INT_Msk)
/* RX_CONFIG[3] :IR_RX_FIFO_OF_INT. 0x1: enable. 0x0: disable.*/
#define IR_RX_FIFO_OF_INT_Pos           (3UL)
#define IR_RX_FIFO_OF_INT_Msk           (0x1UL << IR_RX_FIFO_OF_INT_Pos)
#define IR_RX_FIFO_OF_INT_CLR           (~IR_RX_FIFO_OF_INT_Msk)
/* RX_CONFIG[2] :IR_RX_CNT_OF_INT. 0x1: enable. 0x0: disable.*/
#define IR_RX_CNT_OF_INT_Pos            (2UL)
#define IR_RX_CNT_OF_INT_Msk            (0x1UL << IR_RX_CNT_OF_INT_Pos)
#define IR_RX_CNT_OF_INT_CLR            (~IR_RX_CNT_OF_INT_Msk)
/* RX_CONFIG[1] :IR_RX_FIFO_LEVEL_INT. 0x1: enable. 0x0: disable.*/
#define IR_RX_FIFO_LEVEL_INT_Pos        (1UL)
#define IR_RX_FIFO_LEVEL_INT_Msk        (0x1UL << IR_RX_FIFO_LEVEL_INT_Pos)
#define IR_RX_FIFO_LEVEL_INT_CLR        (~IR_RX_FIFO_LEVEL_INT_Msk)
/* RX_CONFIG[0] :IR_RX_FIFO_FULL_INT. 0x1: enable. 0x0: disable.*/
#define IR_RX_FIFO_FULL_INT_Pos         (0UL)
#define IR_RX_FIFO_FULL_INT_Msk         (0x1UL << IR_RX_FIFO_FULL_INT_Pos)
#define IR_RX_FIFO_FULL_INT_CLR         (~IR_RX_FIFO_FULL_INT_Msk)
#define IR_RX_MSK_TO_EN_Pos             (IR_RX_FIFO_FULL_MASK_INT_Pos - IR_RX_FIFO_FULL_INT_Pos)

#define IR_RX_MASK_ALL_INT              (IR_RX_FIFO_ERROR_MASK_INT_Msk | IR_RX_CNT_THR_MASK_INT_Msk | \
                                         IR_RX_FIFO_OF_MASK_INT_Msk | IR_RX_CNT_OF_MASK_INT_Msk | \
                                         IR_RX_FIFO_LEVEL_MASK_INT_Msk | IR_RX_FIFO_FULL_MASK_INT_Msk)

/* Register: RX_SR -----------------------------------------------------------*/
/* Description: RX_SR register. Offset: 0x1C. Address: 0x4000301C. */

/* RX_SR[17] :IR_TX_FIFO_EMPTY. 0x1: empty. 0x0: not empty. */
#define IR_RX_FIFO_EMPTY_Pos            (17UL)
#define IR_RX_FIFO_EMPTY_Msk            (0x1UL << IR_RX_FIFO_EMPTY_Pos)
/* RX_SR[8] :IR_RX_FIFO_OFFSET. */
#define IR_RX_FIFO_OFFSET_Pos           (8UL)
#define IR_RX_FIFO_OFFSET_Msk           (0x3fUL << IR_RX_FIFO_OFFSET_Pos)
#define IR_RX_FIFO_OFFSET_CLR           (~IR_RX_FIFO_OFFSET_Msk)
/* RX_SR[7] :IR_RX_STATUS. */
#define IR_RX_STATUS_Pos                (7UL)
#define IR_RX_STATUS_Msk                (0x1UL << IR_RX_STATUS_Pos)

/* Register: RX_INT_CLR -----------------------------------------------------------*/
/* Description: RX_INT_CLR register. Offset: 0x20. Address: 0x40003020. */

/* RX_INT_CLR[8] :IR_RX_FIFO_CLR. Write 1 clear. */
#define IR_RX_FIFO_CLR_Pos              (8UL)
#define IR_RX_FIFO_CLR_Msk              (0x1UL << IR_RX_FIFO_CLR_Pos)
/* RX_INT_CLR[5] :IR_RX_FIFO_ERROR_INT_CLR. Write 1 clear. */
#define IR_RX_FIFO_ERROR_INT_CLR_Pos    (5UL)
#define IR_RX_FIFO_ERROR_INT_CLR_Msk    (0x1UL << IR_RX_FIFO_ERROR_INT_CLR_Pos)
/* RX_INT_CLR[4] :IR_RX_CNT_THR_INT_CLR. Write 1 clear. */
#define IR_RX_CNT_THR_INT_CLR_Pos       (4UL)
#define IR_RX_CNT_THR_INT_CLR_Msk       (0x1UL << IR_RX_CNT_THR_INT_CLR_Pos)
/* RX_INT_CLR[3] :IR_RX_FIFO_OF_INT_CLR. Write 1 clear. */
#define IR_RX_FIFO_OF_INT_CLR_Pos       (3UL)
#define IR_RX_FIFO_OF_INT_CLR_Msk       (0x1UL << IR_RX_FIFO_OF_INT_CLR_Pos)
/* RX_INT_CLR[2] :IR_RX_CNT_OF_INT_CLR. Write 1 clear. */
#define IR_RX_CNT_OF_INT_CLR_Pos        (2UL)
#define IR_RX_CNT_OF_INT_CLR_Msk        (0x1UL << IR_RX_CNT_OF_INT_CLR_Pos)
/* RX_INT_CLR[1] :IR_RX_FIFO_LEVEL_INT_CLR. Write 1 clear. */
#define IR_RX_FIFO_LEVEL_INT_CLR_Pos    (1UL)
#define IR_RX_FIFO_LEVEL_INT_CLR_Msk    (0x1UL << IR_RX_FIFO_LEVEL_INT_CLR_Pos)
/* RX_INT_CLR[0] :IR_RX_FIFO_FULL_INT_CLR. Write 1 clear. */
#define IR_RX_FIFO_FULL_INT_CLR_Pos     (0UL)
#define IR_RX_FIFO_FULL_INT_CLR_Msk     (0x1UL << IR_RX_FIFO_FULL_INT_CLR_Pos)

#define IR_RX_INT_ALL_CLR               (IR_RX_FIFO_CLR_Msk | IR_RX_FIFO_ERROR_INT_CLR_Msk | \
                                         IR_RX_CNT_THR_INT_CLR_Msk | IR_RX_FIFO_OF_INT_CLR_Msk | \
                                         IR_RX_CNT_OF_INT_CLR_Msk | IR_RX_FIFO_LEVEL_INT_CLR_Msk | \
                                         IR_RX_FIFO_FULL_INT_CLR_Msk)

/* Register: RX_CNT_INT_SEL -------------------------------------------------*/
/* Description: IR RX counter interrupt setting register. Offset: 0x24. Address: 0x40003024. */

/* RX_CNT_INT_SEL[31] :IR_RX_CNT_THR_TRIGGER_LV. */
/* 0x1: high level couner >= threshlod trigger interrupt. 0x0: low level couner >= threshlod trigger interrupt. */
#define IR_RX_CNT_THR_TYPE_Pos          (31UL)
#define IR_RX_CNT_THR_TYPE_Msk          (0x1UL << IR_RX_CNT_THR_TYPE_Pos)
#define IR_RX_CNT_THR_TYPE_CLR          (~IR_RX_CNT_THR_TYPE_Msk)
/* RX_CNT_INT_SEL[0] :IR_RX_CNT_THR. */
#define IR_RX_CNT_THR_Pos               (0UL)
#define IR_RX_CNT_THR_Msk               (0x7fffffffUL << IR_RX_CNT_THR_Pos)
#define IR_RX_CNT_THR_CLR               (~IR_RX_CNT_THR_Msk)

/* Register: IR_TX_COMPE -------------------------------------------------*/
/* Description: IR TX compensation register. Offset: 0x48. Address: 0x40003048. */

/* IR_TX_COMPE[27:16] : IR_TX_COMPENSATION. */
#define IR_TX_COMPENSATION_Pos          (16UL)
#define IR_TX_COMPENSATION_Msk          (0xFFFUL << IR_TX_COMPENSATION_Pos)
#define IR_TX_COMPENSATION_CLR          (~IR_TX_COMPENSATION_Msk)

/** @addtogroup IR IR
  * @brief IR driver module
  * @{
  */

/*============================================================================*
 *                         Types
 *============================================================================*/

/** @defgroup IR_Exported_Types IR Exported Types
  * @{
  */

/**
 * @brief IR initialize parameters
 *
 * IR initialize parameters
 */
typedef struct
{
    uint32_t IR_Clock;
    uint32_t IR_Freq;               /*!< Specifies the clock frequency. This parameter is IR carrier freqency whose unit is Hz.
                                              This parameter can be a value of @ref IR_Frequency */
    uint32_t IR_DutyCycle;          /*!< Specifies the IR duty cycle. */
    uint32_t IR_Mode;               /*!< Specifies the IR mode.
                                              This parameter can be a value of @ref IR_Mode */
    uint32_t IR_TxIdleLevel;        /*!< Specifies the IR output level in Tx mode
                                              This parameter can be a value of @ref IR_Idle_Status */
    uint32_t IR_TxInverse;          /*!< Specifies inverse FIFO data or not in TX mode
                                              This parameter can be a value of @ref IR_TX_Data_Type */
    uint32_t IR_TxFIFOThrLevel;     /*!< Specifies TX FIFO interrupt threshold in TX mode. When TX FIFO depth <= threshold value, trigger interrupt.
                                              This parameter can be a value of @ref IR_Tx_Threshold */
    uint32_t IR_RxStartMode;        /*!< Specifies Start mode in RX mode
                                              This parameter can be a value of @ref IR_Rx_Start_Mode */
    uint32_t IR_RxFIFOThrLevel;     /*!< Specifies RX FIFO interrupt threshold in RX mode. when RX FIFO depth > threshold value, trigger interrupt.
                                              This parameter can be a value of @ref IR_Rx_Threshold */
    uint32_t IR_RxFIFOFullCtrl;     /*!< Specifies data discard mode in RX mode when RX FIFO is full and receiving new data
                                              This parameter can be a value of @ref IR_RX_FIFO_DISCARD_SETTING */
    uint32_t IR_RxTriggerMode;      /*!< Specifies trigger in RX mode
                                              This parameter can be a value of @ref IR_RX_Trigger_Mode */
    uint32_t IR_RxFilterTime;       /*!< Specifies filter time in RX mode
                                              This parameter can be a value of @ref IR_RX_Filter_Time */
    uint32_t IR_RxCntThrType;       /*!< Specifies counter level type when trigger IR_INT_RX_CNT_THR interrupt in RX mode
                                              This parameter can be a value of @ref IR_RX_COUNTER_THRESHOLD_TYPE */
    uint32_t IR_RxCntThr;           /*!< Specifies counter threshold value when trigger IR_INT_RX_CNT_THR interrupt in RX mode */
} IR_InitTypeDef;

/** End of group IR_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/


/** @defgroup IR_Exported_Constants IR Exported Constants
  * @{
  */

#define IS_IR_PERIPH(PERIPH) ((PERIPH) == IR)
#define IR_TX_FIFO_SIZE                   32
#define IR_RX_FIFO_SIZE                   32

/** @defgroup IR_Frequency IR Frequency
  * @{
  */

#define IS_IR_FREQUENCY(F) (((F) >= 1) && ((F) <= 72))

/** End of group IR_Frequency
  * @}
  */

/** @defgroup IR_Mode IR Mode
  * @{
  */

#define IR_MODE_TX                                  ((uint32_t)((uint32_t)0x0 << IR_MODE_SEL_Pos))
#define IR_MODE_RX                                  ((uint32_t)((uint32_t)0x1 << IR_MODE_SEL_Pos))

#define IS_IR_MODE(MODE) (((MODE) == IR_MODE_TX) || ((MODE) == IR_MODE_RX))

/** End of group IR_Mode
  * @}
  */

/** @defgroup IR_Idle_Status  IR Idle Status
  * @{
  */

#define IR_IDLE_OUTPUT_HIGH                         ((uint32_t)(0x01 << IR_TX_IDLE_STATE_Pos))
#define IR_IDLE_OUTPUT_LOW                          ((uint32_t)(0x00 << IR_TX_IDLE_STATE_Pos))

#define IS_IR_IDLE_STATUS(LEVEL) (((LEVEL) == IR_IDLE_OUTPUT_HIGH) || ((LEVEL) == IR_IDLE_OUTPUT_LOW))

/** End of group IR_Idle_Status
  * @}
  */

/** @defgroup IR_TX_Data_Type IR TX Data Type
  * @{
  */

#define IR_TX_DATA_NORMAL                           ((uint32_t)(0 << IR_FIFO_INVERSE_Pos))
#define IR_TX_DATA_INVERSE                          ((uint32_t)(1 << IR_FIFO_INVERSE_Pos))

#define IS_IR_TX_DATA_TYPE(TYPE) (((TYPE) == IR_TX_DATA_NORMAL) || ((TYPE) == IR_TX_DATA_INVERSE))

/** End of group IR_TX_Data_Type
  * @}
  */

/** @defgroup IR_Tx_Threshold IR TX Threshold
  * @{
  */

#define IS_IR_TX_THRESHOLD(THD)  ((THD) <= IR_TX_FIFO_SIZE)

/** End of group IR_Tx_Threshold
  * @}
  */

/** @defgroup IR_Rx_Start_Mode RX Start Mode
  * @{
  */

#define IR_RX_AUTO_MODE                             ((uint32_t)((0x1) << IR_RX_START_MODE_Pos))
#define IR_RX_MANUAL_MODE                           ((uint32_t)((0x0) << IR_RX_START_MODE_Pos))

#define IS_RX_START_MODE(MODE) (((MODE) == IR_RX_AUTO_MODE) || ((MODE) == IR_RX_MANUAL_MODE))

/** End of group IR_Rx_Start_Mode
  * @}
  */

/** @defgroup IR_Rx_Threshold IR RX Threshold
  * @{
  */

#define IS_IR_RX_THRESHOLD(THD) ((THD) <= IR_RX_FIFO_SIZE)

/** End of group IR_Rx_Threshold
  * @}
  */

/** @defgroup IR_RX_Trigger_Mode RX Trigger Mode
  * @{
  */

#define IR_RX_FALL_EDGE                             ((uint32_t)((0x0) << IR_TRIGGER_MODE_Pos))
#define IR_RX_RISING_EDGE                           ((uint32_t)((0x1) << IR_TRIGGER_MODE_Pos))
#define IR_RX_DOUBLE_EDGE                           ((uint32_t)((0x2) << IR_TRIGGER_MODE_Pos))

#define IS_RX_RX_TRIGGER_EDGE(EDGE) (((EDGE) == IR_RX_FALL_EDGE) || ((EDGE) == IR_RX_RISING_EDGE) || ((EDGE) == IR_RX_DOUBLE_EDGE))

/** End of group IR_RX_Trigger_Mode
  * @}
  */

/** @defgroup IR_RX_FIFO_DISCARD_SETTING IR RX FIFO Discard Setting
  * @{
  */

#define IR_RX_FIFO_FULL_DISCARD_NEWEST              ((uint32_t)(0 << IR_RX_FIFO_DISCARD_SET_Pos))
#define IR_RX_FIFO_FULL_DISCARD_OLDEST              ((uint32_t)(1 << IR_RX_FIFO_DISCARD_SET_Pos))

#define IS_IR_RX_FIFO_FULL_CTRL(CTRL)  (((CTRL) == IR_RX_FIFO_FULL_DISCARD_NEWEST) || ((CTRL) == IR_RX_FIFO_FULL_DISCARD_OLDEST))

/** End of group IR_RX_FIFO_DISCARD_SETTING
  * @}
  */

/** @defgroup IR_RX_Filter_Time RX Filter Time
  * @{
  */

#define IR_RX_FILTER_TIME_50ns                      ((uint32_t)((0x0) << IR_FILTER_TIME_Pos))
#define IR_RX_FILTER_TIME_75ns                      ((uint32_t)((0x2) << IR_FILTER_TIME_Pos))
#define IR_RX_FILTER_TIME_100ns                     ((uint32_t)((0x3) << IR_FILTER_TIME_Pos))
#define IR_RX_FILTER_TIME_125ns                     ((uint32_t)((0x4) << IR_FILTER_TIME_Pos))
#define IR_RX_FILTER_TIME_150ns                     ((uint32_t)((0x5) << IR_FILTER_TIME_Pos))
#define IR_RX_FILTER_TIME_175ns                     ((uint32_t)((0x6) << IR_FILTER_TIME_Pos))
#define IR_RX_FILTER_TIME_200ns                     ((uint32_t)((0x7) << IR_FILTER_TIME_Pos))

#define IS_IR_RX_FILTER_TIME_CTRL(CTRL)  (((CTRL) == IR_RX_FILTER_TIME_50ns) || \
                                          ((CTRL) == IR_RX_FILTER_TIME_75ns) || \
                                          ((CTRL) == IR_RX_FILTER_TIME_100ns) || \
                                          ((CTRL) == IR_RX_FILTER_TIME_125ns) || \
                                          ((CTRL) == IR_RX_FILTER_TIME_150ns) || \
                                          ((CTRL) == IR_RX_FILTER_TIME_175ns) || \
                                          ((CTRL) == IR_RX_FILTER_TIME_200ns))
/** End of group IR_RX_Filter_Time
  * @}
  */

/** @defgroup IR_RX_COUNTER_THRESHOLD_TYPE IR RX Counter threshold type
  * @{
  */

#define IR_RX_Count_Low_Level                       ((uint32_t)0 << IR_RX_CNT_THR_TYPE_Pos)
#define IR_RX_Count_High_Level                      ((uint32_t)1 << IR_RX_CNT_THR_TYPE_Pos)

#define IS_IR_RX_COUNT_LEVEL_CTRL(CTRL)  (((CTRL) == IR_RX_Count_Low_Level) || ((CTRL) == IR_RX_Count_High_Level))

/** End of group IR_RX_COUNTER_THRESHOLD_TYPE
  * @}
  */

/** @defgroup IR_Rx_Counter_Threshold IR RX Counter Threshold
  * @{
  */

#define IS_IR_RX_COUNTER_THRESHOLD(THD) ((THD) <= IR_RX_CNT_THR_Msk)

/** End of group IR_Rx_Counter_Threshold
  * @}
  */

/** @defgroup IR_Interrupts_Definition  IR Interrupts Definition
  * @{
  */

/* All interrupts in transmission mode */
#define IR_INT_TF_EMPTY                             ((uint32_t)IR_TX_FIFO_EMPTY_INT_EN_Msk)
#define IR_INT_TF_LEVEL                             ((uint32_t)IR_TX_FIFO_LEVEL_INT_EN_Msk)
#define IR_INT_TF_OF                                ((uint32_t)IR_TX_FIFO_OVER_INT_EN_Msk)
/* All interrupts in receiving mode */
#define IR_INT_RF_FULL                              ((uint32_t)IR_RX_FIFO_FULL_INT_Msk)
#define IR_INT_RF_LEVEL                             ((uint32_t)IR_RX_FIFO_LEVEL_INT_Msk)
#define IR_INT_RX_CNT_OF                            ((uint32_t)IR_RX_CNT_OF_INT_Msk)
#define IR_INT_RF_OF                                ((uint32_t)IR_RX_FIFO_OF_INT_Msk)
#define IR_INT_RX_CNT_THR                           ((uint32_t)IR_RX_CNT_THR_INT_Msk)
#define IR_INT_RF_ERROR                             ((uint32_t)IR_RX_FIFO_ERROR_INT_Msk)

#define IS_IR_TX_INT_CONFIG(CONFIG)   (((CONFIG) == IR_INT_TF_EMPTY)   || \
                                       ((CONFIG) == IR_INT_TF_LEVEL)   || \
                                       ((CONFIG) == IR_INT_TF_OF))

#define IS_IR_RX_INT_CONFIG(CONFIG)   (((CONFIG) == IR_INT_RF_FULL)     || \
                                       ((CONFIG) == IR_INT_RF_LEVEL)   || \
                                       ((CONFIG) == IR_INT_RX_CNT_OF)  || \
                                       ((CONFIG) == IR_INT_RF_OF)      || \
                                       ((CONFIG) == IR_INT_RX_CNT_THR) || \
                                       ((CONFIG) == IR_INT_RF_ERROR))
#define IS_IR_INT_CONFIG(CONFIG)      (IS_IR_TX_INT_CONFIG(CONFIG) || IS_IR_RX_INT_CONFIG(CONFIG))

/** End of group IR_Interrupts_Definition
  * @}
  */

/** @defgroup IR_Interrupts_Clear_Flag IR Interrupts Clear Flag
  * @{
  */

/* Clear all interrupts in transmission mode */
#define IR_INT_TF_EMPTY_CLR                                 ((uint32_t)IR_TX_FIFO_EMPTY_INT_CLR_Msk)
#define IR_INT_TF_LEVEL_CLR                                 ((uint32_t)IR_TX_FIFO_LEVEL_INT_CLR_Msk)
#define IR_INT_TF_OF_CLR                                    ((uint32_t)IR_TX_FIFO_OVER_INT_CLR_Msk)
/* Clear all interrupts in receiving mode */
#define IR_INT_RF_FULL_CLR                                  ((uint32_t)IR_RX_FIFO_FULL_INT_CLR_Msk)
#define IR_INT_RF_LEVEL_CLR                                 ((uint32_t)IR_RX_FIFO_LEVEL_INT_CLR_Msk)
#define IR_INT_RX_CNT_OF_CLR                                ((uint32_t)IR_RX_CNT_OF_INT_CLR_Msk)
#define IR_INT_RF_OF_CLR                                    ((uint32_t)IR_RX_FIFO_OF_INT_CLR_Msk)
#define IR_INT_RX_CNT_THR_CLR                               ((uint32_t)IR_RX_CNT_THR_INT_CLR_Msk)
#define IR_INT_RF_ERROR_CLR                                 ((uint32_t)IR_RX_FIFO_ERROR_INT_CLR_Msk)
#define IR_RF_CLR                                           ((uint32_t)IR_RX_FIFO_CLR_Msk)
#define IS_IR_INT_CLEAR(INT)            (((INT) == IR_INT_TF_EMPTY_CLR)  || ((INT) == IR_INT_TF_LEVEL_CLR)  || \
                                         ((INT) == IR_INT_TF_OF_CLR)      || ((INT) == IR_INT_RF_FULL_CLR)   || \
                                         ((INT) == IR_INT_RF_LEVEL_CLR)   || ((INT) == IR_INT_RX_CNT_OF_CLR) || \
                                         ((INT) == IR_INT_RF_OF_CLR)      || ((INT) == IR_INT_RX_CNT_THR_CLR)|| \
                                         ((INT) == IR_INT_RF_ERROR_CLR))

/** End of group IR_Interrupts_Clear_Flag
  * @}
  */

/** @defgroup IR_Flag IR Flag
  * @{
  */

#define IR_FLAG_TF_EMPTY                                    ((uint32_t)IR_TX_FIFO_EMPTY_Msk)
#define IR_FLAG_TF_FULL                                     ((uint32_t)IR_TX_FIFO_FULL_Msk)
#define IR_FLAG_TX_RUN                                      ((uint32_t)IR_TX_STATUS_Msk)
#define IR_FLAG_RF_EMPTY                                    ((uint32_t)IR_RX_FIFO_EMPTY_Msk)
#define IR_FLAG_RX_RUN                                      ((uint32_t)IR_RX_STATUS_Msk)

#define IS_IR_FLAG(FLAG)                (((FLAG) == IR_FLAG_TF_EMPTY) || ((FLAG) == IR_FLAG_TF_FULL) || \
                                         ((FLAG) == IR_FLAG_TX_RUN) || ((FLAG) == IR_FLAG_RF_EMPTY) || \
                                         ((FLAG) == IR_FLAG_RX_RUN))
/** End of group IR_Flag
  * @}
  */

/** @defgroup IR_Compensation_Flag IR Compensation Flag
  * @{
  */

typedef enum
{
    IR_COMPEN_FLAG_1_2_CARRIER = ((uint32_t)(1 << IR_LOW_PERIOD_COMPENSATION_Pos)),
    IR_COMPEN_FLAG_1_4_CARRIER = ((uint32_t)(2 << IR_LOW_PERIOD_COMPENSATION_Pos)),
    IR_COMPEN_FLAG_1_N_SYSTEM_CLK = ((uint32_t)(3 << IR_LOW_PERIOD_COMPENSATION_Pos)),
} IR_TX_COMPEN_TYPE;

/** End of group IR_Compensation_Flag
  * @}
  */

/** End of group IR_Exported_Constants
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup IR_Exported_Functions IR Exported Functions
  * @{
  */

/**
  * @brief  Deinitializes the IR peripheral registers to their default values.
  * @param  None
  * @retval None
  */
void IR_DeInit(void);

/**
  * @brief Initializes the IR peripheral according to the specified
  *   parameters in the IR_InitStruct
  * @param  IR_InitStruct: pointer to a IR_InitTypeDef structure that
  *   contains the configuration information for the specified IR peripheral
  * @retval None
  */
void IR_Init(IR_InitTypeDef *IR_InitStruct);

/**
  * @brief  Fills each IR_InitStruct member with its default value.
  * @param  IR_InitStruct: pointer to an IR_InitTypeDef structure which will be initialized.
  * @retval None
  */
void IR_StructInit(IR_InitTypeDef *IR_InitStruct);

/**
  * @brief   Enable or disable the selected IR mode.
  * @param  mode: selected IR operation mode.
  *   This parameter can be the following values:
  *     @arg IR_MODE_TX: Transmission mode.
  *     @arg IR_MODE_RX: Receiving mode.
  * @param  NewState: new state of the operation mode.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void IR_Cmd(uint32_t mode, FunctionalState NewState);

/**
  * @brief   Start trigger only in manual receive mode.
  * @param  None
  * @retval None
  */
void IR_StartManualRxTrigger(void);

/**
  * @brief   Configure counter threshold value in receiving mode.You can use it to stop receiving IR data.
  * @param  IR_RxCntThrType:
  *   This parameter can be the following values:
  *     @arg IR_RX_Count_Low_Level: Low level counter value >= IR_RxCntThr, trigger IR_INT_RX_CNT_THR interrupt.
  *     @arg IR_RX_Count_High_Level: High level counter value >= IR_RxCntThr, trigger IR_INT_RX_CNT_THR interrupt.
  * @param  IR_RxCntThr: Configure IR Rx counter threshold value which can be 0 to 0x7fffffffUL.
  * @retval None
  */
void IR_SetRxCounterThreshold(uint32_t IR_RxCntThrType, uint32_t IR_RxCntThr);

/**
  * @brief  Send data.
  * @param  pBuf: data buffer to send.
  * @param  len: buffer length.
  * @param IsLastPacket:
  *   This parameter can be the following values:
  *     @arg ENABLE: The last data in IR packet and there is no continous data.In other words, An infrared data transmission is completed.
  *     @arg DISABLE: There is data to be transmitted continuously.
  * @retval None
  */
void IR_SendBuf(uint32_t *pBuf, uint32_t len, FunctionalState IsLastPacket);

/**
  * @brief  Send compensation data.
  * @param  comp_type:
  *     @arg IR_COMPEN_FLAG_1_2_CARRIER: 1/2 carrier freqency.
  *     @arg IR_COMPEN_FLAG_1_4_CARRIER: 1/4 carrier freqency.
  *     @arg IR_COMPEN_FLAG_1_N_SYSTEM_CLK: MOD((0x48[27:16]+0x00[11:0]), 4095)/40MHz.
         User can call function of IR_ConfigCompParam to configure 0x48[27:16].
  * @param  buf: data buffer to send.
  * @param  length: buffer length.
  * @param IsLastPacket:
  *   This parameter can be the following values:
  *     @arg ENABLE: The last data in IR packet and there is no continous data.In other words, An infrared data transmission is completed.
  *     @arg DISABLE: There is data to be transmitted continuously.
  * @retval None
  */
void IR_SendCompenBuf(IR_TX_COMPEN_TYPE comp_type, uint32_t *pBuf, uint32_t len,
                      FunctionalState IsLastPacket);

/**
  * @brief  Read data From RX FIO.
  * @param  pBuf: buffer address to receive data.
  * @param  length: read data length.
  * @retval None
  */
void IR_ReceiveBuf(uint32_t *pBuf, uint32_t length);

/**
  * @brief  Enables or disables the specified IR interrupts.
  * @param  IR_INT: specifies the IR interrupts sources to be enabled or disabled.
  *   This parameter can be the following values:
  *     @arg IR_INT_TF_EMPTY: TX FIFO empty interrupt.
  *     @arg IR_INT_TF_LEVEL: TX FIFO threshold interrupt.
  *     @arg IR_INT_TF_OF: TX FIFO overflow interrupt.
  *     @arg IR_INT_RF_FULL: RX FIFO  full interrupt.
  *     @arg IR_INT_RF_LEVEL: RX FIFO threshold interrupt.
  *     @arg IR_INT_RX_CNT_OF: RX counter overflow interrupt.
  *     @arg IR_INT_RF_OF: RX FIFO overflow interrupt.
  *     @arg IR_INT_RX_CNT_THR: RX counter threshold interrupt.
  *     @arg IR_INT_RF_ERROR: RX FIFO error read interrupt. Trigger when RX FIFO empty and read RX FIFO.
  * @param  newState: new state of the specified IR interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void IR_INTConfig(uint32_t IR_INT, FunctionalState newState);

/**
  * @brief  Mask or unmask the specified IR interrupts.
  * @param  IR_INT: specifies the IR interrupts sources to be mask or unmask.
  *   This parameter can be the following values:
  *     @arg IR_INT_TF_EMPTY: TX FIFO empty interrupt.
  *     @arg IR_INT_TF_LEVEL: TX FIFO threshold interrupt.
  *     @arg IR_INT_TF_OF: TX FIFO overflow interrupt.
  *     @arg IR_INT_RF_FULL: RX FIFO  full interrupt.
  *     @arg IR_INT_RF_LEVEL: RX FIFO threshold interrupt.
  *     @arg IR_INT_RX_CNT_OF: RX counter overflow interrupt.
  *     @arg IR_INT_RF_OF: RX FIFO overflow interrupt.
  *     @arg IR_INT_RX_CNT_THR: RX counter threshold interrupt.
  *     @arg IR_INT_RF_ERROR: RX FIFO error read interrupt. Trigger when RX FIFO empty and read RX FIFO.
  * @param  newState: new state of the specified IR interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void IR_MaskINTConfig(uint32_t IR_INT, FunctionalState newState);

/**
  * @brief Get the specified IR interrupt status.
  * @param  IR_INT: the specified IR interrupts.
  * This parameter can be one of the following values:
  *     @arg IR_INT_TF_EMPTY: TX FIFO empty interrupt.
  *     @arg IR_INT_TF_LEVEL: TX FIFO threshold interrupt.
  *     @arg IR_INT_TF_OF: TX FIFO overflow interrupt.
  *     @arg IR_INT_RF_FULL: RX FIFO  full interrupt.
  *     @arg IR_INT_RF_LEVEL: RX FIFO threshold interrupt.
  *     @arg IR_INT_RX_CNT_OF: RX counter overflow interrupt.
  *     @arg IR_INT_RF_OF: RX FIFO overflow interrupt.
  *     @arg IR_INT_RX_CNT_THR: RX counter threshold interrupt.
  *     @arg IR_INT_RF_ERROR: RX FIFO error read interrupt. Trigger when RX FIFO empty and read RX FIFO.
  * @retval The new state of IR_INT (SET or RESET).
  */
ITStatus IR_GetINTStatus(uint32_t IR_INT);

/**
  * @brief  Clears the IR interrupt pending bits.
  * @param  IR_CLEAR_INT: specifies the interrupt pending bit to clear.
  *   This parameter can be any combination of the following values:
  *     @arg IR_INT_TF_EMPTY_CLR: Clear TX FIFO empty interrupt.
  *     @arg IR_INT_TF_LEVEL_CLR: Clear TX FIFO threshold interrupt.
  *     @arg IR_INT_TF_OF_CLR: Clear TX FIFO overflow interrupt.
  *     @arg IR_INT_RF_FULL_CLR: Clear RX FIFO  full interrupt.
  *     @arg IR_INT_RF_LEVEL_CLR: Clear RX FIFO threshold interrupt.
  *     @arg IR_INT_RX_CNT_OF_CLR: Clear RX counter overflow interrupt.
  *     @arg IR_INT_RF_OF_CLR: Clear RX FIFO overflow interrupt.
  *     @arg IR_INT_RX_CNT_THR_CLR: Clear RX counter threshold interrupt.
  *     @arg IR_INT_RF_ERROR_CLR: Clear RX FIFO error read interrupt. Trigger when RX FIFO empty and read RX FIFO.
  * @retval None
  */
void IR_ClearINTPendingBit(uint32_t IR_CLEAR_INT);

/**
  * @brief  Get free size of TX FIFO .
  * @param  None
  * @retval the free size of TX FIFO
  */
__STATIC_INLINE uint16_t IR_GetTxFIFOFreeLen(void)
{
    return (uint16_t)(IR_TX_FIFO_SIZE - ((IR->TX_SR & (IR_TX_FIFO_OFFSET_Msk)) >> 8));
}

/**
  * @brief  Get data size in RX FIFO.
  * @param  None
  * @retval current data size in RX FIFO.
  */
__STATIC_INLINE uint16_t IR_GetRxDataLen(void)
{
    return ((uint16_t)(((IR->RX_SR) & IR_RX_FIFO_OFFSET_Msk) >> 8));
}

/**
  * @brief  Send one data.
  * @param  data: send data.
  * @retval None
  */
__STATIC_INLINE void IR_SendData(uint32_t data)
{
    IR->TX_FIFO = data;
}

/**
  * @brief Read one data.
  * @param none
  * @retval data which read from RX FIFO.
  */
__STATIC_INLINE uint32_t IR_ReceiveData(void)
{
    return IR->RX_FIFO;
}

/**
  * @brief  set tx threshold.when TX FIFO depth <= threshold value, trigger  interrupt
  * @param  thd: tx threshold.
  * @retval None
  */
__STATIC_INLINE void IR_SetTxThreshold(uint8_t thd)
{
    IR->TX_CONFIG &= IR_TX_FIFO_THRESHOLD_CLR;
    IR->TX_CONFIG |= (thd << IR_TX_FIFO_THRESHOLD_Pos);
}

/**
  * @brief  set tx threshold.when RX FIFO depth >= threshold value, trigger  interrupt
  * @param  thd: rx threshold.
  * @retval None
  */
__STATIC_INLINE void IR_SetRxThreshold(uint8_t thd)
{
    IR->RX_CONFIG &= IR_RX_FIFO_LEVEL_CLR;
    IR->RX_CONFIG |= (thd << IR_RX_FIFO_LEVEL_Pos);
}

/**
  * @brief  Clear IR TX FIFO.
  * @param  none
  * @retval None
  */
__STATIC_INLINE void IR_ClearTxFIFO(void)
{
    IR->TX_INT_CLR = IR_TX_FIFO_CLR_Msk;
}

/**
  * @brief  Clear IR RX FIFO.
  * @param  none
  * @retval None
  */
__STATIC_INLINE void IR_ClearRxFIFO(void)
{
    IR->RX_INT_CLR = IR_RX_FIFO_CLR_Msk;
}

/**
  * @brief  Checks whether the specified IR flag is set or not.
  * @param  IR_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg IR_FLAG_TF_EMPTY: TX FIFO empty or not. If SET, TX FIFO is empty.
  *     @arg IR_FLAG_TF_FULL: TX FIFO full or not. If SET, TX FIFO is full.
  *     @arg IR_FLAG_TX_RUN: TX run or not. If SET, TX is running.
  *     @arg IR_FLAG_RF_EMPTY: RX FIFO empty or not. If SET, RX FIFO is empty.
  *     @arg IR_FLAG_RX_RUN: RX run or not. If SET, RX is running.
  * @retval The new state of IR_FLAG (SET or RESET).
  */
__STATIC_INLINE FlagStatus IR_GetFlagStatus(uint32_t IR_FLAG)
{
    FlagStatus bitstatus = RESET;

    /* Check the parameters */
    assert_param(IS_IR_FLAG(IR_FLAG));

    if (IR->TX_CONFIG & IR_MODE_SEL_Msk)
    {
        if (IR->RX_SR & IR_FLAG)
        {
            bitstatus = SET;
        }
        return bitstatus;
    }
    else
    {
        if (IR->TX_SR & IR_FLAG)
        {
            bitstatus = SET;
        }
        return bitstatus;
    }
}

/**
  * @brief  set or reset tx data inverse.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
__STATIC_INLINE void IR_SetTxInverse(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == ENABLE)
    {
        IR->TX_CONFIG |= IR_FIFO_INVERSE_Msk;
    }
    else
    {
        IR->TX_CONFIG &= IR_FIFO_INVERSE_CLR;
    }
}

/**
  * @brief  TX output inverse or not.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
__STATIC_INLINE void IR_TxOutputInverse(FunctionalState NewState)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState == ENABLE)
    {
        IR->TX_CONFIG |= IR_OUTPUT_INVERSE_Msk;
    }
    else
    {
        IR->TX_CONFIG &= IR_OUTPUT_INVERSE_CLR;
    }
}

/**
  * @brief  Get free size of TX FIFO .
  * @param  data: compensation parameter, time = MOD((data+0x00[11:0]),4095)/40MHz
  * @retval the free size of TX FIFO
  */
__STATIC_INLINE void IR_ConfigCompenParam(uint32_t data)
{
    IR->IR_TX_COMPE = (data << IR_TX_COMPENSATION_Pos) & IR_TX_COMPENSATION_Msk;
}

#ifdef __cplusplus
}
#endif

#endif /* _RTL876x_IR_H_ */

/** @} */ /* End of group IR_Exported_Functions */
/** @} */ /* End of group IR */


/******************* (C) COPYRIGHT 2016 Realtek Semiconductor Corporation *****END OF FILE****/

