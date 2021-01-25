#include "utildate.h"
#include <time.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <sys/time.h>
#include "platform.h"

#ifndef _MSC_VER
#include "strptime.h"
#define TMSIZE sizeof(struct tm)
#define localtime_s(tm, time) memmove(tm, localtime(time), TMSIZE)
#endif

const static char *dateformat = "%FT%T";

std::string ltimeString(
	time_t value,
	int ms,
	const std::string &format
) {
	if (!value)
		value = time(NULL);
	struct tm tm;
	localtime_s(&tm, &value);
	char dt[64];
	strftime(dt, sizeof(dt), format.c_str(), &tm);
	if (ms == -1)
		return std::string(dt);
	else {
		std::stringstream ss;
		ss << dt << "." << std::setw(5) << std::setfill('0') << ms;
		return ss.str();
	}
}

/**
 * Unix epoch time (seconds) or 2015-11-25T23:59:11
 */
time_t parseDate(const char *v)
{
	struct tm tmd;
	memset(&tmd, 0, sizeof(struct tm));

	time_t r;
	if (strptime(v, dateformat, &tmd) == NULL)
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
  914803214, 1025136015, 1119744016, 1167264017
};

/**
 * Determines whether a leap second should be added.  Logic works slightly differently
 * for unix->gps and gps->unix.
 * @param {number} gpsMS GPS time in milliseconds.
 * @param {number} curGPSLeapMS Currenly leap represented in milliseconds.
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
};

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
};

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
	uint32_t gps = value - gpsUnixEpochDiff;
	// Account for leap seconds between 1980 epoch and gpsMS.
	gps += countLeaps(gps, true);
	return gps;
}
