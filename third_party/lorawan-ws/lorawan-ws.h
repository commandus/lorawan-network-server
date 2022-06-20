/*
 * @file lorawan-ws.h
 */

#ifndef LORAWANWS_H_
#define LORAWANWS_H_	1

#include <map>
#include <string>
#include <functional>
#include <iostream>
#include <fstream>

#include "db-intf.h"

#define MHD_START_FLAGS 	MHD_USE_POLL | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_SUPPRESS_DATE_NO_CLOCK | MHD_USE_TCP_FASTOPEN | MHD_USE_TURBO

typedef std::map<std::string, DatabaseIntf*> MAP_NAME_DATABASE;

typedef std::function<void(
		void *env,
		int level,
		int modulecode,
		int errorcode,
		const std::string &message)> LOG_CALLBACK;


typedef struct {
	unsigned int threadCount;
	unsigned int connectionLimit;
	unsigned int flags;

	// listener port
	int port;
	// last error code
	int lasterr;
	// html root
	const char* dirRoot;
	// log verbosity
	int verbosity;
	// web server descriptor
	void *descriptor;
	// databases
	MAP_NAME_DATABASE databases;

	LOG_CALLBACK onLog;
} WSConfig;

void setLogCallback(LOG_CALLBACK value);

/**
 * @param threadCount threads count, e.g. 2
 * @param connectionLimit mex connection limit, e.g. 1000
 * @param flags e.g. MHD_SUPPRESS_DATE_NO_CLOCK | MHD_USE_DEBUG | MHD_USE_SELECT_INTERNALLY
 * @param config listener descriptors, port number
 */ 
bool startWS(
	WSConfig &config
);

void doneWS(
	WSConfig &config
);

#endif
