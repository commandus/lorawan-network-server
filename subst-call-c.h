/**
 * Add next lines to the ~/git/rak_common_for_gateway/lora/rak2287/sx1302_hal/libloragw/inc/config.h:
 * #include "subst-call-c.h"
 * #define open open_c
 * #define close close_c
 * #define printf printf_c
 * #define fprintf(fd, args...) printf_c(args)
 *
*/
#ifdef __cplusplus
extern "C" {
#endif

int open_c(const char *file, int flags);

int close_c (int fd);

int printf_c(const char* format, ... );

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#endif
