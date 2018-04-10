set(BENCH _bench.restinio.single_handler_so5_timer)
include(${CMAKE_SOURCE_DIR}/cmake/bench.cmake)

TARGET_LINK_LIBRARIES(${BENCH} PRIVATE ${SOBJECTIZER_LIBS})
