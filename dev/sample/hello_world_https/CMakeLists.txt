set(SAMPLE sample.hello_world_https)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

TARGET_INCLUDE_DIRECTORIES(${SAMPLE} PRIVATE ${OPENSSL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${SAMPLE} PRIVATE ${OPENSSL_LIBRARIES})

IF (RESTINIO_INSTALL_SAMPLES)
	# Put pem files in bin dir:
	install(FILES dh2048.pem key.pem server.pem DESTINATION bin)
ENDIF ()
