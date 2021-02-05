#include <string.h>
#include "errlist.h"

#define ERR_COUNT 23

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
  ERR_INVALID_JSON
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
