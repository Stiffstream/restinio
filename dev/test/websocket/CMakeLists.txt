add_subdirectory(parser)
add_subdirectory(validators)

if ( RESTINIO_ALLOW_SOBJECTIZER )
	add_subdirectory(ws_connection)
endif()

