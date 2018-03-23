set(UNITTEST _unit.test.transforms.zlib_body_handler)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

target_link_libraries(${UNITTEST} ${ZLIB_LIBRARIES})
