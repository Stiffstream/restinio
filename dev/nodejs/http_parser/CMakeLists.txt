cmake_minimum_required (VERSION 3.4)
set(NODEJS_HTTP_PARSER_LIB http_parser)
project (http-parser)

if (BUILD_SHARED_LIBS)
	SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Release")
	add_definitions(-DHTTP_PARSER_STRICT=0)
endif()

add_library(${NODEJS_HTTP_PARSER_LIB} http_parser.c http_parser.h)

IF (NODEJS_HTTP_PARSER_INSTALL)
	install(TARGETS http_parser
		RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
		ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
	)

	if (NOT SKIP_INSTALL_HEADERS)
		install(FILES http_parser.h DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
	endif()
ENDIF ()

IF (NOT RESTINIO_VCPKG_BUILD)
	add_library(nodejs::http_parser ALIAS ${NODEJS_HTTP_PARSER_LIB})
	target_include_directories(${NODEJS_HTTP_PARSER_LIB} PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>)
ENDIF ()

