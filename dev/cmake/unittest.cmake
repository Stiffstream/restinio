if(NOT UNITTEST)
    message(FATAL_ERROR "UNITTEST is not defined!")
endif()

if(NOT UNITTEST_SRCFILES)
    set(UNITTEST_SRCFILES main.cpp)
endif()

add_executable(${UNITTEST} ${UNITTEST_SRCFILES})

target_link_libraries(${UNITTEST} restinio::restinio)

if(WIN32)
	target_link_libraries(${UNITTEST} wsock32 ws2_32)
endif()

if( NOT (RESTINIO_USE_BOOST_ASIO STREQUAL "none") )
	target_link_libraries(${UNITTEST} ${Boost_SYSTEM_LIBRARY} )
endif()

add_test(NAME ${UNITTEST} COMMAND ${SO_5_TEST_LAUNCHER} ${UNITTEST})
# add_test(NAME ${UNITTEST} COMMAND ${SO_5_TEST_LAUNCHER} ${UNITTEST})
