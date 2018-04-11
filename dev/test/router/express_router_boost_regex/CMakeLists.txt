set(UNITTEST _unit.test.router.express_router_boost_regex)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

TARGET_INCLUDE_DIRECTORIES(${UNITTEST} PRIVATE ${Boost_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${UNITTEST} PRIVATE ${Boost_REGEX_LIBRARY})
