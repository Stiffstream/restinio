require 'mxx_ru/cpp'
require 'restinio/asio_helper.rb'

MxxRu::Cpp::exe_target {

	RestinioAsioHelper.attach_propper_asio( self )

	required_prj 'nodejs/http_parser_mxxru/prj.rb'
	required_prj 'fmt_mxxru/prj.rb'
	required_prj 'restinio/platform_specific_libs.rb'
	required_prj 'test/catch_main/prj.rb'

	required_prj 'restinio/pcre2_libs.rb'

	target( "_unit.test.router.express_router_pcre2" )

	cpp_source( "main.cpp" )
}

