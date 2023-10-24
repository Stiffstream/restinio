set(BENCH _bench.restinio.single_handler_no_timer)
include(${CMAKE_SOURCE_DIR}/cmake/bench.cmake)

target_link_libraries(${BENCH} PRIVATE restinio_helpers::cmd_line_args)

