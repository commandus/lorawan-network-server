#ifndef UTIL_DATE_H_
#define UTIL_DATE_H_	1

#include <string>
#include <inttypes.h>

/**
 * @param ms -1: do not add to the tail of formatted date
 */
std::string ltimeString(
	time_t value,
	int usec,
	const std::string &format
);

/**
 * Unix epoch time (seconds) or 2015-11-25T23:59:11
 */
time_t parseDate(const char *v);

/**
 * @return seconds since 1970, milliseconds in @param ms
 */
time_t time_ms(
	int &ms
);

time_t gps2utc(uint32_t value);
uint32_t utc2gps(time_t value);

std::string timeval2string(const struct timeval &val);

std::string time2string(time_t val);

void incTimeval(struct timeval &val, int seconds);

#endif