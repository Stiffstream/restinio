set(SAMPLE sample.async_handling_with_sobjectizer)
add_executable(${SAMPLE} main.cpp)
target_link_libraries(${SAMPLE} nodejs_http_parser)

if(WIN32)
	target_link_libraries(${SAMPLE} wsock32 ws2_32)
endif()

target_link_libraries(${SAMPLE} ${SOBJECTIZER_LIBS})

install(TARGETS ${SAMPLE} DESTINATION bin)
