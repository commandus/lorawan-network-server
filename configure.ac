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
  yes) enable_db_sqlite=true ;;
  no)  enable_db_sqlite=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-db-sqlite]) ;;
esac],[enable_db_sqlite=true])
AM_CONDITIONAL([ENABLE_DB_SQLITE], [test x$enable_db_sqlite = xtrue])

AC_ARG_ENABLE([db-postgres],
[  --enable-db-postgres    Turn on PostgreSQL backend],
[case "${enableval}" in
  yes) enable_db_postgres=true ;;
  no)  enable_db_postgres=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-db-postgres]) ;;
esac],[enable_db_postgres=false])
AM_CONDITIONAL([ENABLE_DB_POSTGRES], [test x$enable_db_postgres = xtrue])

AC_ARG_ENABLE([db-mysql],
[  --enable-db-mysql    Turn on MySQL backend],
[case "${enableval}" in
  yes) enable_db_mysql=true ;;
  no)  enable_db_mysql=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-db-mysql]) ;;
esac],[enable_db_mysql=false])
AM_CONDITIONAL([ENABLE_DB_MYSQL], [test x$enable_db_mysql = xtrue])

AC_ARG_ENABLE([db-firebird],
[  --enable-db-firebird    Turn on FirebirdSQL backend],
[case "${enableval}" in
  yes) enable_db_firebird=true ;;
  no)  enable_db_firebird=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-db-firebird]) ;;
esac],[enable_db_firebird=false])
AM_CONDITIONAL([ENABLE_DB_FIREBIRD], [test x$enable_db_firebird = xtrue])

if test "$ENABLE_DB_SQLITE" == "true"; then
AC_CHECK_HEADERS(sqlite3.h)
AC_CHECK_LIB(sqlite3,sqlite3_close)
fi

if test "$ENABLE_DB_POSTGRES" == "true"; then
AC_CHECK_HEADERS(libpq-fe.h)
AC_CHECK_LIB(pq,PQfformat)
fi

if test "$ENABLE_DB_MYSQL" == "true"; then
# AX_CHECK_MYSQL() cause /configure: line 11688: syntax error near unexpected token `fi'
# AX_CHECK_MYSQL()
AC_CHECK_HEADERS(mysql/mysql.h mysql/mysql_com.h mysql/mysql_version.h)
AC_CHECK_LIB(mysqlclient, mysql_init) 
fi

if test "$ENABLE_DB_FIREBIRD" == "true"; then
AC_CHECK_HEADERS(ibase.h)
AC_CHECK_LIB(fbclient,fb_sqlstate)
fi

# MacOS: install openssl into /usr/local/lib/opt/openssl@1.1
# brew install openssl@1.1
LDFLAGS="$LDFLAGS -L/usr/local/lib"

# Checks for header files. Add /usr/local/include for OS X.
CFLAGS="$CFLAGS -I/usr/local/include"

AC_OUTPUT
