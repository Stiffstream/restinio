set(UNITTEST _unit.test.ws_connection)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

if (RESTINIO_DEP_SOBJECTIZER STREQUAL "system")
	target_compile_definitions(${UNITTEST} PRIVATE SO_5_STATIC_LIB)
endif()

target_link_libraries(${UNITTEST} PRIVATE ${RESTINIO_SOBJECTIZER_LIB_LINK_NAME})
