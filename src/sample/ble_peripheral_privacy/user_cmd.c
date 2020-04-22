/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
   * @file      user_cmd.c
   * @brief     User defined test commands.
   * @details  User command interfaces.
   * @author    jane
   * @date      2017-06-06
   * @version   v1.0
   **************************************************************************************
   * @attention
   * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
   **************************************************************************************
  */
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#if USER_CMD_EN
#include <gap_bond_le.h>
#include <user_cmd.h>
#include <gap_conn_le.h>
#include <peripheral_privacy_app.h>

/** @defgroup  PRIVA_PERIPH_CMD Peripheral Privacy User Command
    * @brief This file handles Peripheral Privacy  User Command.
    * @{
    */
/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @brief User command interface data, used to parse the commands from Data UART. */
T_USER_CMD_IF    user_cmd_if;

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief Disconnect to remote device
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "disc",
        "disc [conn_id]\n\r",
        "Disconnect to remote device\n\r",
        cmd_disc
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_disc(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    uint8_t conn_id = p_parse_value->dw_param[0];
    T_GAP_CAUSE cause;
    cause = le_disconnect(conn_id);
    return (T_USER_CMD_PARSE_RESULT)cause;
}

/**
 * @brief Clear all bonded devices information
 *
 * <b>Command table define</b>
 * \code{.c}
    {
        "bondclear",
        "bondclear\n\r",
        "Clear all bonded devices information\n\r",
        cmd_bondclear
    },
 * \endcode
 */
static T_USER_CMD_PARSE_RESULT cmd_bondclear(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
#if APP_PRIVACY_EN
    app_change_to_pair_mode();
#endif
    le_bond_clear_all_keys();
    return (RESULT_SUCESS);
}

/*----------------------------------------------------
 * command table
 * --------------------------------------------------*/
const T_USER_CMD_TABLE_ENTRY user_cmd_table[] =
{
    /************************** Common cmd *************************************/
    {
        "disc",
        "disc [conn_id]\n\r",
        "Disconnect to remote device\n\r",
        cmd_disc
    },
    {
        "bondclear",
        "bondclear\n\r",
        "Clear all bonded devices information\n\r",
        cmd_bondclear
    },
    /* MUST be at the end: */
    {
        0,
        0,
        0,
        0
    }
};
/** @} */ /* End of group PRIVA_PERIPH_CMD */
#endif

