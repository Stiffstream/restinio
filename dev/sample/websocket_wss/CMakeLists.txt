set(SAMPLE sample.websocket_wss)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

TARGET_INCLUDE_DIRECTORIES(${SAMPLE} PRIVATE ${OPENSSL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${SAMPLE} PRIVATE ${OPENSSL_LIBRARIES})

IF (RESTINIO_INSTALL_SAMPLES)
	# Put pem files in bin dir:
	install(FILES wss_dh2048.pem wss_key.pem wss_server.pem DESTINATION bin)
ENDIF ()
