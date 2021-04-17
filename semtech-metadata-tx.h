#ifndef SEMTECH_METADATA_TX_H_
#define SEMTECH_METADATA_TX_H_	1

#include <string>
#include "utillora.h"

class SemtechMetadataTX {
public:
    bool sendimmediately;       // imme   Send packet immediately (will ignore tmst & time)
    time_t time2send;           // tmst Send packet on a certain timestamp value (will ignore time)
    bool sendgps;               // tmms Send packet at a certain GPS time (GPS synchronization required)
    uint freq100;               // freq number TX central frequency in MHz (float, Hz precision)
    uint rfch;                  // Concentrator "RF chain" used for TX
    uint8_t power;              // TX output power in dBm (unsigned integer, dBm precision). // 0..15-> 8 10 12 13 14 16 18 20 21 24 26 27 29 30 33 36
    MODULATION modulation;      // string Modulation identifier "LORA" or "FSK"
	BANDWIDTH bandwith;
	SPREADING_FACTOR spreadingFactor;
 	CODING_RATE codingRate;     // string LoRa ECC coding rate identifier
    
    uint frequencydeviation;    // FSK frequency deviation, Hz
    bool invertpolarization;    // Lora modulation polarization inversion
    uint preamblesize;          // RF preamble size
    std::string data;           // string Base64 encoded RF packet payload, padding optional; size- RF packet payload size in bytes
    bool disblecrc;             // If true, disable the CRC of the physical layer

    SemtechMetadataTX();
    std::string toString();
};

#endif
