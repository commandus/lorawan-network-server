SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4

commoncppflags = -Ithird_party

GEN = 

AM_CXXFLAGS=-std=c++11 -O0
COMMON_CPP_FLAGS = -D_GLIBCXX_USE_CXX11_ABI=0

BUILT_SOURCES = $(GEN)
CLEANFILES = $(GEN)

if ENABLE_JSON
LIB_STORAGE=
C_STORAGE=identity-service-file-json.cpp 
H_STORAGE=
endif

if ENABLE_LMDB
LIB_STORAGE=-llmdb
C_STORAGE=identity-service-lmdb.cpp db-identity.cpp
H_STORAGE=
endif

if ENABLE_MDBX
LIB_STORAGE=-lmdbx
C_STORAGE=identity-service-lmdb.cpp db-identity.cpp
H_STORAGE=
endif

#
# Binaries
#
bin_PROGRAMS = lorawan-network-server mac-gw

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
	platform.h utilstring.h utildate.h utilfile.h strptime.h lora-radio.h \
	utillora.h udp-socket.h udp-emitter.h udp-listener.h lora-encrypt.h \
	config-json.h semtech-metadata-tx.h packet-handler-abstract.h \
	lora-packet-handler-abstract.h lora-packet-handler-impl.h packet-queue.h \
	errlist.h daemonize.h gateway-stat.h gateway-list.h lorawan-mac.h \
	macgw-config-json.h config-filename.h semtech-pull-resp-packet.h \
	identity-service-abstract.h identity-service-file-json.h \
	db-identity.h identity-service-lmdb.h identity-service-dir-txt.h \
	lora-rejoin.h \
	third_party/filewatch.hpp \
	third_party/argtable3/argtable3.h \
	third_party/base64/base64.h \
	third_party/system/crypto/aes.h third_party/system/crypto/cmac.h
	$(rapid_headers)

SRC_AES = third_party/system/crypto/aes.c third_party/system/crypto/cmac.c
lorawan_network_server_SOURCES = \
	lorawan-network-server.cpp errlist.cpp daemonize.cpp \
	utillora.cpp udp-socket.cpp udp-emitter.cpp udp-listener.cpp \
	config-json.cpp lora-packet-handler-impl.cpp \
	packet-queue.cpp gateway-stat.cpp gateway-list.cpp lorawan-mac.cpp \
	utilstring.cpp utildate.cpp utilfile.cpp strptime.cpp config-filename.cpp \
	semtech-pull-resp-packet.cpp lora-encrypt.cpp \
	semtech-metadata-tx.cpp identity-service-dir-txt.cpp lora-rejoin.cpp \
	$(C_STORAGE) \
	third_party/argtable3/argtable3.c \
	third_party/base64/base64.cpp $(SRC_AES)

lorawan_network_server_LDADD = $(commonlibs) $(LIB_STORAGE) -lpthread
lorawan_network_server_CPPFLAGS = $(commoncppflags)

mac_gw_SOURCES = \
	mac-gw.cpp macgw-config-json.cpp errlist.cpp \
	utillora.cpp udp-socket.cpp udp-emitter.cpp \
	config-json.cpp packet-queue.cpp gateway-stat.cpp gateway-list.cpp lorawan-mac.cpp \
	lora-packet-handler-impl.cpp config-filename.cpp lora-encrypt.cpp \
	utilstring.cpp utildate.cpp utilfile.cpp strptime.cpp semtech-pull-resp-packet.cpp \
	semtech-metadata-tx.cpp lora-rejoin.cpp \
	$(C_STORAGE) \
	third_party/argtable3/argtable3.c \
	third_party/base64/base64.cpp $(SRC_AES)

mac_gw_LDADD = $(commonlibs) $(LIB_STORAGE) -lpthread
mac_gw_CPPFLAGS = $(commoncppflags)

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
	utillora.cpp utildate.cpp utilstring.cpp \
	third_party/base64/base64.cpp $(SRC_AES)
test_1_LDADD = $(commonlibs)
test_1_CPPFLAGS = $(commoncppflags)

test_identity_file_json_SOURCES = \
	tests/test-identity-file-json.cpp \
	identity-service-file-json.cpp \
	utillora.cpp utildate.cpp utilstring.cpp \
	gateway-stat.cpp lora-encrypt.cpp lora-rejoin.cpp \
	third_party/base64/base64.cpp $(SRC_AES)
test_identity_file_json_LDADD = 
test_identity_file_json_CPPFLAGS = $(commoncppflags)

test_identity_lmdb_SOURCES = \
	tests/test-identity-lmdb.cpp \
	$(C_STORAGE) \
	utillora.cpp utildate.cpp utilstring.cpp \
	third_party/base64/base64.cpp $(SRC_AES)
test_identity_lmdb_LDADD = $(LIB_STORAGE)
test_identity_lmdb_CPPFLAGS = $(commoncppflags)

test_udp_listen_SOURCES = \
	tests/test-udp-listen.cpp \
	udp-socket.cpp errlist.cpp $(SRC_AES)
test_udp_listen_LDADD =
test_udp_listen_CPPFLAGS = $(commoncppflags)

test_packet_queue_SOURCES = \
	tests/test-packet-queue.cpp \
	packet-queue.cpp utillora.cpp errlist.cpp \
	utilstring.cpp utildate.cpp \
	third_party/base64/base64.cpp $(SRC_AES)
test_packet_queue_LDADD = -lpthread
test_packet_queue_CPPFLAGS = $(commoncppflags)

test_mac_gw_SOURCES = \
	tests/test-mac-gw.cpp \
	macgw-config-json.cpp lorawan-mac.cpp errlist.cpp \
	utilstring.cpp utildate.cpp \
	third_party/base64/base64.cpp $(SRC_AES)
test_mac_gw_LDADD =
test_mac_gw_CPPFLAGS = $(commoncppflags)

test_file_watch_SOURCES = \
	tests/test-file-watch.cpp
test_file_watch_LDADD = -lpthread
test_file_watch_CPPFLAGS = $(commoncppflags)

test_dir_txt_SOURCES = \
	tests/test-dir-txt.cpp \
	identity-service-file-json.cpp identity-service-dir-txt.cpp \
	utilstring.cpp utildate.cpp utillora.cpp utilfile.cpp \
	lora-encrypt.cpp gateway-stat.cpp lora-rejoin.cpp \
	third_party/base64/base64.cpp $(SRC_AES)
test_dir_txt_LDADD = -lpthread
test_dir_txt_CPPFLAGS = $(commoncppflags)

test_join_accept_decrypt_SOURCES = \
	tests/test-join-accept-decrypt.cpp \
	identity-service-file-json.cpp identity-service-dir-txt.cpp \
	utilstring.cpp utildate.cpp utillora.cpp utilfile.cpp \
	lora-encrypt.cpp gateway-stat.cpp lora-rejoin.cpp \
	third_party/base64/base64.cpp $(SRC_AES)
test_join_accept_decrypt_LDADD = -lpthread
test_join_accept_decrypt_CPPFLAGS = $(commoncppflags)

check_PROGRAMS = test-join-accept-decrypt
# test-dir-txt test-identity-file-json
# test-file-watch test-mac-gw
#test-1 test-udp-listen test-identity-file-json test-identity-lmdb test-packet-queue test-identity-file-json

TESTS = $(check_PROGRAMS)
