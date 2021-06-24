/*
 * utilfile.h
 */
#ifndef UTIL_FILE_H
#define UTIL_FILE_H     1

#include <string>
#include <vector>

namespace config {
	bool rmDir(const std::string &path);
	bool rmFile(const std::string &fn);

	/**
	 * Return list of files in specified path
	 * @param path
	 * @param flags 0- as is, 1- full path, 2- relative (remove parent path)
	 * @param retval can be NULL
	 * @return count files
	 * FreeBSD fts.h fts_*()
	 */
	size_t filesInPath
	(
		const std::string &path,
		const std::string &suffix,
		int flags,
		std::vector<std::string> *retval
	);
}

/**
 * @return last modification file time, seconds since unix epoch
 */
time_t fileModificationTime(
	const std::string &fileName
);

#endif
