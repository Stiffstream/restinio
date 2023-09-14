cmake_minimum_required(VERSION 3.5.1)
project(llhttp C)

set(CMAKE_C_STANDARD 99)

# Source code
set(LLHTTP_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/llhttp.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/http.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/api.c
)

set(LLHTTP_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/include/llhttp.h
)

# On windows with Visual Studio, add a debug postfix so that release
# and debug libraries can coexist.
if(MSVC)
	set(CMAKE_DEBUG_POSTFIX "d")
endif()

if(BUILD_SHARED_LIBS)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_library(llhttp ${LLHTTP_SOURCES} ${LLHTTP_HEADERS})
add_library(unofficial::llhttp::llhttp ALIAS llhttp)

target_include_directories(llhttp PUBLIC
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>)

install(
	TARGETS llhttp
	EXPORT NODEJS_LLHTTP_ALL_TARGETS
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
)

install(
	EXPORT NODEJS_LLHTTP_ALL_TARGETS
	NAMESPACE unofficial::llhttp::
	FILE unofficial-llhttp-parser-config.cmake
	DESTINATION share/unofficial-llhttp-parser
)

install(FILES ${LLHTTP_HEADERS} DESTINATION include)
