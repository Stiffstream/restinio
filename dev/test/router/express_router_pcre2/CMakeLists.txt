set(UNITTEST _unit.test.router.express_router_pcre2)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

target_link_libraries(${UNITTEST} ${PCRE2_LIBRARY})
