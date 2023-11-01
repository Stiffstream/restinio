set(SAMPLE sample.async_chained_handlers)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

target_link_libraries(${SAMPLE} PRIVATE sobjectizer::StaticLib)
