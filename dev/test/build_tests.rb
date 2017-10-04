#!/usr/bin/ruby
require 'mxx_ru/cpp'
require 'restinio/openssl_find.rb'

MxxRu::Cpp::composite_target {

	required_prj( "test/header/prj.ut.rb" )
	required_prj( "test/default_constructed_settings/prj.ut.rb" )
	required_prj( "test/ref_qualifiers_settings/prj.ut.rb" )
	required_prj( "test/buffers/prj.ut.rb" )
	required_prj( "test/response_coordinator/prj.ut.rb" )
	required_prj( "test/uri_helpers/prj.ut.rb" )
	required_prj( "test/socket_options/prj.ut.rb" )

	if RestinioOpenSSLFind.has_openssl(toolset)
		required_prj( "test/socket_options_tls/prj.ut.rb" )
	end

	required_prj( "test/close/prj.ut.rb" )

	required_prj( "test/handle_requests/method/prj.ut.rb" )
	required_prj( "test/handle_requests/echo_body/prj.ut.rb" )
	required_prj( "test/handle_requests/timeouts/prj.ut.rb" )
	required_prj( "test/handle_requests/slow_transmit/prj.ut.rb" )
	required_prj( "test/handle_requests/throw_exception/prj.ut.rb" )
	required_prj( "test/handle_requests/user_controlled_output/prj.ut.rb" )
	required_prj( "test/handle_requests/chunked_output/prj.ut.rb" )
	required_prj( "test/handle_requests/output_and_buffers/prj.ut.rb" )

	required_prj( "test/http_pipelining/sequence/prj.ut.rb" )
	required_prj( "test/http_pipelining/timeouts/prj.ut.rb" )

	required_prj( "test/router/express/prj.ut.rb" )
	required_prj( "test/router/express_router/prj.ut.rb" )

	required_prj( "test/encoders/prj.ut.rb" )

	# ================================================================
	# Benches for implementation tuning.
	required_prj( "test/to_lower_bench/prj.rb" )
	# ================================================================
	# Websocket tests

	required_prj( "test/handle_requests/upgrade/prj.ut.rb" )
	required_prj( "test/websocket/parser/prj.ut.rb" )
	required_prj( "test/websocket/ws_connection/prj.ut.rb" )
}
