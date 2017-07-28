project(sample)

add_subdirectory(hello_world_basic)
add_subdirectory(hello_world)
add_subdirectory(express_router)
add_subdirectory(express_router_tutorial)

if ( RESTINIO_BUILD_TLS_SAMPLES )
	add_subdirectory(hello_world_https)
endif ()
