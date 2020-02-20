require 'mxx_ru/cpp'
require 'restinio/openssl_find.rb'

MxxRu::Cpp::composite_target {
	required_prj 'sample/hello_world_basic/prj.rb'
	required_prj 'sample/hello_world_minimal/prj.rb'
	required_prj 'sample/hello_world/prj.rb'
	required_prj 'sample/hello_world_sendfile/prj.rb'
	required_prj 'sample/hello_world_delayed/prj.rb'
	required_prj 'sample/run_existing_server/prj.rb'
	required_prj 'sample/run_for_minute/prj.rb'
	required_prj 'sample/express_router/prj.rb'
	required_prj 'sample/express_router_tutorial/prj.rb'
	required_prj 'sample/sendfiles/prj.rb'
	required_prj 'sample/query_string_params/prj.rb'
	required_prj 'sample/try_parse_query_string/prj.rb'
	required_prj 'sample/using_external_io_context/prj.rb'
	required_prj 'sample/async_handling_with_sobjectizer/prj.rb'
	required_prj 'sample/compression/prj.rb'
	required_prj 'sample/decompression/prj.rb'
	required_prj 'sample/notificators/prj.rb'
	required_prj 'sample/custom_buffer/prj.rb'
	required_prj 'sample/connection_state/prj.rb'
	required_prj 'sample/ip_blocker/prj.rb'
	required_prj 'sample/file_upload/prj.rb'

	required_prj 'sample/websocket/prj.rb'
	required_prj 'sample/websocket_detailed/prj.rb'

	if "mswin" == toolset.tag( "target_os" )
		required_prj 'sample/hello_world_sendfile_w32_unicode/prj.rb'
	end

	if RestinioOpenSSLFind.has_openssl(toolset)
		required_prj 'sample/hello_world_https/prj.rb'
		required_prj 'sample/hello_world_sendfile_https/prj.rb'
		required_prj 'sample/websocket_wss/prj.rb'
      required_prj 'sample/tls_inspector/prj.rb'
	end
}

