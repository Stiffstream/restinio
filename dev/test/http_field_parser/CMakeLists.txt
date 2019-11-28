set(UNITTEST _unit.test.http_field_parser)
set(UNITTEST_SRCFILES
	basics.cpp
	media-type.cpp
	content-type.cpp
	cache-control.cpp
	content-encoding.cpp
	accept.cpp
	accept-encoding.cpp
	content-disposition.cpp
)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

