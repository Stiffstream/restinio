set(SAMPLE sample.hello_world_sendfile_w32_unicode)

include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

target_compile_definitions(${SAMPLE} PRIVATE "_UNICODE" "UNICODE")

target_link_libraries(${SAMPLE} PRIVATE restinio_helpers::cmd_line_args)

