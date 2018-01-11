if(NOT BENCH)
	message(FATAL_ERROR "BENCH is not defined!")
endif()

if(NOT BENCH_SRCFILES)
	set(BENCH_SRCFILES main.cpp)
endif()

add_executable(${BENCH} ${BENCH_SRCFILES})

target_link_libraries(${BENCH} nodejs_http_parser)

if(WIN32)
	target_link_libraries(${BENCH} wsock32 ws2_32)
endif()

if( NOT (RESTINIO_USE_BOOST_ASIO STREQUAL "none") )
	target_link_libraries(${BENCH} ${Boost_SYSTEM_LIBRARY} )
endif()

install(TARGETS ${BENCH} DESTINATION bin)
