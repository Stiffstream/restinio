add_subdirectory(method)
add_subdirectory(echo_body)
add_subdirectory(timeouts)
add_subdirectory(throw_exception)
add_subdirectory(slow_transmit)
add_subdirectory(user_controlled_output)
add_subdirectory(chunked_output)
add_subdirectory(output_and_buffers)
add_subdirectory(notificators)
add_subdirectory(remote_endpoint)
add_subdirectory(connection_state)
add_subdirectory(ip_blocker)

add_subdirectory(upgrade)

add_subdirectory(chunked_input)

add_subdirectory(acceptor_post_bind_hook)

add_subdirectory(incoming_msg_limits)

add_subdirectory(connection_count_limit)

add_subdirectory(user_data_simple)

add_subdirectory(sync_chained_handlers)
add_subdirectory(async_chained_handlers_1)

if( RESTINIO_WITH_SOBJECTIZER )
	add_subdirectory(async_chained_handlers_2)
endif()

