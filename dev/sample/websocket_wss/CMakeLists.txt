set(SAMPLE sample.websocket_wss)

add_executable(${SAMPLE} main.cpp)
target_link_libraries(${SAMPLE} nodejs_http_parser)

target_link_libraries(${SAMPLE} ${OPENSSL_LIBRARIES})

if(WIN32)
	target_link_libraries(${SAMPLE} wsock32 ws2_32)
endif()

install(TARGETS ${SAMPLE} DESTINATION bin)

# Put pem files in bin dir:
install(FILES wss_dh2048.pem wss_key.pem wss_server.pem DESTINATION bin)
