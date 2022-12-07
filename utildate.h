#ifndef UTIL_DATE_H_
#define UTIL_DATE_H_	1

#include <string>
#include <inttypes.h>

#ifdef _MSC_VER
#include <stdint.h> // portable: uint64_t   MSVC: __int64
#else
#include <sys/time.h>
#endif

/**
 * Format time local time
 * @param value Unix epoch time, seconds
 * @param usec -1: do not add to the tail of formatted date
 */
std::string ltimeString(
	time_t value,
	int usec,
	const std::string &format
);

/**
 * Format time GMT time
 * @param value Unix epoch time, seconds
 * @param usec -1: do not add to the tail of formatted date
 */
std::string gtimeString(
    time_t value,
    int usec,
    const std::string &format
);

/**
 * Parse NULL-terminated timstamp string
 * @return  Unix epoch time (seconds) or 2015-11-25T23:59:11
 */
time_t parseDate(const char *v);

/**
 * Get current system time, return seconds and milliseconds in the paramater ms.
 * @param ms return milliseconds
 * @return seconds since 1970, milliseconds in @param ms
 */
time_t time_ms(
	int &ms
);

/**
 * Convert GPS epoch time to the Unix epoch time
 * @return time in seconds since Unix epoch
 */ 
time_t gps2utc(uint32_t value);

/**
 * Convert Unix epoch time to the GPS epoch time
 * @return tume in seconds since GPS epoch
 */ 
uint32_t utc2gps(time_t value);

std::string timeval2string(const struct timeval &val);

/**
 * Format date and time
 * @return timestamp string
 */
std::string time2string(time_t val);

/**
 * Increment/decrement time
 * @param val value
 * @param seconds seconds
 * @param usec microseconds
 */

void incTimeval(
	struct timeval &val,
	int seconds,
	int usec = 0
);

#ifdef _MSC_VER
int gettimeofday(struct timeval* tp, struct timezone* tzp);
#endif

#endif
