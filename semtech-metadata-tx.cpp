#include "semtech-metadata-tx.h"
#include <sstream>

SemtechMetadataTX::SemtechMetadataTX()
    : sendimmediately(true), time2send(0), sendgps(false),
      modulation(LORA), bandwith(BANDWIDTH_INDEX_125KHZ), spreadingFactor(DRLORA_SF11), codingRate(CRLORA_4_6),
      freq100(868900000), frequencydeviation(0), invertpolarization(false), preamblesize(0),
      data(""), disblecrc(false)
{
    rfch = 0;                  // Concentrator "RF chain" used for TX
    power = 0;                 // TX output power in dBm (unsigned integer, dBm precision)
}

std::string SemtechMetadataTX::toString() {
	return "";
}
