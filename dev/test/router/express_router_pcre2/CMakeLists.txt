set(UNITTEST _unit.test.router.express_router_pcre2)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

TARGET_COMPILE_DEFINITIONS(${UNITTEST} PRIVATE -DPCRE2_STATIC -DPCRE2_CODE_UNIT_WIDTH=8)

target_include_directories(${UNITTEST} PRIVATE ${PCRE2_INCLUDE_DIRS})
target_link_libraries(${UNITTEST} PRIVATE ${PCRE2_LIBRARIES})
