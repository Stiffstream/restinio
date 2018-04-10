set(UNITTEST _unit.test.router.express_router_pcre2)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

target_include_directories(${UNITTEST} PRIVATE ${PCRE2_INCLUDE_DIRS})
target_link_libraries(${UNITTEST} PRIVATE ${PCRE2_LIBRARIES})
