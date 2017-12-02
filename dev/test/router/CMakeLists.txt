add_subdirectory(express)
add_subdirectory(express_router)

if ( PCRE_FOUND )
	add_subdirectory(express_pcre)
	add_subdirectory(express_router_pcre)
endif ()

