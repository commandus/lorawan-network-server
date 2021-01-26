#include <iostream>
#include <cstring>
#include <fstream>

#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>

#include "argtable3/argtable3.h"
#include "platform.h"
#include "utilstring.h"
#include "daemonize.h"

#include "errlist.h"
#include "utillora.h"
#include "utilstring.h"

#include "udp-emitter.h"
#include "config-json.h"

const std::string progname = "lorawan-network-server";
#define DEF_CONFIG_FILE_NAME ".lorawan-network-server"
#define DEF_TIME_FORMAT "%FT%T"

#define DEF_BUFFER_SIZE 4096
#define DEF_BUFFER_SIZE_S "4096"

Configuration *config = NULL;

static UDPEmitter emitter;

static void done()
{
  // destroy and free all
  if (config && config->serverConfig.verbosity > 1)
    std::cerr << "Semtech UDP packet emitter socket closed gracefully" << std::endl;
  exit(0);
}

static void stop()
{
  emitter.stopped = true;
}

void signalHandler(int signal)
{
  switch (signal)
  {
  case SIGINT:
    std::cerr << MSG_INTERRUPTED << std::endl;
    stop();
    done();
    break;
  default:
    break;
  }
}

#ifdef _MSC_VER
// TODO
void setSignalHandler()
{
}
#else
void setSignalHandler()
{
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = &signalHandler;
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGHUP, &action, NULL);
}
#endif

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd(
    UDPEmitter &emitter,
    int argc,
    char *argv[])
{
  // device path
  struct arg_str *a_address = arg_str1(NULL, NULL, "<host:port>", "destination host name or address and port");
  struct arg_int *a_size = arg_int0("s", "size", "<size>", "buffer size. Default " DEF_BUFFER_SIZE_S);

  struct arg_str *a_logfilename = arg_str0("l", "logfile", "<file>", "log file");
  struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "run daemon");
  struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
  struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
  struct arg_end *a_end = arg_end(20);

  void *argtable[] = {
      a_address, a_size,
      a_logfilename, a_daemonize, a_verbosity, a_help, a_end};

  int nerrors;

  // verify the argtable[] entries were allocated successfully
  if (arg_nullcheck(argtable) != 0)
  {
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }
  // Parse the command line as defined by argtable[]
  nerrors = arg_parse(argc, argv, argtable);

  config = new Configuration("");
  config->serverConfig.daemonize = a_daemonize->count > 0;
  config->serverConfig.verbosity = a_verbosity->count;
  if (a_size->count)
  {
    int sz = *a_size->ival;
    if (sz <= 0)
    {
      std::cerr << ERR_INVALID_BUFFER_SIZE << std::endl;
      nerrors++;
    }
    else
    {
      emitter.setBufferSize(sz);
    }
  }

    if (a_logfilename->count)
  {
  }

  if (!nerrors)
  {
    for (int i = 0; i < a_address->count; i++)
    {
      if (!emitter.setAddress(a_address->sval[i]))
      {
        nerrors++;
        break;
      }
    }
  }

  // special case: '--help' takes precedence over error reporting
  if ((a_help->count) || nerrors)
  {
    if (nerrors)
      arg_print_errors(stderr, a_end, progname.c_str());
    std::cerr << "Usage: " << progname << std::endl;
    arg_print_syntax(stderr, argtable, "\n");
    std::cerr << "UDP emitter" << std::endl;
    arg_print_glossary(stderr, argtable, "  %-25s %s\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
  return 0;
}

static void run()
{
  std::stringstream buf;
  int c = 0;
  while (!emitter.stopped)
  {
    struct sockaddr_in peerAddr;
    int r = emitter.receive(&peerAddr, 1);
    switch (r)
    {
    case ERR_CODE_COMMAND_LINE:
      std::cerr << ERROR << r << ": " << strerror_client(r) << std::endl;
      break;
    case -1: // timeout
      break;
    case 0: // ?!!
      break;
    default:
      if (emitter.isPeerAddr(&peerAddr))
      {
        emitter.sendUp(r);
      }
      else
      {
        emitter.setLastRemoteAddress(&peerAddr);
        emitter.sendDown(r);
      }
      break;
    }
  }
}

int main(
    int argc,
    char *argv[])
{
  if (parseCmd(emitter, argc, argv) != 0)
  {
    exit(ERR_CODE_COMMAND_LINE);
  }
#ifdef _MSC_VER
#else
  setSignalHandler();
#endif

  
  if (config->serverConfig.daemonize)
  {
    char wd[PATH_MAX];
    std::string progpath = getcwd(wd, PATH_MAX);
    if (config->serverConfig.verbosity > 1)
      std::cerr << MSG_DAEMON_STARTED << progpath << "/" << progname << MSG_DAEMON_STARTED_1 << std::endl;
    OPENSYSLOG()
    Daemonize daemonize(progname, progpath, run, stop, done);
    // CLOSESYSLOG()
  }
  else
  {
    setSignalHandler();
    run();
    done();
  }
  return 0;
}
