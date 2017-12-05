add_subdirectory(express)
add_subdirectory(express_router)
add_subdirectory(express_router_bench)

if ( PCRE_FOUND )
	add_subdirectory(express_pcre)
	add_subdirectory(express_router_pcre)
	add_subdirectory(express_router_pcre_bench)
endif ()

if ( PCRE2_FOUND )
	add_subdirectory(express_pcre2)
	add_subdirectory(express_router_pcre2)
	add_subdirectory(express_router_pcre2_bench)
endif ()

