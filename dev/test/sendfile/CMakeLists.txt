set(UNITTEST _unit.test.sendfile)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/f1.txt
	${CMAKE_CURRENT_BINARY_DIR}/test/sendfile/f1.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/f2.txt
	${CMAKE_CURRENT_BINARY_DIR}/test/sendfile/f2.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/f3.txt
	${CMAKE_CURRENT_BINARY_DIR}/test/sendfile/f3.txt COPYONLY)
