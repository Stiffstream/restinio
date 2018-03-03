set(SAMPLE sample.hello_world_sendfile_https)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

target_include_directories(${SAMPLE} PUBLIC ${OPENSSL_INCLUDE_DIR})
target_link_libraries(${SAMPLE} ${OPENSSL_LIBRARIES})

target_link_libraries(${SAMPLE} ${OPENSSL_LIBRARIES})
