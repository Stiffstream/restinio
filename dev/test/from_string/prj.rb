require 'mxx_ru/cpp'
MxxRu::Cpp::exe_target {

	required_prj 'fmt_mxxru/prj.rb'

	target( "_unit.test.from_string" )

	cpp_source( "main.cpp" )
}

