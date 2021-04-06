cmake_minimum_required(VERSION 3.9)
project(lorawan-network-server VERSION 1.0)

INCLUDE_DIRECTORIES("third_party")

set(SRC_AES
	third_party/system/crypto/aes.c third_party/system/crypto/cmac.c
)

set(SRC_STORAGE
	identity-service-file-json.cpp
)

set(LIB_STORAGE
)

#	set(SRC_STORAGE
#		identity-service-lmdb.cpp db-identity.cpp
#	)
#	set(LIB_STORAGE
#		-lmdbx
#	)


set(SRC_LORAWAN_NETWORK_SERVER
	lorawan-network-server.cpp errlist.cpp daemonize.cpp utilstring.cpp utillora.cpp utildate.cpp
	strptime.cpp gateway-stat.cpp lora-packet-handler-impl.cpp packet-queue.cpp udp-listener.cpp
	config-json.cpp lorawan-mac.cpp udp-socket.cpp gateway-list.cpp udp-emitter.cpp config-filename.cpp
	${SRC_STORAGE} ${SRC_AES}
	third_party/argtable3/argtable3.c
	third_party/base64/base64.cpp
)
add_executable(lorawan-network-server ${SRC_LORAWAN_NETWORK_SERVER})

set(SRC_MAC_GW
	mac-gw.cpp macgw-config-json.cpp errlist.cpp
	utillora.cpp udp-socket.cpp udp-emitter.cpp
	config-json.cpp identity-service-file-json.cpp
	packet-queue.cpp gateway-stat.cpp gateway-list.cpp lorawan-mac.cpp
	lora-packet-handler-impl.cpp config-filename.cpp
	utilstring.cpp utildate.cpp strptime.cpp
	${SRC_STORAGE} ${SRC_AES}
	third_party/argtable3/argtable3.c
	third_party/base64/base64.cpp

)
add_executable(mac-gw ${SRC_MAC_GW})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(lorawan-network-server PRIVATE Threads::Threads)
target_link_libraries(mac-gw PRIVATE Threads::Threads)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lorawan-network-server DESTINATION cmake)
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/mac-gw DESTINATION cmake)
INSTALL(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${semtech_udp_packet_emitter_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${semtech_udp_packet_emitter_VERSION_MINOR}")
include(CPack)
