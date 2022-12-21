#include "subst-call-c.h"

#include <stdarg.h>
#include <unistd.h>

#include "libloragw-helper.h"

// libloragw-helper.cpp must be linked
extern LibLoragwHelper *globalLibLoragwHelper;

extern "C" int open_c(const char *file, int flags)
{
    if (!globalLibLoragwHelper) {
        return -1;
    }
    return globalLibLoragwHelper->open(file, flags);
}

extern "C" int close_c (int fd)
{
    if (!globalLibLoragwHelper)
        return -1;
    return globalLibLoragwHelper->close(fd);
}

// printf
// @see https://stackoverflow.com/questions/51684894/how-do-i-implement-my-own-printf-in-c

int ft_putchar(int c) {
    if (!globalLibLoragwHelper)
        return 1;
    return globalLibLoragwHelper->log((char)c );
}

static int ft_printf_aux(const char *fmt, va_list ap, int len);

static int ft_print_c(const char *fmt, va_list ap, int len) {
    int c = va_arg(ap, int);
    ft_putchar(c);
    return ft_printf_aux(fmt, ap, len + 1);
}

static int ft_putnum(unsigned long long n, unsigned int base, const char *digits) {
    int res = 1;
    if (n >= base)
        res += ft_putnum(n / base, base, digits);
    ft_putchar(digits[n % base]);
    return res;
}

static int ft_print_d(const char *fmt, va_list ap, int len) {
    int n = va_arg(ap, int);
    unsigned long long u;
    if (n < 0) {
        ft_putchar('-');
        len++;
        u = -(unsigned)n;
    } else {
        u = n;
    }
    len += ft_putnum(u, 10, "0123456789");
    return ft_printf_aux(fmt, ap, len);
}

static int ft_print_o(const char *fmt, va_list ap, int len) {
    unsigned int n = va_arg(ap, unsigned int);
    len += ft_putnum(n, 8, "01234567");
    return ft_printf_aux(fmt, ap, len);
}

static int ft_print_u(const char *fmt, va_list ap, int len) {
    unsigned int n = va_arg(ap, unsigned int);
    len += ft_putnum(n, 10, "0123456789");
    return ft_printf_aux(fmt, ap, len);
}

static int ft_print_x(const char *fmt, va_list ap, int len) {
    unsigned int n = va_arg(ap, unsigned int);
    len += ft_putnum(n, 16, "0123456789abcdef");
    return ft_printf_aux(fmt, ap, len);
}

static int ft_print_X(const char *fmt, va_list ap, int len) {
    unsigned int n = va_arg(ap, unsigned int);
    len += ft_putnum(n, 16, "0123456789ABCDEF");
    return ft_printf_aux(fmt, ap, len);
}

static int ft_print_s(const char *fmt, va_list ap, int len) {
    const char *s = va_arg(ap, const char *);
    if (s == NULL) {
        s = "(null)";
    }
    while (*s) {
        ft_putchar(*s++);
        len++;
    }
    return ft_printf_aux(fmt, ap, len);
}

typedef int (*ft_print_dispatch_f)(const char *fmt, va_list ap, int len);

ft_print_dispatch_f ft_print_dispatch(char v)
{
    switch (v) {
    case 'c':
        return ft_print_c;
    case 'd':
    case 'i':
        return ft_print_d;
    case 'o':
        return ft_print_o;
    case 'u':
        return ft_print_u;
    case 'x':
        return ft_print_x;
    case 'X':
        return ft_print_X;
    case 's':
        return ft_print_s;
    default:
        return nullptr;
    }
};

static int ft_printf_aux(const char *fmt, va_list ap, int len) {
    int c;

    while (*fmt) {
        c = (unsigned char)*fmt++;
        if (c != '%') {
            ft_putchar(c);
            len++;
        } else {
            c = (unsigned char)*fmt++;
            if (ft_print_dispatch(c) == nullptr) {
                if (c == '\0')
                    break;
                ft_putchar(c);
                len++;
            } else {
                return ft_print_dispatch(c)(fmt, ap, len);
            }
        }
    }
    return len;
}

extern "C" int printf_c(const char* format, ... )
{
    va_list ap;
    int n;
    va_start(ap, format);
    n = ft_printf_aux(format, ap, 0);
    va_end(ap);
    return n;
}
