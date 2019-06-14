cmake_minimum_required(VERSION 3.3)

SET(RESTINIO restinio)

include(cmake/target.cmake)

project(${RESTINIO}) # not to misspell it.

add_library(${RESTINIO} INTERFACE)
add_library(restinio::restinio ALIAS ${RESTINIO})

IF (${CMAKE_VERSION} VERSION_LESS "3.8.0")
	set_target_properties(${RESTINIO} PROPERTIES
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
	)
ELSE ()
	target_compile_features(${RESTINIO} INTERFACE cxx_std_14)
ENDIF ()

get_filename_component(CURRENT_FILE_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
get_filename_component(CURRENT_FILE_DIR ${CURRENT_FILE_DIR} DIRECTORY)
SET(RESTINIO_INCLUDE_PATH ${CURRENT_FILE_DIR})
UNSET(CURRENT_FILE_DIR)

# Propagate include directories for RESTinio.
TARGET_INCLUDE_DIRECTORIES(${RESTINIO}
	INTERFACE
		$<BUILD_INTERFACE:${RESTINIO_INCLUDE_PATH}>
		$<INSTALL_INTERFACE:include>
	)

# ------------------------------------------------------------------------------
# ASIO or Boost::ASIO
IF ( RESTINIO_USE_BOOST_ASIO MATCHES "none" )
	IF (RESTINIO_STAND_ALONE_ASIO_HEADERS)
		TARGET_INCLUDE_DIRECTORIES(${RESTINIO} INTERFACE
			$<BUILD_INTERFACE:${RESTINIO_STAND_ALONE_ASIO_HEADERS}>)
	ENDIF ()

	IF( RESTINIO_STAND_ALONE_ASIO_DEFINES )
		TARGET_COMPILE_DEFINITIONS(${RESTINIO} INTERFACE ${RESTINIO_STAND_ALONE_ASIO_DEFINES})
	ENDIF ()

ELSE ()
	TARGET_INCLUDE_DIRECTORIES(${RESTINIO} INTERFACE ${Boost_INCLUDE_DIRS} )
	TARGET_COMPILE_DEFINITIONS(${RESTINIO} INTERFACE -DRESTINIO_USE_BOOST_ASIO )
ENDIF ()
# ------------------------------------------------------------------------------

IF (RESTINIO_FIND_DEPS)
	message(STATUS "RESTinio dependencies are included with `find_package()`")

	TARGET_LINK_LIBRARIES(${RESTINIO} INTERFACE fmt::fmt-header-only)
	TARGET_LINK_LIBRARIES(${RESTINIO} INTERFACE unofficial::http_parser::http_parser)
ELSE ()
	TARGET_LINK_LIBRARIES(${RESTINIO} INTERFACE $<BUILD_INTERFACE:fmt::fmt-header-only>)
	TARGET_LINK_LIBRARIES(${RESTINIO} INTERFACE $<BUILD_INTERFACE:unofficial::http_parser::http_parser>)
ENDIF ()

IF ( NOT (RESTINIO_USE_BOOST_ASIO STREQUAL "none") )
	TARGET_LINK_LIBRARIES(${RESTINIO} INTERFACE ${Boost_SYSTEM_LIBRARY} )
ENDIF ()

SET(RESTINIO_HEADERS_ALL
	all.hpp
	asio_include.hpp
	asio_timer_manager.hpp
	buffers.hpp
	cast_to.hpp
	common_types.hpp
	connection_state_listener.hpp
	exception.hpp
	http_headers.hpp
	http_server.hpp
	http_server_run.hpp
	ip_blocker.hpp
	message_builders.hpp
	null_logger.hpp
	null_timer_manager.hpp
	optional.hpp
	os.hpp
	ostream_logger.hpp
	request_handler.hpp
	sendfile_defs_default.hpp
	sendfile_defs_posix.hpp
	sendfile_defs_win.hpp
	sendfile.hpp
	settings.hpp
	string_view.hpp
	tcp_connection_ctx_base.hpp
	timer_common.hpp
	tls.hpp
	traits.hpp
	uri_helpers.hpp
	value_or.hpp
	variant.hpp
	impl/acceptor.hpp
	impl/connection_base.hpp
	impl/connection.hpp
	impl/connection_settings.hpp
	impl/executor_wrapper.hpp
	impl/fixed_buffer.hpp
	impl/header_helpers.hpp
	impl/ioctx_on_thread_pool.hpp
	impl/os_posix.ipp
	impl/os_unknown.ipp
	impl/os_win.ipp
	impl/parser_callbacks.ipp
	impl/write_group_output_ctx.hpp
	impl/response_coordinator.hpp
	impl/sendfile_operation_default.ipp
	impl/sendfile_operation.hpp
	impl/sendfile_operation_posix.ipp
	impl/sendfile_operation_win.ipp
	impl/tls_socket.hpp
	impl/to_lower_lut.ipp

	path2regex/path2regex.hpp

	router/boost_regex_engine.hpp
	router/express.hpp
	router/pcre2_regex_engine.hpp
	router/pcre_regex_engine.hpp
	router/std_regex_engine.hpp

	so5/so_timer_manager.hpp

	third_party/optional-lite/optional.hpp
	third_party/string-view-lite/string_view.hpp
	third_party/variant-lite/variant.hpp

	transforms/zlib.hpp

	utils/base64.hpp
	utils/base64_lut.ipp
	utils/from_string_details.ipp
	utils/from_string.hpp
	utils/percent_encoding.hpp
	utils/sha1.hpp

	utils/impl/bitops.hpp
	utils/impl/safe_uint_truncate.hpp

	websocket/message.hpp
	websocket/websocket.hpp

	websocket/impl/utf8.hpp
	websocket/impl/ws_connection_base.hpp
	websocket/impl/ws_connection.hpp
	websocket/impl/ws_parser.hpp
	websocket/impl/ws_protocol_validator.hpp
	)

IF (RESTINIO_INSTALL)
	include(GNUInstallDirs)
	include(CMakePackageConfigHelpers)

	SET(RESTINIO_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/restinio CACHE STRING
		"Installation directory for cmake files, relative to ${CMAKE_INSTALL_PREFIX}.")
	SET(RESTINIO_VERSION_CONFIG ${PROJECT_BINARY_DIR}/restinio-config-version.cmake)
	SET(RESTINIO_PROJECT_CONFIG ${PROJECT_BINARY_DIR}/restinio-config.cmake)

	SET(RESTINIO_INC_DIR ${CMAKE_INSTALL_INCLUDEDIR}/restinio/ CACHE STRING
		"Installation directory for include files, relative to ${CMAKE_INSTALL_PREFIX}.")

	# Generate the version, config and target files into the build directory.
	write_basic_package_version_file(
		${RESTINIO_VERSION_CONFIG}
		VERSION ${RESTINIO_VERSION}
		COMPATIBILITY ExactVersion )

	configure_package_config_file(
		"cmake/restinio-config.cmake.in"
		${RESTINIO_PROJECT_CONFIG}
		INSTALL_DESTINATION ${RESTINIO_CMAKE_DIR}
		PATH_VARS RESTINIO_VERSION)

	# Install version, config and target files.
	install(
		FILES ${RESTINIO_PROJECT_CONFIG} ${RESTINIO_VERSION_CONFIG}
		DESTINATION ${RESTINIO_CMAKE_DIR})

	SET(RESTINIO_INSTALL_TARGETS ${RESTINIO})

	install(
		TARGETS ${RESTINIO_INSTALL_TARGETS}
		EXPORT RESTINIO_ALL_TARGETS
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
	)

	install(
		EXPORT RESTINIO_ALL_TARGETS
		NAMESPACE restinio::
		FILE restinio-targets.cmake
		DESTINATION ${RESTINIO_CMAKE_DIR} )

	FOREACH (HEADER_FILE ${RESTINIO_HEADERS_ALL} )
		get_filename_component( DIR ${HEADER_FILE} PATH )
		install( FILES ${HEADER_FILE} DESTINATION ${RESTINIO_INC_DIR}/${DIR} )
	ENDFOREACH ()

ENDIF ()
