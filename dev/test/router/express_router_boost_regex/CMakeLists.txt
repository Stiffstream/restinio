set(UNITTEST _unit.test.router.express_router_boost_regex)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

TARGET_LINK_LIBRARIES(${UNITTEST} PRIVATE ${Boost_REGEX_LIBRARY})
