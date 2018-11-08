set(UNITTEST _unit.test.handle_requests.output_and_buffers)
set(UNITTEST_SRCFILES
	const_buffer.cpp
	std_string.cpp
	shared_ptr_std_string.cpp)

include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)
