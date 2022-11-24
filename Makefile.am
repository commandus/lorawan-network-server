SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4

VERSION_INFO = 0:2:0

commoncppflags = -I. -Ithird_party -I../../third_party

GEN = 

AM_CPPFLAGS=$(commoncppflags) -std=c++11
AM_CXXFLAGS=$(commoncppflags) -std=c++11
COMMON_CPP_FLAGS = -D_GLIBCXX_USE_CXX11_ABI=0

BUILT_SOURCES = $(GEN)
CLEANFILES = $(GEN)

C_STORAGE = 
C_QUEUE_STORAGE = 
LIB_STORAGE = 

if ENABLE_JSON
C_STORAGE += identity-service-file-json.cpp 
C_QUEUE_STORAGE += receiver-queue-service-file-json.cpp receiver-queue-service-dir-txt.cpp
endif

if ENABLE_LMDB
LIB_STORAGE += -llmdb
C_STORAGE += identity-service-lmdb.cpp db-identity.cpp
#C_QUEUE_STORAGE=receiver-queue-service-lmdb.cpp 
endif

if ENABLE_MDBX
LIB_STORAGE += -lmdbx
C_STORAGE += identity-service-lmdb.cpp db-identity.cpp
#C_QUEUE_STORAGE=receiver-queue-service-lmdb.cpp 
endif

# Parsers
PACKET_PARSER_SRC =
PACKET_PARSER_LIB =
PACKET_PARSER_DEF = 

if ENABLE_PKT2
PACKET_PARSER_LIB += -L/home/andrei/src/pkt2 -lpkt2 -lprotobuf
PACKET_PARSER_DEF += -DENABLE_PKT2
endif

if ENABLE_LOGGER_HUFFMAN
PACKET_PARSER_SRC += logger-loader.cpp
PACKET_PARSER_LIB += -L/home/andrei/src/logger-passport -L/home/andrei/src/logger-huffman -llogger-huffman -llogger-passport
PACKET_PARSER_DEF += -DENABLE_LOGGER_HUFFMAN
endif

# Embedded web service
WS_SRC =
WS_LIB =
WS_DEF = 

if ENABLE_WS
WS_SRC += ws-handler.cpp
WS_LIB += -L/home/andrei/src/lorawan-ws -lws-sqlite -lmicrohttpd
WS_DEF += -DENABLE_WS
endif

WS_AUTH_LIB = 
WS_AUTH_DEF = 

if ENABLE_JWT
WS_AUTH_LIB += -lssl -lcrypto
WS_AUTH_DEF += -DENABLE_JWT
endif

BACKEND_DB_SRC = 
BACKEND_DB_LIB =
BACKEND_DB_C =
BACKEND_DB_DEF = 

if ENABLE_DB_SQLITE
BACKEND_DB_SRC += db-sqlite.cpp
BACKEND_DB_LIB += -lsqlite3 
BACKEND_DB_DEF += -DENABLE_DB_SQLITE 

if ENABLE_WS
BACKEND_DB_SRC += ws-sqlite.cpp
BACKEND_DB_DEF += -DENABLE_DB_JSON
endif

endif

if ENABLE_DB_POSTGRES
BACKEND_DB_SRC += db-pg.cpp
BACKEND_DB_LIB += -lpq 
BACKEND_DB_C += 
BACKEND_DB_DEF += -DENABLE_DB_POSTGRES 
endif

if ENABLE_DB_MYSQL
BACKEND_DB_SRC += db-mysql.cpp
BACKEND_DB_LIB += -lmysqlclient 
BACKEND_DB_C += 
BACKEND_DB_DEF += -DENABLE_DB_MYSQL 
endif

if ENABLE_DB_FIREBIRD
BACKEND_DB_SRC += db-fb.cpp
BACKEND_DB_LIB += -lfbclient 
BACKEND_DB_C += 
BACKEND_DB_DEF += -DENABLE_DB_FIREBIRD 
endif

