#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#endif

#include "utildate.h"
#include <time.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>
#include "platform.h"

#ifdef _MSC_VER
#include "strptime.h"
#else
#include <sys/time.h>
#include "strptime.h"
#define TMSIZE sizeof(struct tm)
#define localtime_s(tm, time) memmove(tm, localtime(time), TMSIZE)
#define gmtime_s(tm, time) memmove(tm, gmtime(time), TMSIZE)
#endif

const static char *dateformat0 = "%FT%T";
const static char *dateformat1 = "%FT%T%Z";
const static char *dateformat2 = "%F %T %Z";

/**
 * Return formatted time stamp
 * @param value tm structure, local ot GMT
 * @param usec microseconds
 * @param format format template string
 * @return time stamp
 */
static std::string TM2String(
    const struct tm &value,
    int usec,
    const std::string &format
)
{
    char dt[64];
    strftime(dt, sizeof(dt), format.c_str(), &value);
    if (usec == -1)
        return std::string(dt);
    else {
        std::stringstream ss;
        ss << std::string(dt) << "." << std::setw(6) << std::setfill('0') << usec;
        return ss.str();
    }
}

/**
 * Return formatted time stamp local time
 * @param value seconds
 * @param usec microseconds
 * @param format format template string
 * @return time stamp
 */
std::string ltimeString(
	time_t value,
	int usec,
	const std::string &format
) {
	if (!value)
		value = time(NULL);
	struct tm tm;
	localtime_s(&tm, &value);
    return TM2String(tm, usec, format);
}

/**
 * Return formatted time stamp local time
 * @param value seconds
 * @param usec microseconds
 * @param format format template string
 * @return time stamp
 */
std::string gtimeString(
        time_t value,
        int usec,
        const std::string &format
) {
    if (!value)
        value = time(NULL);
    struct tm tm;
    gmtime_s(&tm, &value);
    return TM2String(tm, usec, format);
}

/**
 * Unix epoch time (seconds) or 2015-11-25T23:59:11
 */
time_t parseDate(const char *v)
{
	struct tm tmd;
	memset(&tmd, 0, sizeof(struct tm));

	time_t r;
	if ((strptime(v, dateformat0, &tmd) == NULL) 
		&& (strptime(v, dateformat1, &tmd) == NULL)
		&& (strptime(v, dateformat2, &tmd) == NULL)
		)
			r = strtol(v, NULL, 0);
	else
			r = mktime(&tmd);
	return r;
}

time_t time_ms(int &ms) {
	struct timeval tp;
	gettimeofday(&tp, NULL);
	ms = tp.tv_usec / 1000;
	return tp.tv_sec;
}

// https://github.com/davidcalhoun/gps-time.js/blob/master/gps-time.js

// Difference in time between Jan 1, 1970 (Unix epoch) and Jan 6, 1980 (GPS epoch).
static const uint32_t gpsUnixEpochDiff = 315964800;

static const uint32_t gpsLeap[] = {
	46828800, 78364801, 109900802, 173059203, 252028804, 315187205, 346723206,
	393984007, 425520008, 457056009, 504489610, 551750411, 599184012, 820108813,
	914803214, 1025136015, 1119744016, 1167264017	// last is 2017-Jan-01
};

/**
 * Determines whether a leap second should be added.  Logic works slightly differently
 * for unix->gps and gps->unix.
 * @param {number} gpsMS GPS time in milliseconds.
 * @param {number} curGPSLeapMS Currently leap represented in milliseconds.
 * @param {number} totalLeapsMS Total accumulated leaps in milliseconds.
 * @param {boolean} isUnixToGPS True if this operation is for unix->gps, falsy if gps->unix.
 * @return {boolean} Whether a leap second should be added.
 */
static bool shouldAddLeap (
	uint32_t gps,
	uint32_t curGPSLeap,
	uint32_t totalLeaps,
	bool isUnixToGPS
) {
	if (isUnixToGPS) {
    	// for unix->gps
    	return gps >= curGPSLeap - totalLeaps;
  } else {
    	// for gps->unix
    	return gps >= curGPSLeap;
  }
}

/**
 * Counts the leaps from the GPS epoch to the inputted GPS time.
 * @param {number} gpsMS GPS time in milliseconds.
 * @param {boolean} isUnixToGPS
 * @return {number}
 */
static uint32_t countLeaps(
	uint32_t gps,
	bool isUnixToGPS
) 
{
	int numLeaps = 0;
	for (int i = 0; i < sizeof(gpsLeap) / sizeof(uint32_t); i++) {
    	if (shouldAddLeap(gps, gpsLeap[i], i, isUnixToGPS)) {
	    	numLeaps++;
    	}
  	}
	return numLeaps;
}

time_t gps2utc(
	 uint32_t value
) {
	// Epoch diff adjustment.
  	uint32_t r = value + gpsUnixEpochDiff;
	// Account for leap seconds between 1980 epoch and gpsMS.
	r -= countLeaps(value, false);
	return r;
}

uint32_t utc2gps(time_t value) {
	// Epoch diff adjustment.
	time_t gps = value - gpsUnixEpochDiff;
	// Account for leap seconds between 1980 epoch and gpsMS.
	gps += countLeaps((uint32_t) gps, true);
	return (uint32_t) gps;
}

std::string timeval2string(
	const struct timeval &val
)
{
	char buf[64];
	const time_t t = val.tv_sec;	// time_t 32/64 bits
	struct tm *tm = localtime(&t);
	strftime(buf, sizeof(buf), dateformat0, tm);
	std::stringstream ss;
	ss << buf << "." << std::setw(6) << std::setfill('0') << val.tv_usec;
	return ss.str();
}

std::string time2string(
	time_t val
)
{
	char buf[64];
	struct tm *tm = localtime(&val);
	strftime(buf, sizeof(buf), dateformat1, tm);
	return std::string(buf);
}

void incTimeval(
	struct timeval &val,
	int seconds,
	int usec
)
{
	val.tv_sec += seconds;
	if (usec > 0) {
		val.tv_usec += usec;
		if (val.tv_usec >= 1000000) {
			val.tv_sec++;
			val.tv_usec -= 1000000;
		}
	} else {
		if (usec < 0) {
			if (val.tv_usec < 0) {
				val.tv_sec--;
				val.tv_usec = 1000000 + val.tv_usec;
			}
		}
	}
}
