require 'mxx_ru/cpp'
MxxRu::Cpp::exe_target {

	required_prj 'asio_mxxru/prj.rb'
	required_prj 'nodejs/http_parser_mxxru/prj.rb'
	required_prj 'fmt_mxxru/prj.rb'
	required_prj 'so_5/prj.rb'
	required_prj 'restinio/platform_specific_libs.rb'

	target( "_unit.test.websocket.ws_connection" )

	cpp_source( "main.cpp" )
}

