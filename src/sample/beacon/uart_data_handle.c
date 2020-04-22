#include <beacon_app.h>
#include <trace.h>
#include <uart_trans.h>
#include <uart_data_handle.h>
#include <gap_adv.h>
#if defined(DEVICE_CONNECTABLE)
#define MINIMUM_INTERVAL             20
T_UART_COMMAND command_check(uint8_t *data);
uint8_t char_to_hex(uint8_t char_in);
void tx_power_modify(TX_POWER_VAL tx_power);
uint8_t uuid_128bit[16] = {0xff};
uint16_t adv_interval = 0;

void data_handle(uint8_t *data, uint8_t length)
{
    if (data[length - 2] != 0x0d || data[length - 1] != 0x0a)
    {
        APP_PRINT_INFO0("fail!!!");
        return;
    }
    if (length == 42)
    {
        uint8_t char2hex[2];
        if (command_check(data) == UART_CHANGE_UUID)
        {
            for (int i = 8; i < UUID_COMMAND_LENGTH ; i++)
            {
                if ((i % 2) == 0)
                {
                    char2hex[0] = char_to_hex(data[i]);
                    char2hex[1] = char_to_hex(data[i + 1]);
                    if (char2hex[0] == 0x24 || char2hex[1] == 0x24)
                    {
                        APP_PRINT_INFO0("UUID format ERROR.");
                        return;
                    }
                    else
                    {
                        uuid_128bit[(i - 8) / 2] = char2hex[0] * 0x10 + char2hex[1];
                    }
                }
            }
            for (int i = 0; i < 16; i++)
            {
                adv_data[UUID_OFFSET + i] = uuid_128bit[i];
            }
            le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data), (void *)adv_data);
            le_adv_update_param();
        }
        else
        {
            APP_PRINT_INFO0("Data/Command format ERROR.");
            return;
        }
    }
    else if (length < 42 && length >= 11)
    {
        switch (command_check(data))
        {
        case UART_CHANGE_MAJOR_ID:
            if (length == 17)
            {
                uint16_t major_id = 0;
                uint8_t char2hex = 0x24;
                for (int i = 11; i < ID_COMMAND_LENGTH; i++)
                {
                    char2hex = char_to_hex(data[i]);
                    if (char2hex == 0x24)
                    {
                        APP_PRINT_INFO0("Major id format ERROR.");
                        return;
                    }
                    else
                    {
                        major_id = major_id * 0x10 + char2hex;
                    }
                }
                adv_data[MAJOR_ID_OFFSET] = HI_WORD(major_id);
                adv_data[MAJOR_ID_OFFSET + 1] = LO_WORD(major_id);
                le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data), (void *)adv_data);
                le_adv_update_param();
            }
            else
            {
                APP_PRINT_INFO0("Major ID exceeded.");
            }
            break;
        case UART_CHANGE_MINOR_ID:
            if (length == 17)
            {
                uint16_t minor_id = 0;
                uint8_t char2hex = 0x24;
                for (int i = 11; i < ID_COMMAND_LENGTH; i++)
                {
                    char2hex = char_to_hex(data[i]);
                    if (char2hex == 0x24)
                    {
                        APP_PRINT_INFO0("Minor id format ERROR.");
                        return;
                    }
                    else
                    {
                        minor_id = minor_id * 0x10 + char2hex;
                    }
                }
                adv_data[MINOR_ID_OFFSET] = HI_WORD(minor_id);
                adv_data[MINOR_ID_OFFSET + 1] = LO_WORD(minor_id);
                le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data), (void *)adv_data);
                le_adv_update_param();
            }
            else
            {
                APP_PRINT_INFO0("Major ID exceeded.");
            }
            break;
        case UART_CHANGE_RSSI:
            if (length == 12)
            {
                uint8_t rssi = 0;
                uint8_t char2hex = 0x24;
                for (int i = 8; i < RSSI_COMMAND_LENGTH; i++)
                {
                    char2hex = char_to_hex(data[i]);
                    APP_PRINT_INFO1("char = %x", char2hex);
                    if (char2hex == 0x24)
                    {
                        APP_PRINT_INFO0("Data format ERROR.");
                        return;
                    }
                    else
                    {
                        rssi = rssi * 0x10 + char2hex;
                    }
                }
                adv_data[RSSI_OFFSET] = rssi;
                le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data), (void *)adv_data);
                le_adv_update_param();
            }
            else
            {
                APP_PRINT_INFO0("TX_POWER exceeded.");
            }
            break;
        case UART_CHANGE_ADV_INTERVAL:
            if (length == 18)
            {
                uint8_t char2hex = 0x24;
                for (int i = 12; i < ADV_INTERVAL_COMMAND_LENGTH; i++)
                {
                    char2hex = char_to_hex(data[i]);
                    if (char2hex == 0x24)
                    {
                        APP_PRINT_INFO0("ADV_INTERVAL format ERROR.");
                        return;
                    }
                    else
                    {
                        adv_interval = adv_interval * 0x10 + char2hex;
                    }
                }
                if (adv_interval >= MINIMUM_INTERVAL)
                {
                    APP_PRINT_INFO1("ADV_INTERVAL %d", adv_interval);
                    adv_interval = adv_interval * 16 / 10;
                    modify_adv_interval = true;
                    le_adv_stop();
                }
                else
                {
                    APP_PRINT_INFO0("ADV_INTERVAL must be larger than 20ms.");
                }
            }
            else
            {
                APP_PRINT_INFO0("ADV_INTERVAL format error.");
            }
            break;
        case UART_CHANGE_TX_POWER:
            if (length == 15)
            {
                uint8_t tx_power = 0;
                uint8_t char2hex = 0x24;
                for (int i = 11; i < TX_POWER_COMMAND_LENGTH; i++)
                {
                    char2hex = char_to_hex(data[i]);
                    APP_PRINT_INFO1("char = %x", char2hex);
                    if (char2hex == 0x24)
                    {
                        APP_PRINT_INFO0("TX_POWER format ERROR.");
                        return;
                    }
                    else
                    {
                        tx_power = tx_power * 0x10 + char2hex;
                    }
                }
                tx_power_modify((TX_POWER_VAL)tx_power);
            }
            break;
        default:
            APP_PRINT_INFO0("Data/Command format ERROR.");
            return;
        }
    }
    else
    {
        APP_PRINT_INFO0("Data/Command format ERROR.");
    }
}

