set(UNITTEST _unit.test.socket_options_tls)

include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

target_include_directories(${UNITTEST} PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(${UNITTEST} ${OPENSSL_LIBRARIES})
