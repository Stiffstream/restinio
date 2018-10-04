cmake_minimum_required(VERSION 3.4)
project(http-parser C)

if(BUILD_SHARED_LIBS)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Release")
  add_definitions(-DHTTP_PARSER_STRICT=0)
endif()

if (NOT RESTINIO_HTTP_PARSER_SHARED)
	add_library(http_parser http_parser.c http_parser.h)
else()
	find_package(http_parser 2.7.1 MODULE)
	if (HTTP_PARSER_FOUND)
		add_library(http_parser SHARED IMPORTED GLOBAL)
		set_target_properties(http_parser 
		  PROPERTIES
			IMPORTED_LOCATION ${HTTP_PARSER_LIBRARIES}
			INTERFACE_INCLUDE_DIRECTORIES ${HTTP_PARSER_INCLUDE_DIRS}
		)
	else()
		message(STATUS "Could not find a shared http_parser; will attempt to use embedded version.")
		add_library(http_parser http_parser.c http_parser.h)
		set(RESTINIO_HTTP_PARSER_SHARED OFF)
	endif()
endif()

add_library(unofficial::http_parser::http_parser ALIAS http_parser)

if (NOT RESTINIO_HTTP_PARSER_SHARED)
	target_include_directories(http_parser PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>)

	install(
		TARGETS http_parser
		EXPORT NODEJS_HTTP_PARSER_ALL_TARGETS
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
	)

	install(
		EXPORT NODEJS_HTTP_PARSER_ALL_TARGETS
		NAMESPACE unofficial::http_parser::
		FILE unofficial-http-parser-config.cmake
		DESTINATION share/unofficial-http-parser
	)
	
	install(FILES http_parser.h DESTINATION include)
endif()