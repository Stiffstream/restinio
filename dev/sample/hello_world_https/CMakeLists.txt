set(SAMPLE sample.hello_world_https)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

target_include_directories(${SAMPLE} PUBLIC ${OPENSSL_INCLUDE_DIR})
target_link_libraries(${SAMPLE} ${OPENSSL_LIBRARIES})


# Put pem files in bin dir:
install(FILES dh2048.pem key.pem server.pem DESTINATION bin)