if ENABLE_DB_JSON
BACKEND_DB_SRC += db-json.cpp
BACKEND_DB_LIB +=
BACKEND_DB_C +=
BACKEND_DB_DEF += -DENABLE_DB_JSON
endif

if ENABLE_WS
	BACKEND_DB_SRC += ws-any.cpp
	BACKEND_DB_DEF += -DENABLE_WS
endif

# Listener

if ENABLE_LISTENER_UDP
LISTENER_SRC = udp-listener.cpp
LISTENER_LIB =
LISTENER_DEF = -DENABLE_LISTENER_UDP
endif

if ENABLE_LISTENER_USB
LISTENER_SRC = usb-listener.cpp lora-gateway-listener.cpp \
    gateway-file-json.cpp \
    third_party/packet_forwarder/jitqueue.c
LISTENER_LIB = -Lthird_party/packet_forwarder -lloragw -ltinymt32
LISTENER_DEF = -DENABLE_LISTENER_USB
endif

if ENABLE_LISTENER_SPI
LGW_HOME = third_party/packet_forwarder
LGW_SRC = embedded-listener.cpp gateway-file-json.cpp gateway-settings.cpp \
    lora-gateway-listener.cpp usb-listener.cpp utilusb.cpp \
    third_party/packet_forwarder/jitqueue.c
LGW_LIB = -L$(LGW_HOME) -lloragw -ltinymt32 -lusb
LGW_DEF = -DENABLE_LISTENER_SPI
endif

#
# mysql_config --cflags
# -I/usr/include/mysql
#

#
# mysql_config --libs
# -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -lz -lm -lrt -lssl -lcrypto -ldl
#

#
# Binaries
#
bin_PROGRAMS = lorawan-network-server lora-print print-netid \
    dev-payload \
    mac-ns mac-gw proto-db	

lib_LTLIBRARIES = libplugin-esp32-demo.la
if ENABLE_PKT2
    lib_LTLIBRARIES += libplugin-pkt2.la
endif
if ENABLE_LOGGER_HUFFMAN
    lib_LTLIBRARIES += libplugin-logger-huffman.la
endif

LIBLORAWANSRC = \
	utillora.cpp errlist.cpp net-id.cpp dev-addr.cpp udp-socket.cpp device-history-item.cpp gateway-stat.cpp \
	lorawan-mac.cpp lora-rejoin.cpp utilstring.cpp utildate.cpp utilfile.cpp utilidentity.cpp utilcurl.cpp \
	utilthread.cpp \
	strptime.cpp lora-encrypt.cpp identity-service.cpp receiver-queue-processor.cpp regional-parameter-channel-plan.cpp \
	device-stat-service-abstract.cpp device-channel-plan.cpp packet-queue.cpp gateway-list.cpp \
	gateway-stat-service-abstract.cpp semtech-pull-resp-packet.cpp receiver-queue-service.cpp \
	semtech-metadata-tx.cpp database-config-json.cpp auth-user.cpp payload-insert-plugin.cpp \
	$(SRC_AES) third_party/base64/base64.cpp

#
#	Static library
#
lib_LIBRARIES = liblorawan.a
liblorawan_a_SOURCES = $(LIBLORAWANSRC)
liblorawan_a_CXXFLAGS = $(commoncppflags) $(PACKET_PARSER_DEF) -fPIC -DLNS_VERSION=$VERSION $(WS_AUTH_DEF)

L_LORAWAN = -L. -llorawan

