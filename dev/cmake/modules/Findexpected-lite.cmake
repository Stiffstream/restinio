# - Try to find expected-lite
#
# Defines the following variables:
#
# expected-lite_FOUND - system has expected-lite
# expected-lite_INCLUDE_DIR - the expected-lite include directory
# expected-lite_LIBRARIES - Link these to use expected-lite

# Find the header and library
find_path(expected-lite_INCLUDE_DIR NAMES nonstd/expected.hpp
    HINTS "${RESTINIO_EXPECTED_LITE_PATH_HINT}"
    PATH_SUFFIXES include)

if(expected-lite_INCLUDE_DIR)
    set(expected-lite_FOUND TRUE)
endif()

if(expected-lite_FOUND)
    add_library(expected-lite::expected-lite INTERFACE IMPORTED)

    set(expected-lite_INCLUDE_DIRS ${expected-lite_INCLUDE_DIR})
    set_target_properties(expected-lite::expected-lite PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                          "${expected-lite_INCLUDE_DIRS}")

    MESSAGE(STATUS "expected-lite libraries: ${expected-lite_LIBRARIES}")
    MESSAGE(STATUS "expected-lite include dir: ${expected-lite_INCLUDE_DIRS}")
endif()

