set(TEST_BENCH _test.router.easy_parser_router_bench)
set(TEST_BENCH_SRCFILES main.cpp)

include(${CMAKE_SOURCE_DIR}/cmake/testbench.cmake)

target_link_libraries(${TEST_BENCH} PRIVATE restinio_helpers::cmd_line_args)
