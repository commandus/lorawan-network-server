SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4

commoncppflags = -Ithird_party

GEN = 

AM_CXXFLAGS=-std=c++11 -O0
COMMON_CPP_FLAGS = -D_GLIBCXX_USE_CXX11_ABI=0

BUILT_SOURCES = $(GEN)
CLEANFILES = $(GEN)

if OS_MACH
LMDB=lmdb
else
LMDB=mdbx
endif

#
# Binaries
#
bin_PROGRAMS = lorawan-network-server

commonlibs =

rapid_headers = \
	third_party/rapidjson/allocators.h       third_party/rapidjson/encodings.h        third_party/rapidjson/fwd.h             third_party/rapidjson/memorystream.h    third_party/rapidjson/prettywriter.h   third_party/rapidjson/schema.h  \
	third_party/rapidjson/writer.h           third_party/rapidjson/document.h         third_party/rapidjson/filereadstream.h  third_party/rapidjson/istreamwrapper.h  third_party/rapidjson/ostreamwrapper.h third_party/rapidjson/rapidjson.h \
	third_party/rapidjson/stream.h           third_party/rapidjson/encodedstream.h    third_party/rapidjson/filewritestream.h third_party/rapidjson/memorybuffer.h    third_party/rapidjson/pointer.h        third_party/rapidjson/reader.h \
	third_party/rapidjson/stringbuffer.h     third_party/rapidjson/error/en.h         third_party/rapidjson/error/error.h     third_party/rapidjson/internal/biginteger.h third_party/rapidjson/internal/dtoa.h  third_party/rapidjson/internal/itoa.h \
	third_party/rapidjson/internal/pow10.h   third_party/rapidjson/internal/stack.h   third_party/rapidjson/internal/strtod.h third_party/rapidjson/internal/diyfp.h  third_party/rapidjson/internal/ieee754.h third_party/rapidjson/internal/meta.h \
	third_party/rapidjson/internal/regex.h   third_party/rapidjson/internal/strfunc.h third_party/rapidjson/internal/swap.h \
	third_party/rapidjson/msinttypes/inttypes.h third_party/rapidjson/msinttypes/stdint.h

nobase_dist_include_HEADERS = \
	platform.h utilstring.h utildate.h strptime.h \
	utillora.h udp-socket.h udp-emitter.h udp-listener.h \
	config-json.h identity-service-abstract.h identity-service-file-json.h \
	lora-packet-handler-abstract.h lora-packet-handler-impl.h \
	db-identity.h identity-service-lmdb.h \
	errlist.h daemonize.h aes-128.h \
	third_party/argtable3/argtable3.h \
	third_party/base64/base64.h \
	$(rapid_headers)

lorawan_network_server_SOURCES = \
	lorawan-network-server.cpp errlist.cpp daemonize.cpp \
	utillora.cpp udp-socket.cpp udp-emitter.cpp udp-listener.cpp \
	config-json.cpp identity-service-file-json.cpp lora-packet-handler-impl.cpp \
	utilstring.cpp utildate.cpp strptime.cpp aes-128.c \
	db-identity.cpp identity-service-lmdb.cpp \
	third_party/argtable3/argtable3.c \
	third_party/base64/base64.cpp

lorawan_network_server_LDADD = $(commonlibs) -l$(LMDB)
lorawan_network_server_CPPFLAGS = $(commoncppflags)

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
   autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
   third_party/argtable3/README

#
# Tests
#
test_1_SOURCES = \
	tests/test1.cpp \
	utillora.cpp utildate.cpp utilstring.cpp aes-128.c \
	third_party/base64/base64.cpp
test_1_LDADD = $(commonlibs)
test_1_CPPFLAGS = $(commoncppflags)

test_identity_file_json_SOURCES = \
	tests/test-identity-file-json.cpp \
	identity-service-file-json.cpp \
	utillora.cpp utildate.cpp utilstring.cpp aes-128.c \
	third_party/base64/base64.cpp
test_identity_file_json_LDADD = 
test_identity_file_json_CPPFLAGS = $(commoncppflags)


test_identity_lmdb_SOURCES = \
	tests/test-identity-lmdb.cpp \
	identity-service-lmdb.cpp db-identity.cpp \
	utillora.cpp utildate.cpp utilstring.cpp aes-128.c \
	third_party/base64/base64.cpp
test_identity_lmdb_LDADD = -l$(LMDB)
test_identity_lmdb_CPPFLAGS = $(commoncppflags)

test_udp_listen_SOURCES = \
	tests/test-udp-listen.cpp \
	udp-socket.cpp errlist.cpp
test_udp_listen_LDADD =
test_udp_listen_CPPFLAGS = $(commoncppflags)

check_PROGRAMS = test-identity-file-json test-identity-lmdb
#test-1 test-udp-listen 

TESTS = $(check_PROGRAMS)
