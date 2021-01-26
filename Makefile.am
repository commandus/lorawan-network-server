SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4

commoncppflags = -Ithird_party

GEN = 

AM_CXXFLAGS=-std=c++11 -O0
COMMON_CPP_FLAGS = -D_GLIBCXX_USE_CXX11_ABI=0

BUILT_SOURCES = $(GEN)
CLEANFILES = $(GEN)

#
# Binaries
#
bin_PROGRAMS = lorawan-network-server

nobase_dist_include_HEADERS = \
	platform.h utilstring.h utildate.h strptime.h \
	utillora.h udp-socket.h udp-emitter.h \
	errlist.h daemonize.h aes-128.h \
	third_party/argtable3/argtable3.h \
	third_party/base64/base64.h

commonlibs =

lorawan_network_server_SOURCES = \
	lorawan-network-server.cpp errlist.cpp daemonize.cpp \
	utillora.cpp udp-socket.cpp udp-emitter.cpp \
	utilstring.cpp utildate.cpp strptime.cpp utillora.cpp aes-128.c \
	third_party/argtable3/argtable3.c \
	third_party/base64/base64.cpp

lorawan_network_server_LDADD = $(commonlibs)
lorawan_network_server_CPPFLAGS = $(commoncppflags)

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
   autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
   third_party/argtable3/README

test_1_SOURCES = \
	tests/test1.cpp \
	utillora.cpp utildate.cpp utilstring.cpp aes-128.c \
	third_party/base64/base64.cpp
test_1_LDADD = $(commonlibs)
test_1_CPPFLAGS = $(commoncppflags)

check_PROGRAMS = test-1

TESTS = $(check_PROGRAMS)
