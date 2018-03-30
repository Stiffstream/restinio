set(UNITTEST _unit.test.transforms.zlib_body_appender)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

target_include_directories(${UNITTEST} PRIVATE ${ZLIB_INCLUDE_DIRS} )
target_link_libraries(${UNITTEST} ${ZLIB_LIBRARIES})
