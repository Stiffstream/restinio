#!/usr/bin/ruby
require 'mxx_ru/cpp'
require 'restinio/openssl_find.rb'
require 'restinio/pcre_find.rb'
require 'restinio/pcre2_find.rb'
require 'restinio/boost_helper.rb'

MxxRu::Cpp::composite_target {

	required_prj( "test/metaprogramming/prj.ut.rb" )
	required_prj( "test/tuple_algorithms/prj.ut.rb" )
	required_prj( "test/http_field_parser/prj.ut.rb" )
	required_prj( "test/multipart_body/prj.ut.rb" )

	required_prj( "test/header/prj.ut.rb" )
	required_prj( "test/default_constructed_settings/prj.ut.rb" )
	required_prj( "test/ref_qualifiers_settings/prj.ut.rb" )
	required_prj( "test/buffers/prj.ut.rb" )
	required_prj( "test/response_coordinator/prj.ut.rb" )
	required_prj( "test/write_group_output_ctx/prj.ut.rb" )
	required_prj( "test/from_string/prj.ut.rb" )
	required_prj( "test/uri_helpers/prj.ut.rb" )

	if not $sanitizer_build or $sanitizer_build != 'thread_sanitizer'
		required_prj( "test/socket_options/prj.ut.rb" )
	end

	if RestinioOpenSSLFind.has_openssl(toolset)
		if not $sanitizer_build or $sanitizer_build != 'thread_sanitizer'
			required_prj( "test/socket_options_tls/prj.ut.rb" )
		end
	end

	required_prj( "test/start_stop/prj.ut.rb" )

	required_prj( "test/handle_requests/build_tests.rb" )

	required_prj( "test/run_on_thread_pool/prj.rb" )

	required_prj( "test/http_pipelining/sequence/prj.ut.rb" )
	required_prj( "test/http_pipelining/timeouts/prj.ut.rb" )

	required_prj( "test/sendfile/prj.ut.rb" )

	# ================================================================
	# Express router
	required_prj( "test/router/build_tests.rb" )

	# ================================================================
	# Transformators
	required_prj( "test/transforms/zlib/prj.ut.rb" )
	required_prj( "test/transforms/zlib_body_appender/prj.ut.rb" )
	required_prj( "test/transforms/zlib_body_handler/prj.ut.rb" )

	# ================================================================
	required_prj( "test/encoders/prj.ut.rb" )

	# ================================================================
	# Benches for implementation tuning.
	required_prj( "test/to_lower_bench/prj.rb" )

	# ================================================================
	# Websocket tests

	required_prj( "test/handle_requests/upgrade/prj.ut.rb" )
	required_prj( "test/websocket/parser/prj.ut.rb" )
	required_prj( "test/websocket/validators/prj.ut.rb" )
	required_prj( "test/websocket/ws_connection/prj.ut.rb" )
	required_prj( "test/websocket/notificators/prj.ut.rb" )

	# ================================================================
	# File upload support.
	required_prj( "test/file_upload/prj.ut.rb" )
}

