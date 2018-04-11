set(UNITTEST _unit.test.router.express_boost_regex)
set(UNITTEST_SRCFILES
	part1.cpp
	part2.cpp
	part3.cpp
	part4.cpp
	part5.cpp
	main.cpp )
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

TARGET_INCLUDE_DIRECTORIES(${UNITTEST} PRIVATE ${Boost_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${UNITTEST} PRIVATE ${Boost_REGEX_LIBRARY})
