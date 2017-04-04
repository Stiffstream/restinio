set(SAMPLE sample.hello_world)
add_executable(${SAMPLE} main.cpp)
install(TARGETS ${SAMPLE} DESTINATION bin)
