db_table_aliases = [
	["iridium.IEPacket", "iridium_packet"]
];

db_field_aliases = [
	["iridium.IEPacket", "iridium_packet"],
	["iridium.IEPacket.iridium_version", "version"],
	["iridium.IEPacket.iridium_size", ""],
	["iridium.IEIOHeader.cdrref", "cddref"],
	["iridium.IEIOHeader.imei", "imei"],
	["iridium.IEIOHeader.status", "status"],
	["iridium.IEIOHeader.recvno", "recvno"],
	["iridium.IEIOHeader.sentno", "sentno"],
	["iridium.IEIOHeader.recvtime", "recvtime"],
	["iridium.IELocation.iridium_latitude", "iridium_latitude"],
	["iridium.IELocation.iridium_longitude", "iridium_longitude"],
	["iridium.IELocation.cepradius", "cepradius"],
	["iridium.GPSCoordinates.latitude", "gps_latitude"],
	["iridium.GPSCoordinates.longitude", "gps_longitude"],
	["iridium.GPSCoordinates.hdop", ""],
	["iridium.GPSCoordinates.pdop", ""],
	["iridium.Time5.time5", "gps_time"],
	["iridium.Packet8.gpsolddata", ""],
	["iridium.Packet8.gpsencoded", ""],
	["iridium.Packet8.gpsfrommemory", ""],
	["iridium.Packet8.gpsnoformat", ""],
	["iridium.Packet8.gpsnosats", ""],
	["iridium.Packet8.gpsbadhdop", ""],
	["iridium.Packet8.gpstime", ""],
	["iridium.Packet8.gpsnavdata", ""],
	["iridium.Packet8.satellite_visible_count", ""],
	["iridium.Packet8.battery_voltage", ""],
	["iridium.Packet8.battery_voltage", ""],
	["iridium.Packet8.battery_low", ""],
	["iridium.Packet8.battery_high", ""],
	["iridium.Packet8.temperature_c", ""],
	["iridium.Packet8.reserved", ""],
	["iridium.Packet8.failurepower", ""],
	["iridium.Packet8.failureeep", ""],
	["iridium.Packet8.failureclock", ""],
	["iridium.Packet8.failurecable", ""],
	["iridium.Packet8.failureint0", ""],
	["iridium.Packet8.software_failure", ""],
	["iridium.Packet8.failurewatchdog", ""],
	["iridium.Packet8.failurenoise", ""],
	["iridium.Packet8.failureworking", ""],
	["iridium.Packet8.key", ""]
];

databases = [
	{
		name: "sqlite",
		type: "sqlite3",
		connection: "lns.data.sqlite.db",
		table_aliases: db_table_aliases,
		field_aliases: db_field_aliases
	},
	{
		name: "postgres",
		type: "postgresql",
		connection: "host=localhost;database=irthermometer;username=irthermometer;password=Rfhnjirf20",
		table_aliases: db_table_aliases,
		field_aliases: db_field_aliases
	}
];
