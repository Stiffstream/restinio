set(UNITTEST _unit.test.http_field_parser)
set(UNITTEST_SRCFILES
	basics.cpp
	media-type.cpp
	content-type.cpp
	cache-control.cpp
	connection.cpp
	content-encoding.cpp
	accept.cpp
	accept-charset.cpp
	accept-encoding.cpp
	accept-language.cpp
	authorization.cpp
	content-disposition.cpp
	range.cpp
	user-agent.cpp
	transfer-encoding.cpp
	host.cpp
)
include(${CMAKE_SOURCE_DIR}/cmake/unittest.cmake)

