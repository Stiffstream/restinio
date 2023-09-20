IF (NOT SAMPLE)
	message(FATAL_ERROR "SAMPLE is not defined!")
ENDIF ()

IF(NOT SAMPLE_SRCFILES)
	SET(SAMPLE_SRCFILES main.cpp)
ENDIF()

add_executable(${SAMPLE} ${SAMPLE_SRCFILES})

target_link_libraries(${SAMPLE} PRIVATE restinio::restinio)
target_include_directories(${SAMPLE} PRIVATE ${CMAKE_SOURCE_DIR}/args)

link_threads_if_necessary(${SAMPLE})
# link_atomic_if_necessary(${SAMPLE})

IF (WIN32)
	target_link_libraries(${SAMPLE} PRIVATE wsock32 ws2_32)
ENDIF ()

IF (RESTINIO_INSTALL_SAMPLES)
	install(TARGETS ${SAMPLE} DESTINATION bin)
endif ()
