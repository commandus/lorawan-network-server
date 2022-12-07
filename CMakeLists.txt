# Options:
# -DENABLE_CURL=on
# -DENABLE_JWT=on
# -DENABLE_PKT2=on
# -DENABLE_LISTENER_EMBEDDED=on
# -DENABLE_MQTT=on
#

cmake_minimum_required(VERSION 3.9)
set(PROJECT_NAME "lorawan-network-n-app-server")
project(${PROJECT_NAME} VERSION 1.0)

set(MQTTAPP_CMAKE_DIR ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH ${MQTTAPP_CMAKE_DIR} ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 11)

option(ENABLE_PKT2 "Build with pkt2" OFF)
option(ENABLE_MQTT "Build with MQTT" OFF)
option(ENABLE_CURL "Build with CURL" OFF)
option(ENABLE_JWT  "Build with JWT" OFF)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(LIB_OS_SPECIFIC_SOURCE usleep.cpp)
endif ()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(ENABLE_JWT)
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
set(LIBJWT OpenSSL::Crypto)
endif(ENABLE_JWT)

#find_package(SQLite3 REQUIRED)
#set(BACKEND_DB_LIB ${SQLite3_LIBRARIES})
#set(BACKEND_DB_INC ${SQLite3_INCLUDE_DIRS})

find_package(unofficial-sqlite3 CONFIG REQUIRED)
set(BACKEND_DB_LIB unofficial::sqlite3::sqlite3)

if(ENABLE_CURL)
	find_package(CURL REQUIRED)
	set(CURL_LIBRARIES curl)
endif(ENABLE_CURL)

if(ENABLE_PKT2)
	find_library(LIBPKT2
		NAMES pkt2
		HINTS "." ".." "../../pkt2" "~/src/pkt2"
	)
endif(ENABLE_PKT2)

if(ENABLE_MQTT)
	find_package(Paho REQUIRED)
	if(NOT Paho_FOUND)
		message(SEND_ERROR "Failed to find Paho C library")
	endif()

	find_package(Paho-C++ REQUIRED)
	if(NOT Paho-C++_FOUND)
		message(SEND_ERROR "Failed to find Paho C++ library")
	endif()
endif(ENABLE_MQTT)

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
	set(SRC_LISTENER_EMBEDDED
			usb-listener.cpp
			lora-gateway-listener.cpp
			gateway-settings.cpp
			gateway-file-json.cpp
			third_party/packet_forwarder/jitqueue.c
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
	semtech-metadata-tx.cpp auth-file.cpp auth-user.cpp payload-insert-plugin.cpp
    utilthread.cpp
	${SRC_AES} ${LIB_OS_SPECIFIC_SOURCE}
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
    ${SRC_LISTENER_EMBEDDED}
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

if(WIN32)
	set (OS_SPECIFIC_LIBS wsock32 ws2_32)
endif()

add_executable(lorawan-network-server ${SRC_LORAWAN_NETWORK_SERVER})
target_link_libraries(lorawan-network-server PRIVATE Threads::Threads
	${LIBPKT2} lorawan ${CURL_LIBRARIES} ${BACKEND_DB_LIB} ws-sqlite microhttpd
	${LIBLORAGW} ${LIBTINYMT32} ${LIBJWT} ${OS_SPECIFIC_LIBS}
)
target_include_directories(lorawan-network-server PRIVATE ${BACKEND_DB_INC} )
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
	${LIBPKT2} lorawan ${BACKEND_DB_LIB} ${OS_SPECIFIC_LIBS}
)
target_include_directories(mac-gw PRIVATE ${BACKEND_DB_INC} )

set(SRC_PRINT_NETID
	print-netid.cpp
	third_party/argtable3/argtable3.c
)
add_executable(print-netid ${SRC_PRINT_NETID})
target_link_libraries(print-netid
	lorawan ${OS_SPECIFIC_LIBS}
)

set(LORA_PRINT_SOURCES
	lora-print.cpp
	database-config-json.cpp config-filename.cpp
	db-any.cpp ${BACKEND_DB_SRC} ${C_STORAGE}
	third_party/argtable3/argtable3.c
	identity-service-dir-txt.cpp
)
add_executable(lora-print ${LORA_PRINT_SOURCES})
target_link_libraries(lora-print PRIVATE Threads::Threads
	${LIBPKT2} lorawan ${BACKEND_DB_LIB} ${OS_SPECIFIC_LIBS}
)
target_include_directories(lora-print PRIVATE ${BACKEND_DB_INC} )

set(PROTO_DB_SOURCES
	proto-db.cpp identity-service-dir-txt.cpp
	db-any.cpp database-config-json.cpp
	${BACKEND_DB_SRC} ${C_STORAGE}
	third_party/argtable3/argtable3.c
)
add_executable(proto-db ${PROTO_DB_SOURCES})
target_link_libraries(proto-db PRIVATE Threads::Threads
	${LIBPKT2} lorawan ${BACKEND_DB_LIB} ${OS_SPECIFIC_LIBS}
)
target_include_directories(proto-db PRIVATE ${BACKEND_DB_INC} )

add_executable(dev-payload ${DEV_PAYLOAD_SOURCES})
target_link_libraries(dev-payload PRIVATE Threads::Threads
	lorawan
	${LIBPKT2} ${OS_SPECIFIC_LIBS}
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

#
# Plugins
#
if(ENABLE_MQTT)
	set (SRC_PLUGIN_MQTT_WSS
		example-plugins/plugin-mqtt-wss.cpp
		mqtt-client-id.cpp utilstring.cpp
	)
	add_library(plugin-mqtt-wss SHARED ${SRC_PLUGIN_MQTT_WSS})
	target_link_libraries(plugin-mqtt-wss paho-mqtt3as paho-mqttpp3)
endif(ENABLE_MQTT)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lorawan-network-server DESTINATION cmake)
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/mac-gw DESTINATION cmake)
INSTALL(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${semtech_udp_packet_emitter_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${semtech_udp_packet_emitter_VERSION_MINOR}")
include(CPack)
