project(bench)

add_subdirectory(single_handler)
add_subdirectory(single_handler_no_timer)

if ( RESTINIO_WITH_SOBJECTIZER )
	add_subdirectory(single_handler_so5_timer)
endif()

