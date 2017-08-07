require 'mxx_ru/cpp'
MxxRu::Cpp::exe_target {

	required_prj 'asio_mxxru/prj.rb'
	required_prj 'nodejs/http_parser_mxxru/prj.rb'
	required_prj 'fmt_mxxru/prj.rb'
	required_prj 'restinio/platform_specific_libs.rb'

	target( "_unit.test.router.express" )

	cpp_source( "part1.cpp" )
	cpp_source( "part2.cpp" )
	cpp_source( "part3.cpp" )
	cpp_source( "part4.cpp" )
	cpp_source( "part5.cpp" )
	cpp_source( "main.cpp" )
}

