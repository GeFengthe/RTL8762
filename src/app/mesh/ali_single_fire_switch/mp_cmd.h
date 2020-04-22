/**
*****************************************************************************************
*     Copyright(c) 2015, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file     mp_cmd.h
  * @brief    Source file for command parse.
  * @details  Data types and external functions declaration.
  * @author   hector
  * @date     2018-08-13
  * @version  v1.0
  * *************************************************************************************
  */


#ifndef _MP_CMD_H_
#define _MP_CMD_H_

#include "mp_cmd_parse.h"

BEGIN_DECLS

/** @brief  command parse related macros. */
#define MP_CMD_UPDATE_ALI_DATA   0x110F

/** @brief mp command table structure array */
extern const mp_cmd_table_t mp_cmd_table[];

/**
 * @brief get command table length
 * @return command table length
 */
uint32_t mp_cmd_table_length(void);

END_DECLS

#endif /* _MP_CMD_H_ */

