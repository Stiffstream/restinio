set(SAMPLE sample.compression)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

target_link_libraries(${UNITTEST} ${ZLIB_LIBRARIES})
