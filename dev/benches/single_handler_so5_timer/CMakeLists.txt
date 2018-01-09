set(BENCH _bench.restinio.single_handler_so5_timer)
include(${CMAKE_SOURCE_DIR}/cmake/bench.cmake)
target_link_libraries(${BENCH} ${SOBJECTIZER_LIBS})
