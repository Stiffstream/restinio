set(UNITTEST _unit.test.sendfile)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/f1.dat
	${CMAKE_CURRENT_BINARY_DIR}/test/sendfile/f1.dat COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/f2.dat
	${CMAKE_CURRENT_BINARY_DIR}/test/sendfile/f2.dat COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/f3.dat
	${CMAKE_CURRENT_BINARY_DIR}/test/sendfile/f3.dat COPYONLY)
