set(SAMPLE sample.using_external_io_context)
add_executable(${SAMPLE} main.cpp)
target_link_libraries(${SAMPLE} nodejs_http_parser)

if(WIN32)
	target_link_libraries(${SAMPLE} wsock32 ws2_32)
endif()

if( NOT (RESTINIO_USE_BOOST_ASIO STREQUAL "none") )
	target_link_libraries(${SAMPLE} ${Boost_LIBRARIES} )
endif()

install(TARGETS ${SAMPLE} DESTINATION bin)

