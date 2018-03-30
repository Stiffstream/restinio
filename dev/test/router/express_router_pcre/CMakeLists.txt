set(UNITTEST _unit.test.router.express_router_pcre)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

target_include_directories(${UNITTEST} PRIVATE ${PCRE_INCLUDE_DIRS})
target_link_libraries(${UNITTEST} ${PCRE_LIBRARY})
