set(SAMPLE sample.websocket_wss)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)
# Put pem files in bin dir:
install(FILES wss_dh2048.pem wss_key.pem wss_server.pem DESTINATION bin)
