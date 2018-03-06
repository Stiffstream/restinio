set(TEST_BENCH _test.router.express_router_pcre_bench)
add_executable(${TEST_BENCH} main.cpp)
target_link_libraries(${TEST_BENCH} nodejs_http_parser ${PCRE_LIBRARY})

if(WIN32)
	target_link_libraries(${TEST_BENCH} wsock32 ws2_32)
endif()

if( NOT (RESTINIO_USE_BOOST_ASIO STREQUAL "none") )
	target_link_libraries(${TEST_BENCH} ${Boost_LIBRARIES} )
endif()

install(TARGETS ${TEST_BENCH} DESTINATION bin)
