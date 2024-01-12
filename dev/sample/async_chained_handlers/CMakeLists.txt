set(SAMPLE sample.async_chained_handlers)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

if (RESTINIO_DEP_SOBJECTIZER STREQUAL "system")
	target_compile_definitions(${SAMPLE} PRIVATE SO_5_STATIC_LIB)
endif()

target_link_libraries(${SAMPLE} PRIVATE ${RESTINIO_SOBJECTIZER_LIB_LINK_NAME})
