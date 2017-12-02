project(sample)

add_subdirectory(hello_world_basic)
add_subdirectory(hello_world)
add_subdirectory(express_router)
add_subdirectory(express_router_tutorial)
add_subdirectory(websocket)
add_subdirectory(websocket_detailed)
add_subdirectory(using_external_io_context)

if ( OPENSSL_FOUND )
	message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
	message("OpenSSL libraries: ${OPENSSL_LIBRARIES}")

	add_subdirectory(hello_world_https)
	add_subdirectory(websocket_wss)
endif ()
