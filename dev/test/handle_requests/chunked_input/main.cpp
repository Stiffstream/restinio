/*
	restinio
*/

#include <catch2/catch_all.hpp>

#include <fmt/ranges.h>

#include <restinio/core.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

#include <iterator>

[[nodiscard]]
restinio::fmt_minimal_memory_buffer_t
format_chunked_input_info(
	const restinio::request_t & req )
{
	restinio::fmt_minimal_memory_buffer_t resp_body;
	auto resp_body_inserter = std::back_inserter( resp_body );

	const auto * chunked_input = req.chunked_input_info();
	if( !chunked_input )
		fmt::format_to( resp_body_inserter,
				RESTINIO_FMT_FORMAT_STRING( "no chunked input" ) );
	else
	{
		fmt::format_to( resp_body_inserter,
				RESTINIO_FMT_FORMAT_STRING( "chunks:{};" ),
				chunked_input->chunk_count() );
		for( const auto & ch : chunked_input->chunks() )
		{
			if( const auto * ext_params = ch.ext_params(); ext_params )
			{
				std::vector<std::string> exts;
				std::transform(
					begin( *ext_params ),
					end( *ext_params ),
					std::back_inserter( exts ),
					[]( const auto & p ){
						return fmt::format(
							RESTINIO_FMT_FORMAT_STRING( "('{}','{}')" ),
							p.m_name,
							p.m_value );
					} );
				fmt::format_to( resp_body_inserter,
						RESTINIO_FMT_FORMAT_STRING( "[{},{};ext:{}]" ),
						ch.started_at(),
						ch.size(),
						fmt::join(begin(exts), end(exts), "," ) );
			}
			else
			{
				fmt::format_to( resp_body_inserter,
					RESTINIO_FMT_FORMAT_STRING( "[{},{};ext:nullptr]" ),
					ch.started_at(), ch.size() );
			}
		}
		fmt::format_to( resp_body_inserter,
				RESTINIO_FMT_FORMAT_STRING( ";trailing_fields:{};" ),
				chunked_input->trailing_fields().fields_count() );
		for( const auto & f : chunked_input->trailing_fields() )
			fmt::format_to(
					resp_body_inserter,
					RESTINIO_FMT_FORMAT_STRING( "('{}':'{}')" ),
					f.name(), f.value() );
	}

	return resp_body;
}

TEST_CASE( "Simple incoming request" , "[chunked-input][simple]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler(
					[]( auto req ){
						if( restinio::http_method_post() == req->header().method() )
						{
							restinio::fmt_minimal_memory_buffer_t resp_body =
								format_chunked_input_info( *req );

							req->create_response()
								.append_header( "Server", "RESTinio utest server" )
								.append_header_date_field()
								.append_header( "Content-Type", "text/plain; charset=utf-8" )
								.set_body( std::move(resp_body) )
								.done();
							return restinio::request_accepted();
						}

						return restinio::request_rejected();
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	SECTION( "no chunked-encoding" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 20\r\n"
			"Connection: close\r\n"
			"\r\n"
			"01234567890123456789"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"no chunked input") );
	}

	SECTION( "three chunks" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"6\r\n"
			"Hello,\r\n"
			"1\r\n"
			" \r\n"
			"6\r\n"
			"World!\r\n"
			"0\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"chunks:3;"
						"[0,6;ext:nullptr]"
						"[6,1;ext:nullptr]"
						"[7,6;ext:nullptr];"
						"trailing_fields:0;") );
	}

	SECTION( "three chunks with trailing headers" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"6\r\n"
			"Hello,\r\n"
			"1\r\n"
			" \r\n"
			"6\r\n"
			"World!\r\n"
			"0\r\n"
			"Header-1: Value-1\r\n"
			"Header-2: Value-2\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"chunks:3;"
						"[0,6;ext:nullptr]"
						"[6,1;ext:nullptr]"
						"[7,6;ext:nullptr];"
						"trailing_fields:2;"
						"('Header-1':'Value-1')('Header-2':'Value-2')" ) );
	}

	SECTION( "three chunks with trailing field without value" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"6\r\n"
			"Hello,\r\n"
			"1\r\n"
			" \r\n"
			"6\r\n"
			"World!\r\n"
			"0\r\n"
			"Header-1:\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"chunks:3;"
						"[0,6;ext:nullptr]"
						"[6,1;ext:nullptr]"
						"[7,6;ext:nullptr];"
						"trailing_fields:1;"
						"('Header-1':'')" ) );
	}

	SECTION( "two chunks with ext" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"6;novalue_field;f1=v1\r\n"
			"Hello,\r\n"
			"1;f=\" \\\\1 \\\"2\\\" 3 ! \";a;b\r\n"
			" \r\n"
			"0\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"chunks:2;"
						"[0,6;ext:('novalue_field',''),('f1','v1')]"
						"[6,1;ext:('f','\" \\\\1 \\\"2\\\" 3 ! \"'),('a',''),('b','')];"
						"trailing_fields:0;" ) );
	}

	SECTION( "two chunks with chunk-ext" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"6;a=b;c;d=e;xxx=yyy\r\n"
			"Hello,\r\n"
			"6;b=d;c=e\r\n"
			"World!\r\n"
			"0\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"chunks:2;"
						"[0,6;ext:('a','b'),('c',''),('d','e'),('xxx','yyy')]"
						"[6,6;ext:('b','d'),('c','e')];"
						"trailing_fields:0;") );
	}

	other_thread.stop_and_join();
}

