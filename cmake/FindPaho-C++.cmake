find_path(Paho-C++_INCLUDE_DIR "mqtt/async_client.h")
find_library(Paho-C++_LIBRARY NAMES paho-mqttpp3)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	Paho-C++
	DEFAULT_MSG
	Paho-C++_LIBRARY Paho-C++_INCLUDE_DIR)

if (PAHO_FOUND)
	set(PAHO_CPP_LIBRARIES ${PAHO_CPP_LIBRARY})
    set(PAHO_CPP_DEFINITIONS)
endif()

mark_as_advanced(Paho-C++_LIBRARY Paho-C++_INCLUDE_DIR)