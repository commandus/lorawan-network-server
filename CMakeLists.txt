#
# Options:
#
# -DENABLE_WS=on   enable embedded web service
# -DENABLE_CURL=on enable Curl lib (read/write statistics to remote https:// )
# -DENABLE_JWT=on  enable embedded web service user authentication
#
# Semtech protocol UDP listener
# -DENABLE_LISTENER_UDP=on
#
# Embedded USB gateway in the network server
#
# -DENABLE_LISTENER_EMBEDDED=on
#
# Build standalone USB gateway
#
# -DENABLE_LORAWAN_GATEWAY=on
#
#  Plugins:
#
# -DENABLE_PKT2=on
# -DENABLE_MQTT=on
# -DENABLE_LOGGER_HUFFMAN=on
#

cmake_minimum_required(VERSION 3.9)
set(PROJECT_NAME "lorawan-network-n-app-server")
project(${PROJECT_NAME} VERSION 1.0)

set(MQTTAPP_CMAKE_DIR ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH ${MQTTAPP_CMAKE_DIR} ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 11)

option(ENABLE_CURL "Build with CURL" OFF)
option(ENABLE_JWT  "Build with JWT" OFF)

# Plugins
option(ENABLE_PKT2 "Build pkt2 plugin" OFF)
option(ENABLE_MQTT "Build MQTT plugin" OFF)
option(ENABLE_LOGGER_HUFFMAN "Build logger-huffman plugin" OFF)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(LIB_OS_SPECIFIC_SOURCE usleep.cpp)
	find_package(unofficial-sqlite3 CONFIG REQUIRED)
	set(BACKEND_DB_LIB unofficial::sqlite3::sqlite3)
else ()
	find_package(SQLite3 REQUIRED)
	set(BACKEND_DB_LIB ${SQLite3_LIBRARIES})
	set(BACKEND_DB_INC ${SQLite3_INCLUDE_DIRS})
endif ()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(ENABLE_JWT)
	set(OPENSSL_USE_STATIC_LIBS TRUE)
	find_package(OpenSSL REQUIRED)
	set(LIBJWT OpenSSL::Crypto)
	set (LNS_DEF ${LNS_DEF} ENABLE_JWT)
endif(ENABLE_JWT)

set(WS_SRC ws-handler.cpp)
if(ENABLE_WS)
	find_library(WS_LIB_SQLITE
		NAMES ws-sqlite
		HINTS "." ".." "../../lorawan-ws" "~/src/lorawan-ws"
			"../lorawan-ws/build/Release"
	)
	set(WS_LIB ${WS_LIB_SQLITE} microhttpd)
	set (LNS_DEF ${LNS_DEF} ENABLE_WS)
endif(ENABLE_WS)

if(ENABLE_CURL)
	find_package(CURL REQUIRED)
	set(CURL_LIBRARIES curl)
endif(ENABLE_CURL)

if(ENABLE_PKT2)
	find_library(LIBPKT2
		NAMES pkt2
		HINTS "." ".." "../../pkt2" "~/src/pkt2"
			"../pkt2/build/Release"
	)
endif(ENABLE_PKT2)

if(ENABLE_LOGGER_HUFFMAN)
	find_library(LIBLOGGER_PASSPORT
		NAMES logger-passport
		HINTS "." ".." "../../logger-passport" "~/src/logger-passport"
		"../logger-passport/build/Release"
	)
	find_library(LIBLOGGER_HUFFMAN
		NAMES logger-huffman
		HINTS "." ".." "../../logger-huffman" "~/src/logger-huffman"
		"../logger-huffman/build/Release"
	)
endif(ENABLE_LOGGER_HUFFMAN)

if(ENABLE_MQTT)
	find_package(OpenSSL REQUIRED)
	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		find_package(eclipse-paho-mqtt-c CONFIG REQUIRED)
		# target_link_libraries(main PRIVATE eclipse-paho-mqtt-c::paho-mqtt3a-static eclipse-paho-mqtt-c::paho-mqtt3c-static
		# eclipse-paho-mqtt-c::paho-mqtt3as-static eclipse-paho-mqtt-c::paho-mqtt3cs-static)
		find_package(PahoMqttCpp CONFIG REQUIRED)
		set(LIB_PAHO_MQTT
			OpenSSL::Crypto
			eclipse-paho-mqtt-c::paho-mqtt3as
			PahoMqttCpp::paho-mqttpp3
		)
	else()
		find_package(Paho REQUIRED)
		if(NOT Paho_FOUND)
			message(SEND_ERROR "Failed to find Paho C library")
		endif()

		find_package(Paho-C++ REQUIRED)
		if(NOT Paho-C++_FOUND)
			message(SEND_ERROR "Failed to find Paho C++ library")
		endif()
		set(LIB_PAHO_MQTT OpenSSL::Crypto paho-mqtt3as paho-mqttpp3)
	endif()
endif(ENABLE_MQTT)

