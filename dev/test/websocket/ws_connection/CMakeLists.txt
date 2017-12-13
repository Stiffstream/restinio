set(UNITTEST _unit.test.ws_connection)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)
target_link_libraries(${UNITTEST} ${SOBJECTIZER_LIBS})
