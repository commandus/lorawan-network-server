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
#include <auth-user.h>

#include "db-intf.h"

typedef std::map<std::string, DatabaseIntf*> MAP_NAME_DATABASE;

#define MHD_START_FLAGS 	MHD_USE_POLL | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_SUPPRESS_DATE_NO_CLOCK | MHD_USE_TCP_FASTOPEN | MHD_USE_TURBO

/**
 * Log callback function prototype
 * @param env WSConfig
 * @param level 0- error, 1- warning,..
 * @param modulecode always 200
 * @param errorcode 0- no error (warning, info)
 * @param message optional error description
 */
typedef std::function<void(
    void *env,
    int level,
    int modulecode,
    int errorcode,
    const std::string &message
)> LOG_CALLBACK;

/**
 * Special path handler provided by host
 * @param content return value
 * @param contentType, return content type. By default "text/javascript;charset=UTF-8"
 * @param env config
 * @param modulecode always 200
 * @param url HTTP request URL
 * @param method HTTP request method e.g. "GET"
 * @param version HTTP request version e.g. "1.0"
 * @param upload_data HTTP request uploaded data
 * @param upload_data_size HTTP request uploaded data size, bytes
 * @param authorized true- user authorized successfully
 * @return true- OK
  */
class WebServiceRequestHandler {
public:
    virtual bool handle(
        std::string &content,
        std::string &contentType,
        void *env,
        int modulecode,
        const char *path,
        const char *method,
        const char *version,
        std::map<std::string, std::string> &params,
        const char *upload_data,
        size_t *upload_data_size,
        bool authorized
    ) = 0;
};

/**
 * Configuration to start up web service
 */
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
    WebServiceRequestHandler *onSpecialPathHandler;

    // Authorization
    void *jwt;	// JWT verifier descriptor
    std::string issuer;
    std::string secret;
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
