set(BENCH _bench.restinio.single_handler_no_timer)
add_executable(${BENCH} main.cpp)
target_link_libraries(${BENCH} nodejs_http_parser)

if(WIN32)
	target_link_libraries(${BENCH} wsock32 ws2_32)
endif()

if( NOT (RESTINIO_USE_BOOST_ASIO STREQUAL "none") )
	target_link_libraries(${BENCH} ${Boost_LIBRARIES} )
endif()

install(TARGETS ${BENCH} DESTINATION bin)
