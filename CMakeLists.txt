cmake_minimum_required(VERSION 3.9)

project(Opera VERSION 1.0)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.20)
        message(FATAL_ERROR "MSVC version must be at least 19.20!")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.2)
        message(FATAL_ERROR "GCC version must be at least 10.2!")
    endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.0)
        message(FATAL_ERROR "Clang version must be at least 11.0!")
    endif()
else()
    message(WARNING "You are using an unsupported compiler! MSVC, GCC and Clang are supported.")
endif()

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    set(LIBRARY_KIND STATIC)
else()
    set(LIBRARY_KIND SHARED)	
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MACOSX_RPATH 1)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
    set(ANY_TARGET_WARN all extra pedantic sign-conversion cast-qual disabled-optimization
        init-self missing-include-dirs sign-promo switch-default undef redundant-decls
        strict-aliasing unused-parameter shadow error)
    foreach(WARN ${ANY_TARGET_WARN})
        add_compile_options(-W${WARN})
    endforeach()
else()
    add_compile_options(/WX)
endif()

include(CheckTypeSize)
check_type_size("size_t" SIZEOF_SIZET)
message(STATUS "size_t size: ${SIZEOF_SIZET}")
check_type_size("long int" SIZEOF_LONG_INT)
message(STATUS "long int size: ${SIZEOF_LONG_INT}")
check_type_size("long long int" SIZEOF_LONG_LONG_INT)
message(STATUS "long long int size: ${SIZEOF_LONG_LONG_INT}")

find_package(rapidjson REQUIRED)
find_package(mqtt REQUIRED)
find_package(Threads REQUIRED)

set(MQTT_HOST "localhost" CACHE STRING "Host for testing MQTT functionality, defaulting to localhost")
set(RESOURCES_PATH ${PROJECT_SOURCE_DIR}/resources/)
configure_file(${PROJECT_SOURCE_DIR}/cmake/config.hpp.cmake ${PROJECT_SOURCE_DIR}/include/config.hpp)

include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/dependencies ${mqtt_INCLUDE_DIR} ${RAPIDJSON_INCLUDEDIR})

add_subdirectory(src)

enable_testing()
add_subdirectory(test)

add_subdirectory(dependencies)

add_subdirectory(profile)

add_library(opera ${LIBRARY_KIND} $<TARGET_OBJECTS:OPERA_SRC>)
target_link_libraries(opera conclog ${mqtt_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

add_executable(operad src/operad.cpp)
target_link_libraries(operad opera)

if(NOT TARGET coverage_config)
    add_library(coverage_config INTERFACE)
endif()

option(COVERAGE "Enable coverage reporting" OFF)
if(COVERAGE)
    message(STATUS "Enabled coverage reporting")
    target_compile_options(coverage_config INTERFACE -O0 -g --coverage)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
        target_link_options(coverage_config INTERFACE --coverage)
    else()
        target_link_libraries(coverage_config INTERFACE --coverage)
    endif()
    target_link_libraries(opera coverage_config)
endif()

install(TARGETS opera DESTINATION lib)
install(DIRECTORY include/ DESTINATION include/opera FILES_MATCHING PATTERN "*.hpp")
