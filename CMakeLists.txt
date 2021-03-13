cmake_minimum_required(VERSION 3.9)
project(lorawan-network-server VERSION 1.0)

INCLUDE_DIRECTORIES("third_party")

set(SRCLMDB 
	db-identity.cpp identity-service-lmdb.cpp
)

set(SRC 
	lorawan-network-server.cpp errlist.cpp daemonize.cpp utilstring.cpp utillora.cpp utildate.cpp
	strptime.cpp
	gateway-stat.cpp lora-packet-handler-impl.cpp packet-queue.cpp udp-listener.cpp
	config-json.cpp identity-service-file-json.cpp lorawan-mac.cpp udp-socket.cpp
    gateway-list.cpp udp-emitter.cpp 
	third_party/argtable3/argtable3.c
	third_party/system/crypto/aes.c third_party/system/crypto/cmac.c
	third_party/base64/base64.cpp
)
add_executable(lorawan-network-server ${SRC})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(lorawan-network-server PRIVATE Threads::Threads)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lorawan-network-server DESTINATION cmake)
INSTALL(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${semtech_udp_packet_emitter_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${semtech_udp_packet_emitter_VERSION_MINOR}")
include(CPack)
