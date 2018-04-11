set(TEST_BENCH _test.router.express_router_boost_regex_bench)
include(${CMAKE_SOURCE_DIR}/cmake/testbench.cmake)

TARGET_INCLUDE_DIRECTORIES(${TEST_BENCH} PRIVATE ${Boost_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${TEST_BENCH} PRIVATE ${Boost_REGEX_LIBRARY})
