#include "utilusb.h"

#include <usb.h>

#define FTDI_VENDOR 0x0483
#define FTDI_PRODUCT 0x5740

/**
 * Vendor 0403 Product 6001
 * 0483:5740 bus 003 dev 006
 * 0483:5740 STMicroelectronics Virtual COM Port
 * @return count if FTDI USB devices
 */
size_t ls_ftdi(std::vector<std::string> *retval) {

    size_t r = 0;
    struct usb_bus *bus;
    struct usb_device *dev;
    usb_init();
    usb_find_busses();
    usb_find_devices();
    for (bus = usb_busses; bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if ((dev->descriptor.idVendor == FTDI_VENDOR) && (dev->descriptor.idProduct == FTDI_PRODUCT)) {
                if (retval) {
                    retval->push_back(dev->filename);
                }
                r++;
            }
        }
    }
    return r;
}
