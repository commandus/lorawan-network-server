#include <string.h>
#include "errlist.h"

#define ERR_COUNT 101

// used by strerror_lorawan_ns()
static const char *errList[ERR_COUNT] = {
	ERR_COMMAND_LINE,
	ERR_OPEN_DEVICE,
	ERR_CLOSE_DEVICE,
	ERR_BAD_STATUS,
	ERR_INVALID_PAR_LOG_FILE,
	ERR_INVALID_SERVICE,
	ERR_INVALID_GATEWAY_ID,
	ERR_INVALID_DEVICE_EUI,
	ERR_INVALID_BUFFER_SIZE,
	ERR_GRPC_NETWORK_SERVER_FAIL,
	ERR_INVALID_RFM_HEADER,
	ERR_INVALID_ADDRESS,
	ERR_INVALID_FAMILY,
	ERR_SOCKET_CREATE,
	ERR_SOCKET_BIND,
	ERR_SOCKET_OPEN,
	ERR_SOCKET_CLOSE,
	ERR_SOCKET_READ,
	ERR_SOCKET_WRITE,
	ERR_SOCKET_NO_ONE,
	ERR_SELECT,
	ERR_INVALID_PACKET,
	ERR_INVALID_JSON,
	ERR_DEVICE_ADDRESS_NOTFOUND,
	ERR_FAIL_IDENTITY_SERVICE,
	ERR_LMDB_TXN_BEGIN,
	ERR_LMDB_TXN_COMMIT,
	ERR_LMDB_OPEN,
	ERR_LMDB_CLOSE,
	ERR_LMDB_PUT,
	ERR_LMDB_PUT_PROBE,
	ERR_LMDB_GET,
	ERR_WRONG_PARAM,
	ERR_INSUFFICIENT_MEMORY,
	ERR_NO_CONFIG,
	ERR_SEND_ACK,
	ERR_NO_GATEWAY_STAT,
	ERR_INVALID_PROTOCOL_VERSION,
	ERR_PACKET_TOO_SHORT,
	ERR_PARAM_NO_INTERFACE,
	ERR_MAC_TOO_SHORT,
	ERR_MAC_INVALID,
	ERR_MAC_UNKNOWN_EXTENSION,
	ERR_PARAM_INVALID,
	ERR_INSUFFICIENT_PARAMS,
	ERR_NO_MAC_NO_PAYLOAD,
	ERR_INVALID_REGEX,
	ERR_NO_DATABASE,
	ERR_LOAD_PROTO,
	ERR_LOAD_DATABASE_CONFIG,
	ERR_DB_SELECT,
	ERR_DB_DATABASE_NOT_FOUND,
	ERR_DB_DATABASE_OPEN,
	ERR_DB_DATABASE_CLOSE,
	ERR_DB_CREATE,
	ERR_DB_INSERT,
	ERR_DB_START_TRANSACTION,
	ERR_DB_COMMIT_TRANSACTION,
	ERR_DB_EXEC,
	ERR_PING,
	ERR_PULLOUT,
	ERR_INVALID_STAT,
	ERR_NO_PAYLOAD,
	ERR_NO_MESSAGE_TYPE,
	ERR_QUEUE_EMPTY,
	ERR_RM_FILE,
	ERR_INVALID_BASE64,
	ERR_MISSED_DEVICE,
	ERR_MISSED_GATEWAY,
	ERR_INVALID_FPORT,
	ERR_INVALID_MIC,
	ERR_SEGMENTATION_FAULT,
    ERR_ABRT,
	ERR_BEST_GATEWAY_NOT_FOUND,
	ERR_REPLY_MAC,
	ERR_NO_MAC,
	ERR_NO_DEVICE_STAT,
	ERR_INIT_DEVICE_STAT,
	ERR_INIT_IDENTITY,
	ERR_INIT_QUEUE,
	ERR_HANGUP_DETECTED,
	ERR_NO_FCNT_DOWN,
	ERR_CONTROL_NOT_AUTHORIZED,
	ERR_GATEWAY_NOT_FOUND,
	ERR_CONTROL_DEVICE_NOT_FOUND,
	ERR_INVALID_CONTROL_PACKET,
    ERR_DUPLICATED_PACKET,
    ERR_INIT_GW_STAT,
    ERR_DEVICE_NAME_NOT_FOUND,
    ERR_GATEWAY_NO_YET_PULL_DATA,
    ERR_REGION_BAND_EMPTY,
    ERR_INIT_REGION_BANDS,
    ERR_INIT_REGION_NO_DEFAULT
};

const char *strerror_lorawan_ns
(
	int errcode
)
{
	if ((errcode <= -500) && (errcode >= -500 - ERR_COUNT))
	{
		return errList[-(errcode + 500)];
	}
	return strerror(errcode);
}


#define LOG_LEVEL_COUNT	8

static const char *logLevelList[LOG_LEVEL_COUNT] = 
{
	"",
	"fatal",
	"critical",
	"error",
	"warning",
	"info",
	"info",
	"debug"
};

static const char *logLevelColorList[LOG_LEVEL_COUNT] = 
{
	"",
	"0;31",
	"0;31",
	"0;31",
	"0;35 ",
	"",
	"",
	""
};

const char *logLevelString
(
	int logLevel
)
{
	if (logLevel < 0)
		logLevel = 0;
	if (logLevel > LOG_LEVEL_COUNT)
		logLevel = 0;
	return logLevelList[logLevel];
}

const char *logLevelColor
(
	int logLevel
)
{
	if (logLevel < 0)
		logLevel = 0;
	if (logLevel > LOG_LEVEL_COUNT)
		logLevel = 0;
	return logLevelColorList[logLevel];
}
