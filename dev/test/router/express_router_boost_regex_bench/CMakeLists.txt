set(TEST_BENCH _test.router.express_router_boost_regex_bench)
include(${CMAKE_SOURCE_DIR}/cmake/testbench.cmake)

TARGET_LINK_LIBRARIES(${TEST_BENCH} PRIVATE Boost::regex)
