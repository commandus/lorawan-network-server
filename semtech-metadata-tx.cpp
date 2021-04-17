#include "semtech-metadata-tx.h"
#include <sstream>

/**
 * @see https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT
 * Section 6
 */
const char* METADATA_TX_NAMES[16] = {
	"txpk",	// 0 array name
	"imme", // bool   Send packet immediately (will ignore tmst & time)
 	"tmst", // number Send packet on a certain timestamp value (will ignore time)
 	"tmms", // number Send packet at a certain GPS time (GPS synchronization required)
 	"freq", // number TX central frequency in MHz (unsigned float, Hz precision)
 	"rfch", // number Concentrator "RF chain" used for TX (unsigned integer)
 	"powe", // number TX output power in dBm (unsigned integer, dBm precision)
 	"modu", // string Modulation identifier "LORA" or "FSK"
 	"datr", // string|number LoRa datarate identifier (eg. SF12BW500) | FSK datarate (unsigned, in bits per second)
 	"codr", // string LoRa ECC coding rate identifier
 	"fdev", // number FSK frequency deviation (unsigned integer, in Hz) 
 	"ipol", // bool   Lora modulation polarization inversion
 	"prea", // number RF preamble size (unsigned integer)
 	"size", // number RF packet payload size in bytes (unsigned integer)
 	"data", // string Base64 encoded RF packet payload, padding optional
 	"ncrc"  // bool   If true, disable the CRC of the physical layer (o
};

SemtechMetadataTX::SemtechMetadataTX()
    : sendimmediately(true), time2send(0), sendgps(false),
    modulation(LORA), bandwith(BW_125KHZ), spreadingFactor(DRLORA_SF11), codingRate(CRLORA_4_6),
    freq100(868900000), frequencydeviation(0), invertpolarization(false), preamblesize(0),
    data(""), disblecrc(false)
{
    rfch = 0;                  // Concentrator "RF chain" used for TX
    power = 0;                 // TX output power in dBm (unsigned integer, dBm precision)
}

std::string SemtechMetadataTX::toString() {

}
