# - Find libmqtt
# Find the native libmosquitto includes and libraries
#
#  mqtt_INCLUDE_DIR - where to find mosquitto.h, etc.
#  mqtt_LIBRARIES   - List of libraries when using libmosquitto.
#  mqtt_FOUND       - True if libmosquitto found.

if (NOT mqtt_INCLUDE_DIR)
  find_path(mqtt_INCLUDE_DIR mosquitto.h)
endif()

if (NOT mqtt_LIBRARY)
  find_library(
    mqtt_LIBRARY
    NAMES mosquitto)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  mqtt DEFAULT_MSG
  mqtt_LIBRARY mqtt_INCLUDE_DIR)

message(STATUS "libmosquitto include dir: ${mqtt_INCLUDE_DIR}")
message(STATUS "libmosquitto: ${mqtt_LIBRARY}")
set(mqtt_LIBRARIES ${mqtt_LIBRARY})

mark_as_advanced(mqtt_INCLUDE_DIR mqtt_LIBRARY)
