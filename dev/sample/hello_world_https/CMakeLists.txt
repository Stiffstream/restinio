set(SAMPLE sample.hello_world_https)

find_package(OpenSSL)

add_executable(${SAMPLE} main.cpp)
target_link_libraries(${SAMPLE} nodejs_http_parser)

target_link_libraries(${SAMPLE} ${OPENSSL_LIBRARIES})

if(WIN32)
	target_link_libraries(${SAMPLE} wsock32 ws2_32)
endif()

# if (${MSVC})
# else ()
# 	target_link_libraries(${SAMPLE} ssl crypto)
# 	if( MINGW )
# 		target_link_libraries(${SAMPLE} gdi32)
# 	endif()
# endif ()

install(TARGETS ${SAMPLE} DESTINATION bin)

# Put pem files in bin dir:
install(FILES dh2048.pem key.pem server.pem DESTINATION bin)
