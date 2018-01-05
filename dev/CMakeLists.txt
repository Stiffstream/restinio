cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# option(RESTINIO_USE_BOOST_ASIO "Build using boost::asio [default: OFF]." OFF )

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/asio/include)
add_definitions(-DASIO_STANDALONE -DASIO_HAS_STD_CHRONO -DASIO_DISABLE_STD_STRING_VIEW)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/fmt)
add_definitions(-DFMT_HEADER_ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/args)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)
add_definitions(-DRAPIDJSON_HAS_STDSTRING -DRAPIDJSON_HAS_CXX11_RVALUE_REFS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")


find_package(OpenSSL)
find_package(PCRE)
find_package(PCRE2)
IF(PCRE2_FOUND)
  add_definitions(-DPCRE2_STATIC)
  add_definitions(-DPCRE2_CODE_UNIT_WIDTH=8)
ENDIF(PCRE2_FOUND)

add_subdirectory(nodejs/http_parser)

set(SOBJECTIZER_BUILD_STATIC ON)
set(SOBJECTIZER_LIBS sobjectizer::StaticLib)
add_subdirectory(so_5)

enable_testing()
add_subdirectory(test)

if(WIN32)
	configure_file(${CMAKE_SOURCE_DIR}/cmake/run_tests.bat ${CMAKE_BINARY_DIR} NEWLINE_STYLE WIN32)
endif()


add_subdirectory(sample)
add_subdirectory(benches)
