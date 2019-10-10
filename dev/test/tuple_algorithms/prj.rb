require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

	required_prj 'test/catch_main/prj.rb'

	target( "_unit.test.tuple_algorithms" )

	cpp_source( "main.cpp" )
}

