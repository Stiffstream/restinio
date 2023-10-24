set(SAMPLE sample.file_upload)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

target_link_libraries(${SAMPLE} PRIVATE restinio_helpers::cmd_line_args)
