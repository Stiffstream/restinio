set(SAMPLE sample.hello_world_sendfile_w32_unicode)

include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

TARGET_COMPILE_DEFINITIONS(${SAMPLE} PRIVATE "_UNICODE" "UNICODE")