# Static library used for buil binaries, so build library first.
$(bin_PROGRAMS): $(lib_LIBRARIES)

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
	run-listener.h platform.h utilstring.h utildate.h utilfile.h utilusb.h strptime.h lora-radio.h \
	utillora.h utilthread.h net-id.h dev-addr.h udp-socket.h udp-emitter.h lora-encrypt.h \
	packet-listener.h udp-listener.h embedded-listener.h usb-listener.h \
	config-json.h semtech-metadata-tx.h packet-handler-abstract.h \
	lora-packet-handler-abstract.h lora-packet-handler-impl.h packet-queue.h \
	errlist.h daemonize.h device-history-item.h gateway-stat.h gateway-list.h lorawan-mac.h \
	macgw-config-json.h config-filename.h semtech-pull-resp-packet.h \
	identity-service.h identity-service-file-json.h logger-loader.h \
	receiver-queue-service.h receiver-queue-service-file-json.h receiver-queue-service-dir-txt.h receiver-queue-service-lmdb.h \
	db-identity.h identity-service-lmdb.h identity-service-dir-txt.h \
	gateway-stat-service-abstract.h gateway-stat-service-file.h gateway-stat-service-post.h \
	lora-rejoin.h utilidentity.h control-packet.h database-config-json.h ws-handler.h gateway-settings.h \
	payload-insert.h payload-insert-plugin.h log-intf.h \
	auth-user.h auth-file.h \
	third_party/filewatch.hpp \
	third_party/argtable3/argtable3.h \
	third_party/base64/base64.h \
	third_party/system/crypto/aes.h third_party/system/crypto/cmac.h \
	third_party/pkt2/str-pkt2.h third_party/pkt2/database-config.h \
	third_party/logger-huffman/logger-parse.h third_party/logger-huffman/logger-collection.h \
	third_party/logger-huffman/logger-huffman.h \
	third_party/lorawan-ws/lorawan-ws.h \
	third_party/jwt-cpp/jwt.h third_party/jwt-cpp/base.h third_party/jwt-cpp/traits/kazuho-picojson/traits.h \
	third_party/jwt-cpp/traits/kazuho-picojson/defaults.h third_party/picojson/picojson.h \
	db-intf.h db-any.h db-sqlite.h db-pg.h db-mysql.h db-fb.h db-json.h \
	receiver-queue-processor.h device-history-item.h device-history-service-abstract.h device-history-service-json.h \
	device-stat-service-abstract.h device-stat-service-post.h device-stat-service-file.h utilcurl.h \
	device-channel-plan-file-json.h device-channel-plan.h regional-parameter-channel-plan.h regional-parameter-channel-plans.h regional-parameter-channel-plan-file-json.h \
	third_party/get_rss/get_rss.h \
	gateway-lora.h gateway-file-json.h lora-gateway-listener.h \
	$(rapid_headers)

SRC_AES = third_party/system/crypto/aes.c third_party/system/crypto/cmac.c

lorawan_network_server_SOURCES = \
	lorawan-network-server.cpp run-listener.cpp daemonize.cpp config-json.cpp lora-packet-handler-impl.cpp \
	config-filename.cpp identity-service-dir-txt.cpp gateway-stat-service-file.cpp gateway-stat-service-post.cpp \
	device-stat-service-file.cpp device-stat-service-post.cpp packet-listener.cpp db-any.cpp \
	device-channel-plan-file-json.cpp device-history-service-json.cpp regional-parameter-channel-plan-file-json.cpp \
	auth-file.cpp third_party/get_rss/get_rss.c third_party/argtable3/argtable3.c \
	$(C_STORAGE) $(C_QUEUE_STORAGE) $(LISTENER_SRC) $(LGW_SRC) $(BACKEND_DB_SRC) $(WS_SRC) $(PACKET_PARSER_SRC)

lorawan_network_server_LDADD = $(commonlibs) $(L_LORAWAN) $(LIB_STORAGE) -lpthread \
    $(PACKET_PARSER_LIB) $(LISTENER_LIB) $(LGW_LIB) \
    $(BACKEND_DB_LIB) -lcurl $(WS_AUTH_LIB) $(WS_LIB)

lorawan_network_server_CPPFLAGS = $(commoncppflags) \
    $(BACKEND_DB_DEF) $(LISTENER_DEF) $(LGW_DEF) $(PACKET_PARSER_DEF) -DLNS_VERSION=$(VERSION) $(WS_AUTH_DEF) $(WS_DEF)

