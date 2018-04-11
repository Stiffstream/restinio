project(nodejs_http_parser)

add_library(nodejs_http_parser http_parser.c)
add_library(nodejs::http_parser ALIAS nodejs_http_parser)

TARGET_INCLUDE_DIRECTORIES(nodejs_http_parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_COMPILE_DEFINITIONS(nodejs_http_parser INTERFACE HTTP_PARSER_STRICT=0)

IF (NODEJS_HTTP_PARSER_INSTALL)
	include(GNUInstallDirs)

	set(NODEJS_HTTP_PARSER_LIB_DIR ${CMAKE_INSTALL_LIBDIR} CACHE STRING
		"Installation directory for libraries, relative to ${CMAKE_INSTALL_PREFIX}.")

	set(NODEJS_HTTP_PARSER_INC_DIR ${CMAKE_INSTALL_INCLUDEDIR}/nodejs/http_parser/ CACHE STRING
		"Installation directory for include files, relative to ${CMAKE_INSTALL_PREFIX}.")

	install(
		TARGETS nodejs_http_parser
		EXPORT NODEJS_HTTP_PARSER_TARGET
		DESTINATION ${NODEJS_HTTP_PARSER_LIB_DIR} )

	install( FILES http_parser.h DESTINATION ${NODEJS_HTTP_PARSER_INC_DIR} )
ENDIF ()
