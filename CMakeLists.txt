# Options:
# -D ENABLE_PKT2
# -D ENABLE_LGW_EMBEDDED

cmake_minimum_required(VERSION 3.9)
set(PROJECT_NAME "lorawan-network-n-app-server")
project(${PROJECT_NAME} VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)

option(ENABLE_PKT2 "Build with pkt2" OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(CURL REQUIRED)
# find_package(unofficial-sqlite3 REQUIRED)
set(CURL_LIBRARIES curl)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

if(ENABLE_PKT2)
	find_library(LIBPKT2
			NAMES pkt2
			HINTS "." ".." "../../pkt2" "~/src/pkt2"
			)
endif(ENABLE_PKT2)

option(ENABLE_LISTENER_EMBEDDED "Build with embedded Lora gateway" OFF)
if(ENABLE_LISTENER_EMBEDDED)
	find_library(LIBLORAGW
			NAMES loragw
			HINTS "third_party/packet_forwarder" "." ".." "../../packet_forwarder" "~/src/packet_forwarder"
	)
	find_library(LIBTINYMT32
			NAMES tinymt32
			HINTS "third_party/packet_forwarder" "." ".." "../../packet_forwarder" "~/src/packet_forwarder"
	)
endif(ENABLE_LISTENER_EMBEDDED)

include_directories("third_party" ${CURL_INCLUDE_DIR} ".")
link_directories(${CMAKE_SOURCE_DIR}/../lorawan-ws)

set(SRC_AES
	third_party/system/crypto/aes.c third_party/system/crypto/cmac.c
)

set(C_STORAGE
	identity-service-file-json.cpp
)
set(C_QUEUE_STORAGE
	receiver-queue-service-file-json.cpp receiver-queue-service-dir-txt.cpp
)

set(BACKEND_DB_LIB sqlite3)

set(BACKEND_DB_SRC
	db-json.cpp
	db-sqlite.cpp
)

set(LIBLORAWANSRC
	utillora.cpp errlist.cpp net-id.cpp dev-addr.cpp udp-socket.cpp device-history-item.cpp gateway-stat.cpp
	lorawan-mac.cpp lora-rejoin.cpp utilstring.cpp utildate.cpp utilfile.cpp utilidentity.cpp utilcurl.cpp
	strptime.cpp lora-encrypt.cpp identity-service.cpp receiver-queue-processor.cpp regional-parameter-channel-plan.cpp
	device-stat-service-abstract.cpp device-channel-plan.cpp packet-queue.cpp gateway-list.cpp
	gateway-stat-service-abstract.cpp semtech-pull-resp-packet.cpp receiver-queue-service.cpp
	semtech-metadata-tx.cpp auth-file.cpp auth-user.cpp
	${SRC_AES}
	third_party/base64/base64.cpp
)

set(SRC_LORAWAN_NETWORK_SERVER
	lorawan-network-server.cpp run-listener.cpp daemonize.cpp udp-listener.cpp
	packet-listener.cpp config-json.cpp lora-packet-handler-impl.cpp config-filename.cpp
	identity-service-dir-txt.cpp ${C_STORAGE} ${C_QUEUE_STORAGE}
	gateway-stat-service-file.cpp gateway-stat-service-post.cpp
	device-stat-service-file.cpp device-stat-service-post.cpp
	db-any.cpp ${BACKEND_DB_SRC}
	device-channel-plan-file-json.cpp device-history-service-json.cpp
	regional-parameter-channel-plan-file-json.cpp
	ws-handler.cpp database-config-json.cpp 
	embedded-listener.cpp gateway-file-json.cpp lora-gateway-listener.cpp
	third_party/get_rss/get_rss.c
	third_party/argtable3/argtable3.c
)

set(DEV_PAYLOAD_SOURCES
	dev-payload.cpp
	config-json.cpp config-filename.cpp
	${C_STORAGE}
    third_party/argtable3/argtable3.c
)

add_library(lorawan STATIC
	${LIBLORAWANSRC}
)

add_executable(lorawan-network-server ${SRC_LORAWAN_NETWORK_SERVER})
target_link_libraries(lorawan-network-server PRIVATE Threads::Threads
	${LIBPKT2} lorawan protobuf ${CURL_LIBRARIES} ${BACKEND_DB_LIB} ws-sqlite microhttpd
	${LIBLORAGW} ${LIBTINYMT32}
		OpenSSL::Crypto
)
target_compile_definitions(lorawan-network-server PRIVATE ENABLE_DB_SQLITE ENABLE_LISTENER_UDP)

set(SRC_MAC_GW
	mac-gw.cpp macgw-config-json.cpp config-json.cpp lora-packet-handler-impl.cpp
	config-filename.cpp
	${C_STORAGE} db-any.cpp  ${BACKEND_DB_SRC}
	receiver-queue-service-file-json.cpp receiver-queue-service-dir-txt.cpp
	third_party/argtable3/argtable3.c
)
add_executable(mac-gw ${SRC_MAC_GW})
target_link_libraries(mac-gw PRIVATE Threads::Threads
	${LIBPKT2} lorawan protobuf ${CURL_LIBRARIES} ${BACKEND_DB_LIB}
)

set(SRC_PRINT_NETID
	print-netid.cpp
	third_party/argtable3/argtable3.c
)
add_executable(print-netid ${SRC_PRINT_NETID})
target_link_libraries(print-netid
	lorawan
)

set(LORA_PRINT_SOURCES
	lora-print.cpp
	database-config-json.cpp
	db-any.cpp ${BACKEND_DB_SRC} ${C_STORAGE}
	third_party/argtable3/argtable3.c
	identity-service-dir-txt.cpp
)
add_executable(lora-print ${LORA_PRINT_SOURCES})
target_link_libraries(lora-print PRIVATE Threads::Threads
	${LIBPKT2} lorawan protobuf ${CURL_LIBRARIES} ${BACKEND_DB_LIB}
)

set(PROTO_DB_SOURCES
	proto-db.cpp identity-service-dir-txt.cpp
	db-any.cpp database-config-json.cpp
	${BACKEND_DB_SRC} ${C_STORAGE}
	third_party/argtable3/argtable3.c
)
add_executable(proto-db ${PROTO_DB_SOURCES})
target_link_libraries(proto-db PRIVATE Threads::Threads
	${LIBPKT2} lorawan protobuf ${CURL_LIBRARIES} ${BACKEND_DB_LIB}
)

add_executable(dev-payload ${DEV_PAYLOAD_SOURCES})
target_link_libraries(dev-payload PRIVATE Threads::Threads
	lorawan protobuf
	${LIBPKT2} ${CURL_LIBRARIES}
)

# Strip binary for release builds
add_custom_command(TARGET proto-db POST_BUILD
	COMMAND "$<$<CONFIG:release>:${CMAKE_STRIP}>" proto-db
)
add_custom_command(TARGET lora-print POST_BUILD
	COMMAND "$<$<CONFIG:release>:${CMAKE_STRIP}>" lora-print
)
add_custom_command(TARGET lorawan-network-server POST_BUILD
	COMMAND "$<$<CONFIG:release>:${CMAKE_STRIP}>" lorawan-network-server
)
add_custom_command(TARGET mac-gw POST_BUILD
	COMMAND "$<$<CONFIG:release>:${CMAKE_STRIP}>" mac-gw
)
add_custom_command(TARGET print-netid POST_BUILD
	COMMAND "$<$<CONFIG:release>:${CMAKE_STRIP}>" print-netid
)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lorawan-network-server DESTINATION cmake)
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/mac-gw DESTINATION cmake)
INSTALL(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${semtech_udp_packet_emitter_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${semtech_udp_packet_emitter_VERSION_MINOR}")
include(CPack)