option(ENABLE_LISTENER_UDP "Build with Semtech UDP protocol" ON)
option(ENABLE_LISTENER_EMBEDDED "Build with embedded LoRaWAN gateway" OFF)
option(ENABLE_LORAWAN_GATEWAY "Build with standalone LoRaWAN gateway" OFF)

set(SRC_LISTENER_EMBEDDED
	usb-listener.cpp
	lora-gateway-listener.cpp
	gateway-settings.cpp
	gateway-file-json.cpp
	subst-call-c.cpp libloragw-helper.cpp
	third_party/packet_forwarder/jitqueue.c
)

if(ENABLE_LISTENER_EMBEDDED OR ENABLE_LORAWAN_GATEWAY)
	find_library(LIBLORAGW
			NAMES loragw
			HINTS
				"../libloragw" "../libloragw/build" "../libloragw/build/Release"
				"../../libloragw" "../../libloragw/build" "../../libloragw/build/Release"
				"~/src/libloragw" "~/src/libloragw/build" "~/src/libloragw/build/Release"
				"third_party/packet_forwarder"
				"." ".."
				"../../packet_forwarder"
				"~/src/packet_forwarder"
				"../packet_forwarder/build/Release"
	)
	#
	# tinymt32 added to the loragw lib
	# find_library(LIBTINYMT32 NAMES tinymt32
	#		HINTS "third_party/packet_forwarder" "." ".." "../../packet_forwarder" "~/src/packet_forwarder" "../packet_forwarder/build/Release"
	# )
	#
endif ()

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
	lorawan-network-server.cpp run-listener.cpp daemonize.cpp
	packet-listener.cpp config-json.cpp lora-packet-handler-impl.cpp config-filename.cpp
	identity-service-dir-txt.cpp ${C_STORAGE} ${C_QUEUE_STORAGE}
	gateway-stat-service-file.cpp gateway-stat-service-post.cpp
	device-stat-service-file.cpp device-stat-service-post.cpp
	db-any.cpp ${BACKEND_DB_SRC}
	device-channel-plan-file-json.cpp device-history-service-json.cpp
	regional-parameter-channel-plan-file-json.cpp
	database-config-json.cpp
	third_party/get_rss/get_rss.c
	third_party/argtable3/argtable3.c
)

if(ENABLE_WS)
	set(SRC_LORAWAN_NETWORK_SERVER ${SRC_LORAWAN_NETWORK_SERVER} ${WS_SRC})
endif()

if(ENABLE_LISTENER_EMBEDDED)
	set(SRC_LORAWAN_NETWORK_SERVER ${SRC_LORAWAN_NETWORK_SERVER} ${SRC_LISTENER_EMBEDDED})
endif()

if(ENABLE_LISTENER_UDP)
	set(SRC_LORAWAN_NETWORK_SERVER ${SRC_LORAWAN_NETWORK_SERVER} udp-listener.cpp)
endif()

#
# Auto generated file gateway_usb_conf.cpp:
#   make gateway-config2cpp
#   ./gateway-config2cpp /home/andrei/git/rak_common_for_gateway/lora/rak2287/global_conf_usb/*.json > gateway_usb_conf.cpp
#   g++ -std=c++11 -I. -Ithird_party -I../../third_party gateway_usb_conf.cpp
#
set (SRC_LORAWAN_GATEWAY
	lorawan-gateway.cpp usb-listener.cpp lora-gateway-listener.cpp gateway-settings.cpp
	identity-service-file-json.cpp client-id.cpp packet-listener.cpp daemonize.cpp
	libloragw-helper.cpp subst-call-c.cpp
	third_party/packet_forwarder/jitqueue.c
)

set(DEV_PAYLOAD_SOURCES
	dev-payload.cpp config-json.cpp config-filename.cpp
	${C_STORAGE}
    third_party/argtable3/argtable3.c
)

add_library(lorawan STATIC
	${LIBLORAWANSRC}
)
# -fPIC
set_property(TARGET lorawan PROPERTY POSITION_INDEPENDENT_CODE ON)
# auth-file depends on JWT
target_compile_definitions(lorawan PRIVATE ${LNS_DEF} ENABLE_DB_SQLITE)
target_link_libraries(lorawan PRIVATE ${LIBJWT}
)

