add_subdirectory(express)
add_subdirectory(express_router)
add_subdirectory(express_router_bench)

if ( PCRE_FOUND )
	add_subdirectory(express_pcre)
	add_subdirectory(express_router_pcre)
	add_subdirectory(express_router_pcre_bench)
endif ()

