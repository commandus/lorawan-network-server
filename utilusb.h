#ifndef UTILUSBL_H_
#define UTILUSB_H_ 1

#include <string>
#include <vector>

/**
 * Return list of devices FTDI
 * @param retval return list of devices FTDI if not NULL
 * @return
 */
size_t ls_ftdi(std::vector<std::string> *retval);

#endif
