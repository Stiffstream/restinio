set(BENCH _bench.restinio.single_handler_so5_timer)
include(${CMAKE_SOURCE_DIR}/cmake/bench.cmake)

if (RESTINIO_DEP_SOBJECTIZER STREQUAL "system")
	target_compile_definitions(${BENCH} PRIVATE SO_5_STATIC_LIB)
endif()

target_link_libraries(${BENCH} PRIVATE ${RESTINIO_SOBJECTIZER_LIB_LINK_NAME})

target_link_libraries(${BENCH} PRIVATE restinio_helpers::cmd_line_args)

