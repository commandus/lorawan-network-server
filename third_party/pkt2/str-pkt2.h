#ifndef STR_PKT2_H
#define STR_PKT2_H     1

#include <string>

/**
 * Initialize packet declarations
 * @param proto_path path to the catalog with protobuf decraration files
 * @param verbosity if 1, 2 or 3 print out to the stderr errors parsing declarations
 * @return structure of the packet declaratuions to be passed to the parsePacket()
 */
void* initPkt2(
	const std::string &proto_path,
	int verbosity
);

/**
 * Destroy and free packet declarations
 * @param env packet declaratuions
 */
void donePkt2(void *env);

/**
 * Parse packet by declaration
 * @param env packet declaratuions
 * @param inputFormat 0- binary, 1- hex string
 * @param outputFormat 0- json(default), 1- csv, 2- tab, 3- sql, 4- Sql, 5- pbtext, 6- dbg, 7- hex, 8- bin 
 * @param packet data
 * @param forceMessage "" If specifed, try only message type
 * @return empty string if fails
 */
std::string parsePacket(
	void *env, 
	int inputFormat,
	int outputFormat,
	const std::string &packet,
	const std::string &forceMessage
);

/**
 * Return field name list string
 * @param env contains options_cache
 * @param message_type
 * @param delimiter "\t" or ", "
 * @return field name list
 */
std::string headerFields(
	void *env, 
	const std::string &message_type,
	const std::string &delimiter
);

#endif