#if(WIN32)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	# avoid Error LNK2038 mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'
	# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

	if(ENABLE_MQTT)
		set (OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv iphlpapi)
	else()
		set (OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif()
endif()

set (LNS_DEF ${LNS_DEF} )
if (ENABLE_LISTENER_UDP)
	set (LNS_DEF ${LNS_DEF} ENABLE_LISTENER_UDP )
endif()
if (ENABLE_LISTENER_EMBEDDED)
	set (LNS_DEF ${LNS_DEF} ENABLE_LISTENER_EMBEDDED )
endif()

add_executable(lorawan-network-server ${SRC_LORAWAN_NETWORK_SERVER})
target_link_libraries(lorawan-network-server PRIVATE Threads::Threads
	lorawan ${WS_LIB} ${CURL_LIBRARIES} ${BACKEND_DB_LIB}
	${LIBLORAGW} ${LIBJWT} ${OS_SPECIFIC_LIBS}
)
target_include_directories(lorawan-network-server PRIVATE ${BACKEND_DB_INC} )
target_compile_definitions(lorawan-network-server PRIVATE ${LNS_DEF} ENABLE_DB_SQLITE)

set(SRC_MAC_GW
	mac-gw.cpp macgw-config-json.cpp config-json.cpp lora-packet-handler-impl.cpp
	config-filename.cpp
	${C_STORAGE} db-any.cpp  ${BACKEND_DB_SRC}
	receiver-queue-service-file-json.cpp receiver-queue-service-dir-txt.cpp
	third_party/argtable3/argtable3.c
)

add_executable(mac-gw ${SRC_MAC_GW})
target_link_libraries(mac-gw PRIVATE Threads::Threads
	lorawan ${BACKEND_DB_LIB} ${OS_SPECIFIC_LIBS}
)
target_include_directories(mac-gw PRIVATE ${BACKEND_DB_INC} )

set (SRC_MAC_NS
	mac-ns.cpp macgw-config-json.cpp config-json.cpp lora-packet-handler-impl.cpp config-filename.cpp
	receiver-queue-service-file-json.cpp receiver-queue-service-dir-txt.cpp device-history-service-json.cpp control-packet.cpp
	${C_STORAGE} db-any.cpp  ${BACKEND_DB_SRC}
	third_party/argtable3/argtable3.c
)
add_executable(mac-ns ${SRC_MAC_NS})
target_link_libraries(mac-ns PRIVATE Threads::Threads
	lorawan ${BACKEND_DB_LIB} ${OS_SPECIFIC_LIBS}
)
target_include_directories(mac-ns PRIVATE ${BACKEND_DB_INC} )

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
	lorawan ${BACKEND_DB_LIB} ${OS_SPECIFIC_LIBS}
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
	lorawan ${BACKEND_DB_LIB} ${OS_SPECIFIC_LIBS}
)
target_include_directories(proto-db PRIVATE ${BACKEND_DB_INC} )

add_executable(dev-payload ${DEV_PAYLOAD_SOURCES})
target_link_libraries(dev-payload PRIVATE Threads::Threads
	lorawan	${OS_SPECIFIC_LIBS}
)

add_executable(gateway-config2cpp gateway-config2cpp.cpp gateway-settings.cpp gateway-file-json.cpp utilstring.cpp
	third_party/argtable3/argtable3.c
)
target_link_libraries(gateway-config2cpp PRIVATE ${OS_SPECIFIC_LIBS})

if (ENABLE_LORAWAN_GATEWAY)
	add_executable(lorawan-gateway ${SRC_LORAWAN_GATEWAY}
		third_party/argtable3/argtable3.c
	)
	target_link_libraries(lorawan-gateway PRIVATE Threads::Threads
		lorawan ${LIBLORAGW} ${OS_SPECIFIC_LIBS}
	)
	# target_include_directories(lorawan-gateway PRIVATE ${BACKEND_DB_INC} )
	# target_compile_definitions(lorawan-gateway PRIVATE ${LNS_DEF})
endif()

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
		client-id.cpp utilstring.cpp
	)
	add_library(plugin-mqtt-wss SHARED ${SRC_PLUGIN_MQTT_WSS})
	target_link_libraries(plugin-mqtt-wss lorawan ${LIB_PAHO_MQTT})
endif(ENABLE_MQTT)

if(ENABLE_PKT2)
	set (SRC_PLUGIN_PKT2
		example-plugins/plugin-pkt2.cpp
	)
	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		set (OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif ()
	add_library(plugin-pkt2 SHARED ${SRC_PLUGIN_PKT2})
	target_link_libraries(plugin-pkt2 lorawan ${LIBPKT2} ${OS_SPECIFIC_LIBS})
endif(ENABLE_PKT2)

if(ENABLE_LOGGER_HUFFMAN)
	set (SRC_PLUGIN_LOGGER_HUFFMAN
		example-plugins/plugin-logger-huffman.cpp logger-loader.cpp
		db-any.cpp ${BACKEND_DB_SRC}
	)
	add_library(plugin-logger-huffman SHARED ${SRC_PLUGIN_LOGGER_HUFFMAN})
	target_include_directories(plugin-logger-huffman PRIVATE ${BACKEND_DB_INC} )
	target_compile_definitions(plugin-logger-huffman PRIVATE ENABLE_DB_SQLITE)

	if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
		set (OS_SPECIFIC_LIBS wsock32 ws2_32 Userenv)
	endif ()
	target_link_libraries(plugin-logger-huffman lorawan ${LIBLOGGER_PASSPORT} ${LIBLOGGER_HUFFMAN} ${BACKEND_DB_LIB} ${OS_SPECIFIC_LIBS})
endif(ENABLE_LOGGER_HUFFMAN)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lorawan-network-server DESTINATION cmake)
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/mac-gw DESTINATION cmake)
INSTALL(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "3")
include(CPack)
