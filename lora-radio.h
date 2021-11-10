#ifndef LORA_RADIO_H_
#define LORA_RADIO_H_	1

typedef enum {
	LORA = 0,
	FSK = 1
} MODULATION;

/**
 * TODO nor sure for BW_7KHZ..BW_125KHZ
 * @see https://github.com/x893/SX1231/blob/master/SX12xxDrivers-2.0.0/src/radio/sx1276-LoRa.c
 * SignalBw
 * 0: 7.8kHz, 1: 10.4 kHz, 2: 15.6 kHz, 3: 20.8 kHz, 4: 31.2 kHz,
 * 5: 41.6 kHz, 6: 62.5 kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz, other: Reserved
 */ 
typedef enum {
    BW_7KHZ  = 0,    // 7.8
    BW_10KHZ  = 1,   // 10.4
    BW_15KHZ  = 2,   // 15.6
    BW_20KHZ  = 3,   // 20.8
    BW_31KHZ  = 4,   // 31.2
    BW_41KHZ  = 5,   // 41.6
    BW_62KHZ  = 6,   // 62.5
    BW_125KHZ  = 7,   // 125

    BW_200KHZ  = 8, // 9 -> 300?
    BW_400KHZ  = 10,
    BW_800KHZ  = 12,    // default?
    BW_1600KHZ = 13
} BANDWIDTH;

typedef enum {
    DRLORA_SF5 = 5,
    DRLORA_SF6,
    DRLORA_SF7,
    DRLORA_SF8,
    DRLORA_SF9,
    DRLORA_SF10,
    DRLORA_SF11,
    DRLORA_SF12
} SPREADING_FACTOR;

typedef enum {
    CRLORA_4_5    = 0x01,
    CRLORA_4_6    = 0x02,   // default?
    CRLORA_4_7    = 0x03,
    CRLORA_4_8    = 0x04,
    CRLORA_LI_4_5 = 0x05,
    CRLORA_LI_4_6 = 0x06,
    CRLORA_LI_4_8 = 0x07
} CODING_RATE;

#define DATA_RATE_SIZE 8

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
    STAT_UNDEFINED  = 0x00,
    STAT_NO_CRC     = 0x01,
    STAT_CRC_BAD    = 0x11,
    STAT_CRC_OK     = 0x10
} CRCSTATUS;

typedef enum {
    TEMP_SRC_EXT,   /* the temperature has been measured with an external sensor */
    TEMP_SRC_MCU    /* the temperature has been measured by the gateway MCU */
} TEMPERATURE_SRC;

#endif
