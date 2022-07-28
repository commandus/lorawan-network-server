AC_PREREQ([2.63])
AC_INIT([lorawan-network-server], [0.2], [andrey.ivanov@ikfia.ysn.ru])
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

AC_ARG_ENABLE([pkt2],
[  --enable-pkt2    Turn on PKT2 packet parser],
[case "${enableval}" in
  yes) pkt2=true ;;
  no)  pkt2=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-pkt2]) ;;
esac],[pkt2=false])
AM_CONDITIONAL([ENABLE_PKT2], [test x$pkt2 = xtrue])

AC_ARG_ENABLE([logger-huffman],
[  --enable-logger-huffman    Turn on logger-huffman packet parser],
[case "${enableval}" in
  yes) logger_huffman=true ;;
  no)  logger_huffman=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-logger-huffman]) ;;
esac],[logger_huffman=false])
AM_CONDITIONAL([ENABLE_LOGGER_HUFFMAN], [test x$logger_huffman = xtrue])

AC_ARG_ENABLE([jwt],
[  --enable-jwt    Turn on JWT web service authorization],
[case "${enableval}" in
  yes) enable_jwt=true ;;
  no)  enable_jwt=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-jwt]) ;;
esac],[enable_jwt=false])
AM_CONDITIONAL([ENABLE_JWT], [test x$enable_jwt = xtrue])

AC_ARG_ENABLE([json],
[  --enable-json    Turn on JSON file storage],
[case "${enableval}" in
  yes) enable_json=true ;;
  no)  enable_json=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-json]) ;;
esac],[enable_json=true])
AM_CONDITIONAL([ENABLE_JSON], [test x$enable_json = xtrue])

AC_ARG_ENABLE([lmdb],
[  --enable-lmdb    Turn on LMDB],
[case "${enableval}" in
  yes) enable_lmdb=true ;;
  no)  enable_lmdb=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-lmdb]) ;;
esac],[enable_lmdb=false])
AM_CONDITIONAL([ENABLE_LMDB], [test x$enable_lmdb = xtrue])

AC_ARG_ENABLE([mdbx],
[  --enable-mdbx    Turn on MDBX],
[case "${enableval}" in
  yes) enable_mdbx=true ;;
  no)  enable_mdbx=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-mdbx]) ;;
esac],[enable_mdbx=false])
AM_CONDITIONAL([ENABLE_MDBX], [test x$enable_mdbx = xtrue])

# backend databases

# by default SQLite is on
AC_ARG_ENABLE([db-sqlite],
[  --enable-db-sqlite    Turn on SQLite backend],
[case "${enableval}" in
  yes) db_sqlite=true ;;
  no)  db_sqlite=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-db-sqlite]) ;;
esac],[db_sqlite=true])
AM_CONDITIONAL([ENABLE_DB_SQLITE], [test x$db_sqlite = xtrue])

AC_ARG_ENABLE([db-postgres],
[  --enable-db-postgres    Turn on PostgreSQL backend],
[case "${enableval}" in
  yes) db_postgres=true ;;
  no)  db_postgres=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-db-postgres]) ;;
esac],[db_postgres=true])
AM_CONDITIONAL([ENABLE_DB_POSTGRES], [test x$db_postgres = xtrue])

AC_ARG_ENABLE([db-mysql],
[  --enable-db-mysql    Turn on MySQL backend],
[case "${enableval}" in
  yes) db_mysql=true ;;
  no)  db_mysql=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-db-mysql]) ;;
esac],[db_mysql=false])
AM_CONDITIONAL([ENABLE_DB_MYSQL], [test x$db_mysql = xtrue])

AC_ARG_ENABLE([db-firebird],
[  --enable-db-firebird    Turn on logger-huffmanFirebirdSQL backend],
[case "${enableval}" in
  yes) db_firebird=true ;;
  no)  db_firebird=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-db-firebird]) ;;
esac],[db_firebird=false])
AM_CONDITIONAL([ENABLE_DB_FIREBIRD], [test x$db_firebird = xtrue])

AC_ARG_ENABLE([db-json],
[  --enable-db-json    Turn on JSON POST web service backend],
[case "${enableval}" in
  yes) db_json=true ;;
  no)  db_json=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-db-json]) ;;
esac],[db_json=true])
AM_CONDITIONAL([ENABLE_DB_JSON], [test x$db_json = xtrue])

# Embedded web service

AC_ARG_ENABLE([ws],
[  --enable-ws    Turn on embedded web service],
[case "${enableval}" in
  yes) enable_ws=true ;;
  no)  enable_ws=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-ws]) ;;
esac],[enable_ws=true])
AM_CONDITIONAL([ENABLE_WS], [test x$enable_ws = xtrue])

# Add database headers and libs

if test "$db_sqlite" = "true"; then
AC_CHECK_HEADERS(sqlite3.h)
AC_CHECK_LIB(sqlite3,sqlite3_close)
fi

if test "$db_postgres" = "true"; then
AC_CHECK_HEADERS(libpq-fe.h)
AC_CHECK_LIB(pq,PQfformat)
fi

if test "$db_mysql" = "true"; then
# AX_CHECK_MYSQL() cause /configure: line 11688: syntax error near unexpected token `fi'
# AX_CHECK_MYSQL()
AC_CHECK_HEADERS(mysql/mysql.h)
AC_CHECK_LIB(mysqlclient, mysql_init) 
fi

if test "$db_firebird" = "true"; then
AC_CHECK_HEADERS(ibase.h)
AC_CHECK_LIB(fbclient,fb_sqlstate)
fi

if test "$db_json" = "true"; then
AC_CHECK_HEADERS(curl/curl.h)
fi

# MacOS: install openssl into /usr/local/lib/opt/openssl@1.1
# brew install openssl@1.1
LDFLAGS="$LDFLAGS -L/usr/local/lib"

# Checks for header files. Add /usr/local/include for OS X.
CFLAGS="$CFLAGS -I/usr/local/include"

AC_MSG_NOTICE([Databases:])
if test x$db_postgres = xtrue; then
AC_MSG_NOTICE([  Postgres on])
fi
if test x$db_json = xtrue; then
AC_MSG_NOTICE([  Json])
fi
if test x$db_sqlite = xtrue; then
AC_MSG_NOTICE([  SQLite])
fi
if test x$db_mysql = xtrue; then
AC_MSG_NOTICE([  MySQL])
fi
if test x$db_firebird = xtrue; then
AC_MSG_NOTICE([  Firebird])
fi

AC_MSG_NOTICE([Parsers:])
if test x$logger_huffman = xtrue; then
AC_MSG_NOTICE([  logger-huffman on])
fi
if test x$logger_pkt2 = xtrue; then
AC_MSG_NOTICE([  pkt2 on])
fi

AC_OUTPUT
