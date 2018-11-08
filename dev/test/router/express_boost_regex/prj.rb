require 'mxx_ru/cpp'
require 'restinio/asio_helper.rb'

MxxRu::Cpp::exe_target {

	RestinioAsioHelper.attach_propper_asio( self )

	required_prj 'nodejs/http_parser_mxxru/prj.rb'
	required_prj 'fmt_mxxru/prj.rb'
	required_prj 'restinio/platform_specific_libs.rb'
	required_prj 'test/catch_main/prj.rb'

	lib_static( RestinioBoostHelper.get_lib_name( 'boost_regex', self ) )

	target( "_unit.test.router.express_boost_regex" )

	cpp_source( "part1.cpp" )
	cpp_source( "part2.cpp" )
	cpp_source( "part3.cpp" )
	cpp_source( "part4.cpp" )
	cpp_source( "part5.cpp" )
	cpp_source( "main.cpp" )
}

