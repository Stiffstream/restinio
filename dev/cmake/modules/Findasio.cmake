# - Try to find asio
#
# Defines the following variables:
#
# asio_FOUND - system has asio
# asio_INCLUDE_DIR - the asio include directory
# asio_LIBRARIES - Link these to use asio

# Find the header and library
find_path(asio_INCLUDE_DIR NAMES asio.hpp
    HINTS "${RESTINIO_ASIO_PATH_HINT}"
    PATH_SUFFIXES include)

if(asio_INCLUDE_DIR)
    set(asio_FOUND TRUE)
endif()

if(asio_FOUND)
    add_library(asio::asio INTERFACE IMPORTED)

    set(asio_INCLUDE_DIRS ${asio_INCLUDE_DIR})
    set_target_properties(asio::asio PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                          "${asio_INCLUDE_DIRS}")

    MESSAGE(STATUS "asio libraries: ${asio_LIBRARIES}")
    MESSAGE(STATUS "asio include dir: ${asio_INCLUDE_DIRS}")
endif()

