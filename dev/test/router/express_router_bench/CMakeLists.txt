set(TEST_BENCH _test.router.express_router_bench)
add_executable(${TEST_BENCH} main.cpp)
target_link_libraries(${TEST_BENCH} nodejs_http_parser)

if(WIN32)
	target_link_libraries(${TEST_BENCH} wsock32 ws2_32)
endif()

install(TARGETS ${TEST_BENCH} DESTINATION bin)
