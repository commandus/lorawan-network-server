#include <string.h>
#include "errlist.h"

#define ERR_COUNT 11

static const char* errlist[ERR_COUNT] = {
  ERR_COMMAND_LINE,
  ERR_OPEN_SOCKET,
  ERR_CLOSE_SOCKET,
  ERR_BAD_STATUS,
  ERR_INVALID_PAR_LOG_FILE,
  ERR_INVALID_SERVICE,
  ERR_INVALID_GATEWAY_ID,
  ERR_INVALID_DEVICE_EUI,
  ERR_INVALID_BUFFER_SIZE,
  ERR_GRPC_NETWORK_SERVER_FAIL,
  ERR_INVALID_RFM_HEADER
};

const char *strerror_client(
  int errcode
)
{
  if ((errcode <= -500) && (errcode >= -500 - ERR_COUNT)) {
    return errlist[-(errcode + 500)];
  }
  return strerror(errcode);
}
