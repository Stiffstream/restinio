set(SAMPLE sample.notificators)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

TARGET_LINK_LIBRARIES(${SAMPLE} PRIVATE ${SOBJECTIZER_LIBS})
