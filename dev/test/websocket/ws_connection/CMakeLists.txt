set(UNITTEST _unit.test.ws_connection)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

TARGET_LINK_LIBRARIES(${UNITTEST} PRIVATE ${SOBJECTIZER_LIBS})