#
#	Dynamic loaded library (plugins)
#
libplugin_esp32_demo_la_SOURCES = \
	example-plugins/plugin-esp32-demo.cpp
libplugin_esp32_demo_la_CPPFLAGS =
libplugin_esp32_demo_la_LDFLAGS = -version-info $(VERSION_INFO)

if ENABLE_PKT2
libplugin_pkt2_la_SOURCES = example-plugins/plugin-pkt2.cpp
libplugin_pkt2_la_CPPFLAGS = -DENABLE_PKT2
libplugin_pkt2_la_LDFLAGS = -version-info $(VERSION_INFO)
libplugin_pkt2_la_LIBADD = -L/home/andrei/src/pkt2 -lpkt2 -lprotobuf
endif

if ENABLE_LOGGER_HUFFMAN
libplugin_logger_huffman_la_SOURCES = example-plugins/plugin-logger-huffman.cpp logger-loader.cpp \
    db-any.cpp $(BACKEND_DB_SRC)
libplugin_logger_huffman_la_CPPFLAGS = -DENABLE_LOGGER_HUFFMAN $(BACKEND_DB_DEF)
libplugin_logger_huffman_la_LDFLAGS = -version-info $(VERSION_INFO) $(BACKEND_DB_LIB)
libplugin_logger_huffman_la_LIBADD = -L/home/andrei/src/logger-passport -L/home/andrei/src/logger-huffman \
    -llogger-huffman -llogger-passport
endif

mac_gw_SOURCES = \
	mac-gw.cpp macgw-config-json.cpp \
	config-json.cpp  \
	lora-packet-handler-impl.cpp config-filename.cpp \
	$(C_STORAGE) \
	db-any.cpp  $(BACKEND_DB_SRC) \
	receiver-queue-service-file-json.cpp receiver-queue-service-dir-txt.cpp \
	third_party/argtable3/argtable3.c

mac_gw_LDADD = $(commonlibs) $(L_LORAWAN) $(LIB_STORAGE) -lpthread \
    $(PACKET_PARSER_LIB) \
    -lcurl $(BACKEND_DB_LIB)
mac_gw_CPPFLAGS = $(commoncppflags)

mac_ns_SOURCES = \
	mac-ns.cpp macgw-config-json.cpp config-json.cpp \
	lora-packet-handler-impl.cpp config-filename.cpp \
    db-any.cpp \
	receiver-queue-service-file-json.cpp receiver-queue-service-dir-txt.cpp \
    device-history-service-json.cpp control-packet.cpp \
	$(C_STORAGE) $(BACKEND_DB_SRC) \
	third_party/argtable3/argtable3.c
mac_ns_LDADD = $(commonlibs) $(L_LORAWAN) $(LIB_STORAGE) -lpthread \
    $(PACKET_PARSER_LIB) \
    -lcurl $(BACKEND_DB_LIB)
mac_ns_CPPFLAGS = $(commoncppflags)

dev_payload_SOURCES = \
	dev-payload.cpp \
	config-json.cpp \
	config-filename.cpp \
	$(C_STORAGE) \
	third_party/argtable3/argtable3.c
dev_payload_LDADD = $(commonlibs) $(L_LORAWAN) $(LIB_STORAGE) -lpthread \
    $(PACKET_PARSER_LIB) \
    -lcurl $(BACKEND_DB_LIB)
dev_payload_CPPFLAGS = $(commoncppflags)

proto_db_SOURCES = \
	proto-db.cpp identity-service-dir-txt.cpp $(C_STORAGE) \
	db-any.cpp  $(BACKEND_DB_SRC) \
	third_party/argtable3/argtable3.c
	
proto_db_LDADD = $(L_LORAWAN) \
    $(PACKET_PARSER_LIB) \
    -lpthread -lcurl $(BACKEND_DB_LIB)
