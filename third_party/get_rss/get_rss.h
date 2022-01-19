/*
 * getrss.h
 * David Robert Nadeau
 * http://nadeausoftware.com/articles/2012/07/c_c_tip_how_get_process_resident_set_size_physical_memory_use
 * License: Creative Commons Attribution 3.0 Unported License
 *    http://creativecommons.org/licenses/by/3.0/deed.en_US
 */
 
#ifndef GET_RSS_H_
#define GET_RSS_H_

#ifdef __cplusplus
extern "C" {
#endif
/**
 * Returns the peak (maximum so far) resident set size (physical
 * memory use) measured in bytes, or zero if the value cannot be
 * determined on this OS.
 */
size_t getPeakRSS();

/**
 * Returns the current resident set size (physical memory use) measured
 * in bytes, or zero if the value cannot be determined on this OS.
 */
size_t getCurrentRSS();

#ifdef __cplusplus
}
#endif

#endif /* GET_RSS_H_ */
