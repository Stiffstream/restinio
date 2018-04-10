cmake_minimum_required(VERSION 3.3)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(RESTINIO_USE_BOOST_ASIO "none" CACHE STRING "Use boost version of ASIO")
SET(RESTINIO_USE_BOOST_ASIO_VALUES "none;static;shared")

set_property(CACHE RESTINIO_USE_BOOST_ASIO PROPERTY STRINGS ${RESTINIO_USE_BOOST_ASIO_VALUES})

IF ("x${CMAKE_CXX_COMPILER_ID}" MATCHES "xGNU"
		OR "x${CMAKE_CXX_COMPILER_ID}" MATCHES "xClang"
		OR "x${CMAKE_CXX_COMPILER_ID}" MATCHES "xIntel")
	add_definitions(-pthread -mtune=native -march=native)
	SET(CMAKE_EXE_LINKER_FLAGS "-pthread")
	SET(CMAKE_SHARED_LINKER_FLAGS "-pthread")
ELSEIF (${MSVC})
ELSE ()
	message(FATAL_ERROR "Your C++ compiler (ID: '${CMAKE_CXX_COMPILER_ID}') not supported.")
ENDIF ()

# ------------------------------------------------------------------------------
# BOOST
IF ( RESTINIO_USE_BOOST_ASIO MATCHES "none" )
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/asio/include)
	add_definitions(-DASIO_STANDALONE -DASIO_HAS_STD_CHRONO -DASIO_DISABLE_STD_STRING_VIEW)

	# Try to find boost for boost regex.
	SET(Boost_USE_MULTITHREADED ON)
	find_package(Boost COMPONENTS regex)
	if ( Boost_FOUND )
		include_directories( ${Boost_INCLUDE_DIRS} )
	endif ()

ELSE ()
	SET(Boost_USE_MULTITHREADED ON)
	IF ( RESTINIO_USE_BOOST_ASIO MATCHES "shared" )
		SET(Boost_USE_STATIC_LIBS OFF)
	ELSE ()
		SET(Boost_USE_STATIC_LIBS ON)
	ENDIF ()

	find_package(Boost 1.66.0 REQUIRED COMPONENTS system regex)

	include_directories( ${Boost_INCLUDE_DIRS} )
	add_definitions( -DRESTINIO_USE_BOOST_ASIO )
ENDIF ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

SET(FMT_INSTALL ON)
add_subdirectory(fmt)

# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/args)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)
# add_definitions(-DRAPIDJSON_HAS_STDSTRING -DRAPIDJSON_HAS_CXX11_RVALUE_REFS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# ------------------------------------------------------------------------------
# OpenSSL
find_package(OpenSSL)
IF ( OPENSSL_FOUND )
	message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
	message("OpenSSL libraries: ${OPENSSL_LIBRARIES}")
ENDIF ( OPENSSL_FOUND )

# ------------------------------------------------------------------------------
# PCRE*
find_package(PCRE)
IF (PCRE_FOUND)
	message( STATUS "PCRE_LIBRARIES='" ${PCRE_LIBRARIES} "'" )
	message( STATUS "PCRE_INCLUDE_DIRS='" ${PCRE_INCLUDE_DIRS} "'" )
ENDIF ()

find_package(PCRE2)
IF (PCRE2_FOUND)
	add_definitions(-DPCRE2_STATIC)
	add_definitions(-DPCRE2_CODE_UNIT_WIDTH=8)

	message( STATUS "PCRE2_LIBRARIES='" ${PCRE2_LIBRARIES} "'" )
	message( STATUS "PCRE2_INCLUDE_DIRS='" ${PCRE2_INCLUDE_DIRS} "'" )
ENDIF ()

# HTTP parser
add_subdirectory(nodejs/http_parser)

# ------------------------------------------------------------------------------
# SObjectizer
SET(SOBJECTIZER_BUILD_STATIC ON)
SET(SOBJECTIZER_LIBS sobjectizer::StaticLib)
add_subdirectory(so_5)

# ------------------------------------------------------------------------------
# Zlib
find_package(ZLIB)

IF (NOT ZLIB_FOUND)
	add_subdirectory(restinio/third_party/zlib)
	message( STATUS "USE OWN ZLIB SOURCES")
	SET(ZLIB_LIBRARIES zlibstatic)
	SET(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/restinio/third_party/zlib)
ENDIF ()

message( STATUS "ZLIB_LIBRARIES='" ${ZLIB_LIBRARIES} "'" )
message( STATUS "ZLIB_INCLUDE_DIRS='" ${ZLIB_INCLUDE_DIRS} "'" )


# RESTinio itself.
add_subdirectory(restinio)

# ------------------------------------------------------------------------------
# Tests
enable_testing()
add_subdirectory(test)

IF (WIN32)
	configure_file(${CMAKE_SOURCE_DIR}/cmake/run_tests.bat ${CMAKE_BINARY_DIR} NEWLINE_STYLE WIN32)
ENDIF ()

# ------------------------------------------------------------------------------
# Samples
add_subdirectory(sample)

# ------------------------------------------------------------------------------
# Benches
add_subdirectory(benches)
