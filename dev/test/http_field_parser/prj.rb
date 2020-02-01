require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

	required_prj 'test/catch_main/prj.rb'

	target( "_unit.test.http_field_parser" )

	cpp_source( "basics.cpp" )
	cpp_source( "media-type.cpp" )
	cpp_source( "content-type.cpp" )
	cpp_source( "cache-control.cpp" )
	cpp_source( "content-encoding.cpp" )
	cpp_source( "accept.cpp" )
	cpp_source( "accept-charset.cpp" )
	cpp_source( "accept-encoding.cpp" )
	cpp_source( "accept-language.cpp" )
	cpp_source( "content-disposition.cpp" )
	cpp_source( "range.cpp" )
	cpp_source( "user-agent.cpp" )
}

