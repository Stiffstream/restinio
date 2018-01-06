set(SAMPLE sample.async_handling_with_sobjectizer)
add_executable(${SAMPLE} main.cpp)
target_link_libraries(${SAMPLE} nodejs_http_parser)

if(WIN32)
	target_link_libraries(${SAMPLE} wsock32 ws2_32)
endif()

target_link_libraries(${SAMPLE} ${SOBJECTIZER_LIBS})

if( NOT (RESTINIO_USE_BOOST_ASIO STREQUAL "none") )
	target_link_libraries(${SAMPLE} ${Boost_LIBRARIES} )
endif()

install(TARGETS ${SAMPLE} DESTINATION bin)
