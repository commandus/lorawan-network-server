#ifndef PLATFORM_H
#define PLATFORM_H 1

#ifdef _MSC_VER
#define	SYSLOG(msg)
#define OPENSYSLOG()
#define CLOSESYSLOG()
#else
#include <syslog.h>
#include <sstream>
#define	SYSLOG(msg) { syslog (LOG_ALERT, "%s", msg); }
#define OPENSYSLOG() { setlogmask (LOG_UPTO(LOG_NOTICE)); openlog("semtech_udp_packet_emitter", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1); }
#define CLOSESYSLOG() closelog();
#endif

#ifdef _MSC_VER
#define ALIGN	__declspec(align(1))
#define PACKED	
#else
#define ALIGN	
#define PACKED	__attribute__((aligned(1), packed))
#endif

#if BYTE_ORDER == BIG_ENDIAN
#define ntoh2(x) (x)
#define ntoh4(x) (x)
#define ntoh8(x) (x)
#else
#define ntoh2(x) be16toh(x)
#define ntoh4(x) be32toh(x)
#define ntoh8(x) be64toh(x)
#endif

#endif
