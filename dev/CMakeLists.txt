cmake_minimum_required(VERSION 3.3)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RESTINIO_USE_BOOST_ASIO "none" CACHE STRING "Use boost version of ASIO")

set(RESTINIO_USE_BOOST_ASIO_VALUES "none;static;shared")

set_property(CACHE RESTINIO_USE_BOOST_ASIO PROPERTY STRINGS ${RESTINIO_USE_BOOST_ASIO_VALUES})

if ("x${CMAKE_CXX_COMPILER_ID}" MATCHES "xGNU"
		OR "x${CMAKE_CXX_COMPILER_ID}" MATCHES "xClang"
		OR "x${CMAKE_CXX_COMPILER_ID}" MATCHES "xIntel")
	add_definitions(-pthread -mtune=native -march=native)
	set(CMAKE_EXE_LINKER_FLAGS "-pthread")
	set(CMAKE_SHARED_LINKER_FLAGS "-pthread")
elseif (${MSVC})
else ()
	message(FATAL_ERROR "Your C++ compiler (ID: '${CMAKE_CXX_COMPILER_ID}') not supported.")
endif ()

# ------------------------------------------------------------------------------
# BOOST
if ( RESTINIO_USE_BOOST_ASIO MATCHES "none" )
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/asio/include)
	add_definitions(-DASIO_STANDALONE -DASIO_HAS_STD_CHRONO -DASIO_DISABLE_STD_STRING_VIEW)

	# Try to find boost for boost regex.
	set(Boost_USE_MULTITHREADED ON)
	find_package(Boost COMPONENTS regex)
	if ( Boost_FOUND )
		include_directories( ${Boost_INCLUDE_DIRS} )
	endif ()

else ()
	set(Boost_USE_MULTITHREADED ON)
	if ( RESTINIO_USE_BOOST_ASIO MATCHES "shared" )
		set(Boost_USE_STATIC_LIBS OFF)
	else ()
		set(Boost_USE_STATIC_LIBS ON)
	endif ()

	find_package(Boost 1.66.0 REQUIRED COMPONENTS system regex)

	include_directories( ${Boost_INCLUDE_DIRS} )
	add_definitions( -DRESTINIO_USE_BOOST_ASIO )
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/fmt)
add_definitions(-DFMT_HEADER_ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/args)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)
add_definitions(-DRAPIDJSON_HAS_STDSTRING -DRAPIDJSON_HAS_CXX11_RVALUE_REFS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")


# ------------------------------------------------------------------------------
# OpenSSL
find_package(OpenSSL)

# ------------------------------------------------------------------------------
# PCRE*
find_package(PCRE)
find_package(PCRE2)
IF(PCRE2_FOUND)
  add_definitions(-DPCRE2_STATIC)
  add_definitions(-DPCRE2_CODE_UNIT_WIDTH=8)
ENDIF(PCRE2_FOUND)

# HTTP parser
add_subdirectory(nodejs/http_parser)

# ------------------------------------------------------------------------------
# SObjectizer
set(SOBJECTIZER_BUILD_STATIC ON)
set(SOBJECTIZER_LIBS sobjectizer::StaticLib)
add_subdirectory(so_5)

# ------------------------------------------------------------------------------
# Zlib
find_package(ZLIB)

if (NOT ZLIB_FOUND)
  add_subdirectory(restinio/third_party/zlib)
  message( STATUS "USE OWN ZLIB SOURCES")
  SET(ZLIB_LIBRARIES zlibstatic)
  SET(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/restinio/third_party/zlib)
endif ()
message( STATUS "ZLIB_LIBRARIES='" ${ZLIB_LIBRARIES} "'" )
message( STATUS "ZLIB_INCLUDE_DIRS='" ${ZLIB_INCLUDE_DIRS} "'" )

include_directories( ${ZLIB_INCLUDE_DIRS} )

# ------------------------------------------------------------------------------
# Tests
enable_testing()
add_subdirectory(test)

if(WIN32)
	configure_file(${CMAKE_SOURCE_DIR}/cmake/run_tests.bat ${CMAKE_BINARY_DIR} NEWLINE_STYLE WIN32)
endif()

# ------------------------------------------------------------------------------
# Samples
add_subdirectory(sample)

# ------------------------------------------------------------------------------
# Benches
add_subdirectory(benches)
