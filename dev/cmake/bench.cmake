if(NOT BENCH)
	message(FATAL_ERROR "BENCH is not defined!")
endif()

if(NOT BENCH_SRCFILES)
	set(BENCH_SRCFILES main.cpp)
endif()

add_executable(${BENCH} ${BENCH_SRCFILES})

TARGET_LINK_LIBRARIES(${BENCH} PRIVATE restinio::restinio)

if(WIN32)
	TARGET_LINK_LIBRARIES(${BENCH} wsock32 ws2_32)
endif()

IF (RESTINIO_INSTALL_BENCHES)
	install(TARGETS ${BENCH} DESTINATION bin)
ENDIF()
