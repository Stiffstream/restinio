require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

	required_prj 'test/catch_main/prj.rb'

	target( "_unit.test.http_field_parser" )

	cpp_source( "main.cpp" )
}

