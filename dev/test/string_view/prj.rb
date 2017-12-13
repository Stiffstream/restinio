require 'mxx_ru/cpp'
MxxRu::Cpp::exe_target {

	required_prj 'fmt_mxxru/prj.rb'
	required_prj 'restinio/platform_specific_libs.rb'

	target( "_unit.test.string_view" )

	cpp_source( "main.cpp" )
}

