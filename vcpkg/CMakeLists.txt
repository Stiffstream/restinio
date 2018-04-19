cmake_minimum_required(VERSION 3.3)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

option(RESTINIO_INSTALL "Generate the install target." ON)

SET(RESTINIO_USE_BOOST_ASIO "none" CACHE STRING "Use boost version of ASIO")

SET( RESTINIO_STAND_ALONE_ASIO_DEFINES -DASIO_STANDALONE -DASIO_HAS_STD_CHRONO -DASIO_DISABLE_STD_STRING_VIEW)

find_package(nodejs_http_parser)
find_package(fmt)

SET(RESTINIO_VCPKG_BUILD ON)

# RESTinio itself.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../dev/restinio ${CMAKE_CURRENT_BINARY_DIR}/restinio)
