cmake_minimum_required(VERSION 3.3)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

option(RESTINIO_INSTALL "Generate the install target." ON)
option(RESTINIO_FMT_HEADER_ONLY "Include fmt library as header-only." ON)

SET(RESTINIO_USE_BOOST_ASIO "none" CACHE STRING "Use boost version of ASIO")

SET( RESTINIO_STAND_ALONE_ASIO_DEFINES -DASIO_STANDALONE -DASIO_HAS_STD_CHRONO -DASIO_DISABLE_STD_STRING_VIEW)

# http_parser must be found.
find_package(unofficial-http-parser REQUIRED)
# fmt must be found.
find_package(fmt REQUIRED)

option(RESTINIO_FIND_DEPS "Use `find_package()` for including RESTinio dependencies." ON)

# RESTinio:
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../dev/restinio ${CMAKE_CURRENT_BINARY_DIR}/restinio)
