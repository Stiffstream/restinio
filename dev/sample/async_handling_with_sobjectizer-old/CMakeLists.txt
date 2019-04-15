set(SAMPLE sample.async_handling_with_sobjectizer)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

TARGET_LINK_LIBRARIES(${SAMPLE} PRIVATE ${SOBJECTIZER_LIBS})
