#include <string>
#include <iostream>

#include <google/protobuf/message.h>

#include "pkt2/str-pkt2.h"

int main(int argc, char **argv) {
	std::string protoPath = "proto";
	void* env = initPkt2(protoPath, 0);
	if (!env) {
		std::cerr << "Init error" << std::endl;
		exit(1);
	}

	std::string hexData = "01004e01001c9a0ba5f633303032333430363032333533343000011900005ab8f59303000b003e68a68143d40000000502001e0810003e01b21200004e812b4e160000390000221400829486247a0d1c09";
	std::string mt = "iridium.IEPacket"; // iridium.IE_Packet

	std::map<std::string, std::string> tableAliases;
	tableAliases["iridium.IEPacket"] = "iridium_packet";
	std::map<std::string, std::string> fieldAliases;
	fieldAliases["iridium.IEPacket.iridium_version"] = "version";
	fieldAliases["iridium.IEPacket.iridium_size"] = "";
	fieldAliases["iridium.IEIOHeader.cdrref"] = "cddref";
	fieldAliases["iridium.IEIOHeader.imei"] = "imei";
	fieldAliases["iridium.IEIOHeader.status"] = "status";
	fieldAliases["iridium.IEIOHeader.recvno"] = "recvno";
	fieldAliases["iridium.IEIOHeader.sentno"] = "sentno";
	fieldAliases["iridium.IEIOHeader.recvtime"] = "recvtime";
	fieldAliases["iridium.IELocation.iridium_latitude"] = "iridium_latitude";
	fieldAliases["iridium.IELocation.iridium_longitude"] = "iridium_longitude";
	fieldAliases["iridium.IELocation.cepradius"] = "cepradius";
	fieldAliases["iridium.GPSCoordinates.latitude"] = "gps_latitude";
	fieldAliases["iridium.GPSCoordinates.longitude"] = "gps_longitude";
	fieldAliases["iridium.GPSCoordinates.hdop"] = "";
	fieldAliases["iridium.GPSCoordinates.pdop"] = "";
	fieldAliases["iridium.Time5.time5"] = "gps_time";
	fieldAliases["iridium.Packet8.gpsolddata"] = "";
	fieldAliases["iridium.Packet8.gpsencoded"] = "";
	fieldAliases["iridium.Packet8.gpsfrommemory"] = "";
	fieldAliases["iridium.Packet8.gpsnoformat"] = "";
	fieldAliases["iridium.Packet8.gpsnosats"] = "";
	fieldAliases["iridium.Packet8.gpsbadhdop"] = "";
	fieldAliases["iridium.Packet8.gpstime"] = "";
	fieldAliases["iridium.Packet8.gpsnavdata"] = "";
	fieldAliases["iridium.Packet8.satellite_visible_count"] = "";
	fieldAliases["iridium.Packet8.battery_voltage"] = "";
	fieldAliases["iridium.Packet8.battery_voltage"] = "";
	fieldAliases["iridium.Packet8.battery_low"] = "";
	fieldAliases["iridium.Packet8.battery_high"] = "";
	fieldAliases["iridium.Packet8.temperature_c"] = "";
	fieldAliases["iridium.Packet8.reserved"] = "";
	fieldAliases["iridium.Packet8.failurepower"] = "";
	fieldAliases["iridium.Packet8.failureeep"] = "";
	fieldAliases["iridium.Packet8.failureclock"] = "";
	fieldAliases["iridium.Packet8.failurecable"] = "";
	fieldAliases["iridium.Packet8.failureint0"] = "";
	fieldAliases["iridium.Packet8.software_failure"] = "";
	fieldAliases["iridium.Packet8.failurewatchdog"] = "";
	fieldAliases["iridium.Packet8.failurenoise"] = "";
	fieldAliases["iridium.Packet8.failureworking"] = "";
	fieldAliases["iridium.Packet8.key"] = "";

	for (int i = OUTPUT_FORMAT_JSON; i < OUTPUT_FORMAT_BIN; i++) {
		std::string s = parsePacket(env, INPUT_FORMAT_HEX, i, hexData, mt, &tableAliases, &fieldAliases);
		std::cout << i << ": " << s << std::endl;
	}

	google::protobuf::Message *msg;
	parsePacket2ProtobufMessage((void **) &msg, env, INPUT_FORMAT_HEX, hexData, mt, &tableAliases, &fieldAliases);

	if (msg) {
		std::cout << msg->DebugString() << std::endl;
		delete msg;
	}

	/*
	std::string flds = headerFields(env, mt, ", ");
	std::cout << flds << std::endl;
	*/
	donePkt2(env);
	
}
