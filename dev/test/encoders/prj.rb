require 'mxx_ru/cpp'
MxxRu::Cpp::exe_target {

	required_prj 'restinio/platform_specific_libs.rb'

	target( "_unit.test.encoders" )

	cpp_source( "main.cpp" )
}

