set(UNITTEST _unit.test.socket_options_tls)

include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

find_package(OpenSSL)
target_link_libraries(${UNITTEST} ${OPENSSL_LIBRARIES})