proto_db_CPPFLAGS = $(commoncppflags) $(BACKEND_DB_DEF) $(PACKET_PARSER_DEF)

lora_print_SOURCES = \
	lora-print.cpp utilfile.cpp config-filename.cpp db-any.cpp \
	third_party/argtable3/argtable3.c \
	identity-service-dir-txt.cpp \
	$(BACKEND_DB_SRC) $(C_STORAGE)
lora_print_LDADD = $(L_LORAWAN) \
    $(PACKET_PARSER_LIB) \
    -lpthread -lcurl $(BACKEND_DB_LIB)
lora_print_CPPFLAGS = $(commoncppflags) $(BACKEND_DB_DEF) $(PACKET_PARSER_DEF)

print_netid_SOURCES = \
	print-netid.cpp \
	third_party/argtable3/argtable3.c
print_netid_LDADD = $(L_LORAWAN)
print_netid_CPPFLAGS = $(commoncppflags)

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
   autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
   service/lorawan-network-server.service \
   third_party/argtable3/README \
   tests/netid-list.sh tests/netid-list.txt

#
# Tests
#
test_1_SOURCES = \
	tests/test1.cpp
	
test_1_LDADD = $(commonlibs) $(L_LORAWAN)
test_1_CPPFLAGS = $(commoncppflags)

test_identity_file_json_SOURCES = \
	tests/test-identity-file-json.cpp \
	identity-service-file-json.cpp
	
test_identity_file_json_LDADD = $(L_LORAWAN)
test_identity_file_json_CPPFLAGS = $(commoncppflags)

test_identity_lmdb_SOURCES = \
	tests/test-identity-lmdb.cpp \
	$(C_STORAGE) db-identity.cpp identity-service-lmdb.cpp
	
test_identity_lmdb_LDADD = $(L_LORAWAN) $(LIB_STORAGE) -llmdb
test_identity_lmdb_CPPFLAGS = $(commoncppflags) -DENABLE_LMDB

test_udp_listen_SOURCES = \
	tests/test-udp-listen.cpp
test_udp_listen_LDADD = $(L_LORAWAN)
test_udp_listen_CPPFLAGS = $(commoncppflags)

test_packet_queue_SOURCES = \
	tests/test-packet-queue.cpp \
test_packet_queue_LDADD = $(L_LORAWAN) -lpthread
test_packet_queue_CPPFLAGS = $(commoncppflags)

test_mac_gw_SOURCES = \
	tests/test-mac-gw.cpp \
	macgw-config-json.cpp
	
test_mac_gw_LDADD = $(L_LORAWAN)
test_mac_gw_CPPFLAGS = $(commoncppflags)

test_file_watch_SOURCES = \
	tests/test-file-watch.cpp  
test_file_watch_LDADD = $(L_LORAWAN) -lpthread
test_file_watch_CPPFLAGS = $(commoncppflags)

test_dir_txt_SOURCES = \
	tests/test-dir-txt.cpp \
	identity-service-file-json.cpp identity-service-dir-txt.cpp
	
test_dir_txt_LDADD = $(L_LORAWAN) -lpthread
test_dir_txt_CPPFLAGS = $(commoncppflags)

test_join_accept_decrypt_SOURCES = \
	tests/test-join-accept-decrypt.cpp   \
	identity-service-file-json.cpp identity-service-dir-txt.cpp
	
test_join_accept_decrypt_LDADD = $(L_LORAWAN) -lpthread
test_join_accept_decrypt_CPPFLAGS = $(commoncppflags)

if ENABLE_PKT2
test_pkt2_parse_SOURCES = \
	tests/test-pkt2-parse.cpp
test_pkt2_parse_LDADD = $(L_LORAWAN) \
    $(PACKET_PARSER_LIB) \
    -lpthread
test_pkt2_parse_CPPFLAGS = $(commoncppflags)
endif

test_sqlite_SOURCES = \
	tests/test-sqlite.cpp db-any.cpp  $(BACKEND_DB_SRC)
