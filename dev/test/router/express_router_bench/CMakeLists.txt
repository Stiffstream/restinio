set(TEST_BENCH _test.router.express_router_bench)
include(${CMAKE_SOURCE_DIR}/cmake/testbench.cmake)

target_link_libraries(${TEST_BENCH} PRIVATE restinio_helpers::cmd_line_args)

