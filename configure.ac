AC_PREREQ([2.63])
AC_INIT([lorawan-network-server], [0.1], [andrey.ivanov@ikfia.ysn.ru])
AM_INIT_AUTOMAKE([1.11 foreign no-define subdir-objects])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_SRCDIR([lorawan-network-server.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])
build_mac=no
# Detect the target system
case "${host_os}" in
  darwin*)
    build_mac=yes
    ;;
esac
AM_CONDITIONAL([OS_MACH], [test "$build_mac" = "yes"])

AC_ARG_ENABLE([usejson],
[  --enable-lmdb    Turn on JSON file storage],
[case "${enableval}" in
  yes) usejson=true ;;
  no)  usejson=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-json]) ;;
esac],[usejson=true])
AM_CONDITIONAL([USEJSON], [test x$usejson = xtrue])

AC_ARG_ENABLE([uselmdb],
[  --enable-lmdb    Turn on LMDB],
[case "${enableval}" in
  yes) uselmdb=true ;;
  no)  uselmdb=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-lmdb]) ;;
esac],[uselmdb=false])
AM_CONDITIONAL([USELMDB], [test x$uselmdb = xtrue])

AC_ARG_ENABLE([usemdbx],
[  --enable-lmdb    Turn on MDBX],
[case "${enableval}" in
  yes) usemdbx=true ;;
  no)  usemdbx=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-mdbx]) ;;
esac],[usemdbx=false])
AM_CONDITIONAL([USEMDBX], [test x$usemdbx = xtrue])

# MacOS: install openssl into /usr/local/lib/opt/openssl@1.1
# brew install openssl@1.1
LDFLAGS="$LDFLAGS -L/usr/local/lib"

# Checks for header files. Add /usr/local/include for OS X.
CFLAGS="$CFLAGS -I/usr/local/include"

AC_OUTPUT