TEST_CASE( "Limit violations" , "[chunked-input][simple][limits]" )
{
	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.incoming_http_msg_limits(
						restinio::incoming_http_msg_limits_t{}
							.max_field_count( 16 ) 
							.max_body_size( 100 )
					)
				.request_handler(
					[]( auto req ){
						if( restinio::http_method_post() == req->header().method() )
						{
							restinio::fmt_minimal_memory_buffer_t resp_body =
								format_chunked_input_info( *req );

							req->create_response()
								.append_header( "Server", "RESTinio utest server" )
								.append_header_date_field()
								.append_header( "Content-Type", "text/plain; charset=utf-8" )
								.set_body( std::move(resp_body) )
								.done();
							return restinio::request_accepted();
						}

						return restinio::request_rejected();
					} );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	SECTION( "allowed count of leading fields" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"My-Header-01: bla-bla-bla\r\n"
			"My-Header-02: bla-bla-bla\r\n"
			"My-Header-03: bla-bla-bla\r\n"
			"My-Header-04: bla-bla-bla\r\n"
			"My-Header-05: bla-bla-bla\r\n"
			"My-Header-06: bla-bla-bla\r\n"
			"My-Header-07: bla-bla-bla\r\n"
			"My-Header-08: bla-bla-bla\r\n"
			"My-Header-09: bla-bla-bla\r\n"
			"My-Header-10: bla-bla-bla\r\n"
			"My-Header-11: bla-bla-bla\r\n"
			"\r\n"
			"6\r\n"
			"Hello,\r\n"
			"1\r\n"
			" \r\n"
			"6\r\n"
			"World!\r\n"
			"0\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"chunks:3;"
						"[0,6;ext:nullptr]"
						"[6,1;ext:nullptr]"
						"[7,6;ext:nullptr];"
						"trailing_fields:0;") );
	}
	SECTION( "too many leading fields" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"My-Header-01: bla-bla-bla\r\n"
			"My-Header-02: bla-bla-bla\r\n"
			"My-Header-03: bla-bla-bla\r\n"
			"My-Header-04: bla-bla-bla\r\n"
			"My-Header-05: bla-bla-bla\r\n"
			"My-Header-06: bla-bla-bla\r\n"
			"My-Header-07: bla-bla-bla\r\n"
			"My-Header-08: bla-bla-bla\r\n"
			"My-Header-09: bla-bla-bla\r\n"
			"My-Header-10: bla-bla-bla\r\n"
			"My-Header-11: bla-bla-bla\r\n"
			"My-Header-12: bla-bla-bla\r\n"
			"\r\n"
			"6\r\n"
			"Hello,\r\n"
			"1\r\n"
			" \r\n"
			"6\r\n"
			"World!\r\n"
			"0\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_THROWS( response = do_request( request ) );
	}

	SECTION( "three chunks with allowed number of trailing headers" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"6\r\n"
			"Hello,\r\n"
			"1\r\n"
			" \r\n"
			"6\r\n"
			"World!\r\n"
			"0\r\n"
			"Header-01: Value\r\n"
			"Header-02: Value\r\n"
			"Header-03: Value\r\n"
			"Header-04: Value\r\n"
			"Header-05: Value\r\n"
			"Header-06: Value\r\n"
			"Header-07: Value\r\n"
			"Header-08: Value\r\n"
			"Header-09: Value\r\n"
			"Header-10: Value\r\n"
			"Header-11: Value\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT( response,
				Catch::Matchers::EndsWith(
						"chunks:3;"
						"[0,6;ext:nullptr]"
						"[6,1;ext:nullptr]"
						"[7,6;ext:nullptr];"
						"trailing_fields:11;"
						"('Header-01':'Value')('Header-02':'Value')"
						"('Header-03':'Value')('Header-04':'Value')"
						"('Header-05':'Value')('Header-06':'Value')"
						"('Header-07':'Value')('Header-08':'Value')"
						"('Header-09':'Value')('Header-10':'Value')"
						"('Header-11':'Value')"
			) );
	}

	SECTION( "three chunks with disallowed number of trailing headers" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"6\r\n"
			"Hello,\r\n"
			"1\r\n"
			" \r\n"
			"6\r\n"
			"World!\r\n"
			"0\r\n"
			"Header-01: Value\r\n"
			"Header-02: Value\r\n"
			"Header-03: Value\r\n"
			"Header-04: Value\r\n"
			"Header-05: Value\r\n"
			"Header-06: Value\r\n"
			"Header-07: Value\r\n"
			"Header-08: Value\r\n"
			"Header-09: Value\r\n"
			"Header-10: Value\r\n"
			"Header-11: Value\r\n"
			"Header-12: Value\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_THROWS( response = do_request( request ) );
	}

	SECTION( "three chunks with trailing field without value" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"6\r\n"
			"Hello,\r\n"
			"1\r\n"
			" \r\n"
			"6\r\n"
			"World!\r\n"
			"0\r\n"
			"Header-01: Value\r\n"
			"Header-02: Value\r\n"
			"Header-03: Value\r\n"
			"Header-04: Value\r\n"
			"Header-05: Value\r\n"
			"Header-06: Value\r\n"
			"Header-07: Value\r\n"
			"Header-08: Value\r\n"
			"Header-09: Value\r\n"
			"Header-10: Value\r\n"
			"Header-11: Value\r\n"
			"Header-1:\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_THROWS( response = do_request( request ) );
	}

	SECTION( "total body length to long" )
	{
		std::string request{
			"POST /data HTTP/1.0\r\n"
			"From: unit-test\r\n"
			"User-Agent: unit-test\r\n"
			"Content-Type: text/plain\r\n"
			"Transfer-Encoding: chunked\r\n"
			"Connection: close\r\n"
			"\r\n"
			"a\r\n"
			"0123456789\r\n"
			"a\r\n"
			"0123456789\r\n"
			"a\r\n"
			"0123456789\r\n"
			"a\r\n"
			"0123456789\r\n"
			"a\r\n"
			"0123456789\r\n"
			"a\r\n"
			"0123456789\r\n"
			"a\r\n"
			"0123456789\r\n"
			"a\r\n"
			"0123456789\r\n"
			"a\r\n"
			"0123456789\r\n"
			"a\r\n"
			"0123456789\r\n"
			"1\r\n"
			"0\r\n"
			"0\r\n"
			"\r\n"
		};

		std::string response;
		REQUIRE_THROWS( response = do_request( request ) );
	}

	other_thread.stop_and_join();
}
