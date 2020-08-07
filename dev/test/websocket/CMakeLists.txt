add_subdirectory(parser)
add_subdirectory(validators)

if ( RESTINIO_SOBJECTIZER_ENABLED )
	add_subdirectory(ws_connection)
endif()

