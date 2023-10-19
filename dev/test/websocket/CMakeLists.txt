add_subdirectory(parser)
add_subdirectory(validators)

if ( RESTINIO_WITH_SOBJECTIZER )
	add_subdirectory(ws_connection)
	add_subdirectory(notificators)
endif()

