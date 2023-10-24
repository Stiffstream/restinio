set(SAMPLE sample.hello_world_sendfile_https)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

target_include_directories(${SAMPLE} PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(${SAMPLE} PRIVATE ${OPENSSL_LIBRARIES})

target_link_libraries(${SAMPLE} PRIVATE restinio_helpers::cmd_line_args)
