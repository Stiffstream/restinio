set(SAMPLE sample.decompression)
include(${CMAKE_SOURCE_DIR}/cmake/sample.cmake)

target_include_directories(${SAMPLE} PRIVATE ${ZLIB_INCLUDE_DIRS} )
target_link_libraries(${SAMPLE} PRIVATE ${ZLIB_LIBRARIES})

IF (RESTINIO_INSTALL_SAMPLES)
	install(FILES test_decompression.sh DESTINATION bin)
ENDIF ()

target_link_libraries(${SAMPLE} PRIVATE restinio_helpers::cmd_line_args)

