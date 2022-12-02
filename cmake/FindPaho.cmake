#  Try to find paho
#  Once done this will define
#  PAHO_FOUND - System has paho
#  PAHO_ROOT_DIR - The paho root path
#  PAHO_INCLUDE_DIRS - The paho include directories
#  PAHO_LIBRARY_DIRS - The libraries include directories
#  PAHO_DEFINITIONS - Compiler switches required for using paho
# 
#	To satisfy this finder paho has to be installed properly
#   Download https://github.com/eclipse/paho.mqtt.c.git
#   Build in a directory outside sources:
#     mkdir pahoBuild
#	  cd pahoBuild
#	  cmake -G "Visual Studio 12 2013 Win64" -DPAHO_WITH_SSL=FALSE -DPAHO_BUILD_DOCUMENTATION=FALSE -DPAHO_BUILD_SAMPLES=TRUE ../paho.mqtt.c
#	  cd ..
#	  cmake --build ./pahoBuild
#	Install with admin(WIN) privileges or sudo(LIN)
#	  cmake --build ./pahoBuild --target install 
#  Linux specific:
#    Paho is installed in /usr/local/bin and /usr/local/include
#  Windows specific:
#    Paho intallation path has to be specified in the PATH env. variable:
#    C:\Program Files\paho\bin
#    It is used by this finder and the system to find paho*.dll 

find_path(Paho_INCLUDE_DIR "MQTTClient.h")

find_library(Paho_a_LIBRARY NAMES paho-mqtt3a)
find_library(Paho_as_LIBRARY NAMES paho-mqtt3as)
find_library(Paho_c_LIBRARY NAMES paho-mqtt3c)
find_library(Paho_cs_LIBRARY NAMES paho-mqtt3cs)

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LOGGING_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(
	Paho
	DEFAULT_MSG
	Paho_a_LIBRARY Paho_as_LIBRARY Paho_c_LIBRARY Paho_cs_LIBRARY
	Paho_INCLUDE_DIR)

if (Paho_FOUND)
	list(APPEND Paho_LIBRARIES
		${Paho_a_LIBRARY} 
		${Paho_as_LIBRARY}
		${Paho_c_LIBRARY} 
		${Paho_cs_LIBRARY} )
    set(PAHO_DEFINITIONS )
endif()

# Tell cmake GUIs to ignore the "local" variables.
mark_as_advanced(
	Paho_a_LIBRARY Paho_as_LIBRARY Paho_c_LIBRARY Paho_cs_LIBRARY
	Paho_INCLUDE_DIR)
