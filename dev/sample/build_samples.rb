require 'mxx_ru/cpp'
require 'restinio/openssl_find.rb'

MxxRu::Cpp::composite_target {
	required_prj 'sample/hello_world_basic/prj.rb'
	required_prj 'sample/hello_world_minimal/prj.rb'
	required_prj 'sample/hello_world/prj.rb'
	required_prj 'sample/hello_world_sendfile/prj.rb'
	required_prj 'sample/hello_world_delayed/prj.rb'
	required_prj 'sample/async_handling_with_sobjectizer/prj.rb'
	required_prj 'sample/express_router/prj.rb'
	required_prj 'sample/express_router_tutorial/prj.rb'
	required_prj 'sample/sendfiles/prj.rb'
	required_prj 'sample/query_string_params/prj.rb'
	required_prj 'sample/websocket/prj.rb'
	required_prj 'sample/websocket_detailed/prj.rb'
	required_prj 'sample/using_external_io_context/prj.rb'

	if RestinioOpenSSLFind.has_openssl(toolset)
		required_prj 'sample/hello_world_https/prj.rb'
		required_prj 'sample/hello_world_sendfile_https/prj.rb'
		required_prj 'sample/websocket_wss/prj.rb'
	end
}
