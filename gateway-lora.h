/*
    Semtech's Lora concentrator definitions
*/

#include <stdint.h>         // C99 types
#include <stdbool.h>        // bool type
#include <stdio.h>          // printf, fprintf, snprintf, fopen, fputs
#include <inttypes.h>       // PRIx64, PRIu64...

#include <string.h>         // memset
#include <signal.h>         // sigaction
#include <time.h>           // time, clock_gettime, strftime, gmtime
#include <sys/time.h>       // timeval
#include <unistd.h>         // getopt, access
#include <stdlib.h>         // atoi, exit
#include <errno.h>          // error messages
#include <math.h>           // modf

#include <sys/socket.h>     // socket specific definitions
#include <netinet/in.h>     // INET constants and stuff
#include <arpa/inet.h>      // IP address conversion stuff
#include <netdb.h>          // gai_strerror

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#ifdef __GNUC__
#define restrict __restrict__ // G++ has restrict
#else
#define restrict // C++ in general doesn't
#endif

#define _XOPEN_SOURCE 700

#endif

#include "packet_forwarder/loragw_hal.h"
#include "packet_forwarder/loragw_aux.h"
#include "packet_forwarder/loragw_reg.h"
#include "packet_forwarder/loragw_gps.h"

#include "packet_forwarder/jitqueue.h"

/**
 * spectral scan
 */
typedef struct spectral_scan_s {
    bool enable;            ///< enable spectral scan thread
    uint32_t freq_hz_start; ///< first channel frequency, in Hz
    uint8_t nb_chan;        ///< number of channels to scan (200kHz between each channel)
    uint16_t nb_scan;       ///< number of scan points for each frequency scan
    uint32_t pace_s;        ///< number of seconds between 2 scans in the thread
} spectral_scan_t;

#ifdef __cplusplus
}
#endif
