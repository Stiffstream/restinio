cmake_minimum_required(VERSION 3.3)

set(RESTINIO restinio)

project(${RESTINIO}) # not to misspell it.

add_library(${RESTINIO} INTERFACE)
add_library(restinio::restinio ALIAS ${RESTINIO})

TARGET_LINK_LIBRARIES(${RESTINIO} INTERFACE fmt::fmt-header-only)
TARGET_LINK_LIBRARIES(${RESTINIO} INTERFACE nodejs::http_parser)

set(RESTINIO_HEADERS_ALL
	all.hpp
	asio_include.hpp
	asio_timer_manager.hpp
	buffers.hpp
	cast_to.hpp
	common_types.hpp
	exception.hpp
	http_headers.hpp
	http_server.hpp
	http_server_run.hpp
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
	impl/acceptor.hpp
	impl/connection_base.hpp
	impl/connection.hpp
	impl/connection_settings.hpp
	impl/executor_wrapper.hpp
	impl/fixed_buffer.hpp
	impl/header_helpers.hpp
	impl/ioctx_on_thread_pool.hpp
	impl/os_posix.inl
	impl/os_unknown.inl
	impl/os_win.inl
	impl/parser_callbacks.inl
	impl/raw_resp_output_ctx.hpp
	impl/response_coordinator.hpp
	impl/sendfile_operation_default.inl
	impl/sendfile_operation.hpp
	impl/sendfile_operation_posix.inl
	impl/sendfile_operation_win.inl
	impl/tls_socket.hpp
	impl/to_lower_lut.inl

	path2regex/path2regex.hpp

	router/boost_regex_engine.hpp
	router/express.hpp
	router/pcre2_regex_engine.hpp
	router/pcre_regex_engine.hpp
	router/std_regex_engine.hpp

	so5/so_timer_manager.hpp

	third_party/optional-lite/optional.hpp
	third_party/string-view-lite/string_view.hpp

	transforms/zlib.hpp

	utils/base64.hpp
	utils/base64_lut.inl
	utils/from_string_details.inl
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

foreach(HEADER_FILE ${RESTINIO_HEADERS_ALL} )
	get_filename_component( DIR ${HEADER_FILE} PATH )
	install( FILES ${HEADER_FILE} DESTINATION include/restinio/${DIR} )
endforeach()
