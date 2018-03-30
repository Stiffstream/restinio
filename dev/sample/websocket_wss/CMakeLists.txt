set(SAMPLE sample.websocket_wss)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

target_include_directories(${SAMPLE} PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(${SAMPLE} ${OPENSSL_LIBRARIES})

# Put pem files in bin dir:
install(FILES wss_dh2048.pem wss_key.pem wss_server.pem DESTINATION bin)
