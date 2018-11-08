require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

	required_prj 'fmt_mxxru/prj.rb'
	required_prj 'test/catch_main/prj.rb'

	target( "_unit.test.encoders" )

	cpp_source( "main.cpp" )
}

