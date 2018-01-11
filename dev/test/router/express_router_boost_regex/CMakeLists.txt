set(UNITTEST _unit.test.router.express_router_boost_regex)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

target_link_libraries(${UNITTEST} ${Boost_REGEX_LIBRARY})
