set(SAMPLE sample.compression)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

target_include_directories(${SAMPLE} PRIVATE ${ZLIB_INCLUDE_DIRS} )
target_link_libraries(${SAMPLE} ${ZLIB_LIBRARIES})