T_UART_COMMAND command_check(uint8_t *data)
{
    if ((data[0] == 'A') && (data[1] == 'T') && (data[2] == '+'))
    {
        if (data[3] == 'U' && data[4] == 'U' && data[5] == 'I' && data[6] == 'D' \
            && data[7] == '=')
        {
            return UART_CHANGE_UUID;
        }
        else if (data[3] == 'M' && data[4] == 'A' && data[5] == 'J' && data[6] == 'O' \
                 && data[7] == 'R' && data[8] == 'I' && data[9] == 'D' && data[10] == '=')
        {
            return UART_CHANGE_MAJOR_ID;
        }
        else if (data[3] == 'M' && data[4] == 'I' && data[5] == 'N' && data[6] == 'O' \
                 && data[7] == 'R' && data[8] == 'I' && data[9] == 'D' && data[10] == '=')
        {
            return UART_CHANGE_MINOR_ID;
        }
        else if (data[3] == 'T' && data[4] == 'X' && data[5] == 'P' && data[6] == 'O' \
                 && data[7] == 'W' && data[8] == 'E' && data[9] == 'R' && data[10] == '=')
        {
            return UART_CHANGE_TX_POWER;
        }
        else if (data[3] == 'A' && data[4] == 'D' && data[5] == 'V' && data[6] == 'I' \
                 && data[7] == 'N' && data[8] == 'T' && data[9] == 'V' && data[10] == 'L'\
                 && data[11] == '=')
        {
            return UART_CHANGE_ADV_INTERVAL;
        }
        else if (data[3] == 'R' && data[4] == 'S' && data[5] == 'S' && data[6] == 'I' \
                 && data[7] == '=')
        {
            return UART_CHANGE_RSSI;
        }
    }
    return UART_FAULT_COMMAND;
}

uint8_t char_to_hex(uint8_t char_in)
{
    uint8_t char_out;
    if (char_in >= 48 && char_in <= 57)
    {
        char_out = (char_in - 48);
    }
    else if (char_in >= 65 && char_in <= 70)
    {
        char_out = (char_in - 55);
    }
    else if (char_in >= 97 && char_in <= 102)
    {
        char_out = (char_in - 87);
    }
    else
    {
        char_out = 0x24;
    }
    return char_out;
}

void tx_power_modify(TX_POWER_VAL tx_power)
{
    switch (tx_power)
    {
    case POWER_N35_dBm:
    case POWER_N30_dBm:
    case POWER_N20_dBm:
    case POWER_N17_dBm:
    case POWER_N15_dBm:
    case POWER_N10_dBm:
    case POWER_N05_dBm:
    case POWER_N02_dBm:
    case POWER_0_dBm:
    case POWER_P03_dBm:
    case POWER_P04_dBm:
    case POWER_P06_dBm:
    case POWER_P07_dBm:
        le_adv_set_tx_power(GAP_ADV_TX_POW_SET_1M, tx_power);
        break;
    default:
        APP_PRINT_INFO0("TX_POWER format ERROR.");
        break;
    }
}
#endif
