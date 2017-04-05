cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

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
add_definitions(-DASIO_STANDALONE -DASIO_HAS_STD_CHRONO)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/fmt)
add_definitions(-DFMT_HEADER_ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(nodejs/http_parser)

add_subdirectory(sample)

