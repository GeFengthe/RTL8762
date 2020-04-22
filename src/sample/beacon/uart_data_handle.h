#include <stdint.h>

#define UUID_COMMAND_LENGTH             40
#define TX_POWER_COMMAND_LENGTH         13
#define RSSI_COMMAND_LENGTH             10
#define ID_COMMAND_LENGTH               15
#define ADV_INTERVAL_COMMAND_LENGTH     16
typedef enum
{
    UART_CHANGE_UUID = 0x08,
    UART_CHANGE_MAJOR_ID,
    UART_CHANGE_MINOR_ID,
    UART_CHANGE_TX_POWER,
    UART_CHANGE_ADV_INTERVAL,
    UART_CHANGE_RSSI,
    UART_FAULT_COMMAND,
} T_UART_COMMAND;

typedef enum
{
    POWER_N35_dBm           =           0x10,
    POWER_N30_dBm           =           0x20,
    POWER_N20_dBm           =           0x30,
    POWER_N17_dBm           =           0x40,
    POWER_N15_dBm           =           0x50,
    POWER_N10_dBm           =           0x60,
    POWER_N05_dBm           =           0x70,
    POWER_N02_dBm           =           0x80,
    POWER_0_dBm             =           0x90,
    POWER_P03_dBm           =           0xA0,
    POWER_P04_dBm           =           0xB0,
    POWER_P06_dBm           =           0xC0,
    POWER_P07_dBm           =           0xD0,
} TX_POWER_VAL;
void data_handle(uint8_t *data, uint8_t length);
extern uint8_t uuid_128bit[16];