test_sqlite_LDADD = $(L_LORAWAN) \
    $(PACKET_PARSER_LIB) \
    -lpthread -lcurl $(BACKEND_DB_LIB)
test_sqlite_CPPFLAGS = $(commoncppflags) $(BACKEND_DB_DEF) $(PACKET_PARSER_DEF)

test_fb_SOURCES = \
	tests/test-fb.cpp
test_fb_LDADD = $(L_LORAWAN) $(BACKEND_DB_LIB)
test_fb_CPPFLAGS = $(BACKEND_DB_DEF) $(PACKET_PARSER_DEF)

test_message_queue_SOURCES = \
	tests/test-message-queue.cpp \
	receiver-queue-service-file-json.cpp receiver-queue-service-dir-txt.cpp
     
test_message_queue_LDADD = $(L_LORAWAN)
test_message_queue_CPPFLAGS = $(commoncppflags)

test_queue_processor_SOURCES = \
	tests/test-queue-processor.cpp \
	identity-service-file-json.cpp \
	receiver-queue-service-file-json.cpp \
	db-any.cpp $(BACKEND_DB_SRC) \
test_queue_processor_LDADD = $(L_LORAWAN) \
    $(PACKET_PARSER_LIB) \
    -lpthread -lcurl $(BACKEND_DB_LIB)
test_queue_processor_CPPFLAGS = $(commoncppflags) $(BACKEND_DB_DEF) $(PACKET_PARSER_DEF)

test_region_band_file_json_SOURCES = \
	tests/test-region-band-file-json.cpp \
	device-channel-plan-file-json.cpp   \
	regional-parameter-channel-plan-file-json.cpp
	
test_region_band_file_json_LDADD = $(L_LORAWAN)
test_region_band_file_json_CPPFLAGS = $(commoncppflags)

test_join_request_SOURCES = \
	tests/test-join-request.cpp
	
test_join_request_LDADD = $(L_LORAWAN)
test_join_request_CPPFLAGS = $(commoncppflags)

test_dev_addr_SOURCES = \
	tests/test-dev-addr.cpp \
	identity-service-file-json.cpp

test_dev_addr_LDADD = $(L_LORAWAN)
test_dev_addr_CPPFLAGS = $(commoncppflags)


test_encrypt_join_accept_response_SOURCES = \
	tests/test-encrypt-join-accept-response.cpp
test_encrypt_join_accept_response_LDADD = $(L_LORAWAN)
test_encrypt_join_accept_response_CPPFLAGS = $(commoncppflags)

test_gateway_file_json_SOURCES = \
    tests/test-gateway-file-json.cpp \
    gateway-file-json.cpp utilstring.cpp
test_gateway_file_json_LDADD = -Lthird_party/packet_forwarder -lloragw -ltinymt32
test_gateway_file_json_CPPFLAGS = $(commoncppflags)

test_gateway_listen_SOURCES = \
    tests/test-gateway-listen.cpp \
    lora-gateway-listener.cpp gateway-settings.cpp gateway-file-json.cpp utilstring.cpp utildate.cpp \
    errlist.cpp \
    third_party/packet_forwarder/jitqueue.c
test_gateway_listen_LDADD = -Lthird_party/packet_forwarder -lloragw -ltinymt32 -lusb
test_gateway_listen_CPPFLAGS = $(commoncppflags)

check_PROGRAMS = \
	test-gateway-listen
#   test-gateway-file-json \
#	test-encrypt-join-accept-response \
#   test-dev-addr \
#   test-queue-processor \
#	test-join-request \
#	test-message-queue test-fb test-sqlite test-pkt2-parseRX test-join-accept-decrypt test-dir-txt test-identity-file-json \
#	test-file-watch test-mac-gw	test-1 test-udp-listen test-identity-file-json test-identity-lmdb test-packet-queue test-identity-file-json \
#	test-region-band-file-json

TESTS = $(check_PROGRAMS)
