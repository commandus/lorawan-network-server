#include "utilusb.h"

#include <usb.h>

#define FTDI_VENDOR 0x0403
#define FTDI_PRODUCT 0x6001

/**
 * Vendor 0403 Product 6001
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
