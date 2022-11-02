#ifndef LORA_RADIO_H_
#define LORA_RADIO_H_	1

typedef enum {
	LORA = 0,
	FSK = 1
} MODULATION;

/**
 * TODO nor sure for BANDWIDTH_INDEX_7KHZ..BANDWIDTH_INDEX_125KHZ
 * @see https://github.com/x893/SX1231/blob/master/SX12xxDrivers-2.0.0/src/radio/sx1276-LoRa.c
 * SignalBw
 * 0: 7.8kHz, 1: 10.4 kHz, 2: 15.6 kHz, 3: 20.8 kHz, 4: 31.2 kHz,
 * 5: 41.6 kHz, 6: 62.5 kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz, other: Reserved
 */ 
typedef enum {
    BANDWIDTH_INDEX_7KHZ  = 0,    // 7.8
    BANDWIDTH_INDEX_10KHZ  = 1,   // 10.4
    BANDWIDTH_INDEX_15KHZ  = 2,   // 15.6
    BANDWIDTH_INDEX_20KHZ  = 3,   // 20.8
    BANDWIDTH_INDEX_31KHZ  = 4,   // 31.2
    BANDWIDTH_INDEX_41KHZ  = 5,   // 41.6
    BANDWIDTH_INDEX_62KHZ  = 6,   // 62.5
    BANDWIDTH_INDEX_125KHZ  = 7,  // 125
    BANDWIDTH_INDEX_250KHZ  = 8,  // 250
    BANDWIDTH_INDEX_500KHZ  = 9  // 500
} BANDWIDTH;

typedef enum {
    DRLORA_SF5 = 5,
    DRLORA_SF6 = 6,
    DRLORA_SF7 = 7,
    DRLORA_SF8 = 8,
    DRLORA_SF9 = 9,
    DRLORA_SF10 = 10,
    DRLORA_SF11 = 11,
    DRLORA_SF12 = 12
} SPREADING_FACTOR;

typedef enum {
    CRLORA_0FF    = 0,
    CRLORA_4_5    = 1,
    CRLORA_4_6    = 2,   // default?
    CRLORA_4_7    = 3,
    CRLORA_4_8    = 4,
    CRLORA_LI_4_5 = 5,
    CRLORA_LI_4_6 = 6,
    CRLORA_LI_4_8 = 7
} CODING_RATE;

#define DATA_RATE_SIZE              8

#define TX_POWER_OFFSET_MAX_SIZE    16

typedef enum {
    TXSTATUS_UNKNOWN,
    TXOFF,
    TXFREE,
    TXSCHEDULED,
    TXEMITTING,
    RXSTATUS_UNKNOWN,
    RXOFF,
    RXON,
    RXSUSPENDED
} STATUS;

typedef enum {
    CRC_STATUS_UNDEFINED  = 0x00,
    CRC_STATUS_NO_CRC     = 0x01,
    CRC_STATUS_CRC_BAD    = 0x11,
    CRC_STATUS_CRC_OK     = 0x10
} CRCSTATUS;

typedef enum {
    TEMP_SRC_EXT,   // the temperature has been measured with an external sensor
    TEMP_SRC_MCU    // the temperature has been measured by the gateway MCU
} TEMPERATURE_SRC;

#endif
