#ifndef PLATFORM_H
#define PLATFORM_H 1

#ifdef _MSC_VER
#define	SYSLOG(msg)
#define OPEN_SYSLOG()
#define CLOSE_SYSLOG()
#else
#include <syslog.h>
#include <sstream>
#define	SYSLOG(level, msg) { syslog (level, "%s", msg); }
#define OPEN_SYSLOG(NAME) { setlogmask (LOG_UPTO(LOG_NOTICE)); openlog(NAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1); }
#define CLOSE_SYSLOG() closelog();
#endif

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif
#endif

#define SWAP_BYTES_2(x) be16toh(x)
#define SWAP_BYTES_4(x) be32toh(x)
#define SWAP_BYTES_8(x) be64toh(x)

#if BYTE_ORDER == BIG_ENDIAN
#define NTOH2(x) (x)
#define NTOH4(x) (x)
#define NTOH8(x) (x)
#define HTON2(x) (x)
#define HTON4(x) (x)
#define HTON8(x) (x)
#else
#define NTOH2(x) be16toh(x)
#define NTOH4(x) be32toh(x)
#define NTOH8(x) be64toh(x)
#define HTON2(x) htobe16(x)
#define HTON4(x) htobe32(x)
#define HTON8(x) htobe64(x)
#endif

#ifdef __MACH__
#include <libkern/OSByteOrder.h>
#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)
#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)
#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)
#endif

#endif
