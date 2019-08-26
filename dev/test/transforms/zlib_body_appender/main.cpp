/*
	restinio
*/

/*!
	Echo server.
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>
#include <restinio/transforms/zlib.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

#include "../random_data_generators.ipp"

TEST_CASE( "restinio_controlled_output" , "[zlib][body_appender][restinio_controlled_output]" )
{
	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	const auto response_body = create_random_text( 128 * 1024, 16 );

	using router_t = restinio::router::express_router_t<>;

	auto router = std::make_unique< router_t >();

	namespace rtz = restinio::transforms::zlib;

	router->http_get(
		R"-(/deflate/:level(-?\d+))-",
		[ & ]( auto req, auto params ){
				auto resp = req->create_response();

				resp
					.append_header( "Server", "RESTinio Benchmark" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" );

				const auto level = restinio::cast_to< int >( params["level" ] );

				rtz::deflate_body_appender( resp, level )
					.append( response_body )
					.complete();

				return resp.done();
		} );

	router->http_get(
		R"-(/gzip/:level(-?\d+))-",
		[ & ]( auto req, auto params ){
				auto resp = req->create_response();

				resp
					.append_header( "Server", "RESTinio Benchmark" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" );

				const auto level = restinio::cast_to< int >( params["level" ] );

				rtz::gzip_body_appender( resp, level )
					.append( response_body )
					.complete();

				return resp.done();
		} );

	router->http_get(
		R"-(/identity)-",
		[ & ]( auto req, auto ){
				auto resp = req->create_response();

				resp
					.append_header( "Server", "RESTinio Benchmark" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" );

				rtz::identity_body_appender( resp )
					.append( response_body )
					.complete();

				return resp.done();
		} );


	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t,
				router_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[&]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler( std::move( router ) );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread{ http_server };
	other_thread.run();

	{
		const std::string request{
				"GET /deflate/-1 HTTP/1.0\r\n"
				"From: unit-test\r\n"
				"User-Agent: unit-test\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n"
				"Connection: close\r\n"
				"\r\n"
		};
		std::string response;

		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "Content-Encoding: deflate\r\n" ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::EndsWith(
				"\r\n\r\n" +
				rtz::deflate_compress( response_body, -1 ) ) );
	}

	{
		const std::string request{
				"GET /gzip/-1 HTTP/1.0\r\n"
				"From: unit-test\r\n"
				"User-Agent: unit-test\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n"
				"Connection: close\r\n"
				"\r\n"
		};
		std::string response;

		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "Content-Encoding: gzip\r\n" ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::EndsWith(
				"\r\n\r\n" +
				rtz::gzip_compress( response_body, -1 ) ) );
	}

	{
		const std::string request{
				"GET /identity HTTP/1.0\r\n"
				"From: unit-test\r\n"
				"User-Agent: unit-test\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n"
				"Connection: close\r\n"
				"\r\n"
		};
		std::string response;

		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "Content-Encoding: identity\r\n" ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::EndsWith( "\r\n\r\n" + response_body ) );
	}

	for( int i = 0; i <= 9; ++i )
	{
		{
			const std::string request =
				fmt::format(
					"GET /deflate/{} HTTP/1.0\r\n"
					"From: unit-test\r\n"
					"User-Agent: unit-test\r\n"
					"Content-Type: application/x-www-form-urlencoded\r\n"
					"Connection: close\r\n"
					"\r\n",
					i );

			std::string response;

			REQUIRE_NOTHROW( response = do_request( request ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::Contains( "Content-Encoding: deflate\r\n" ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::EndsWith(
					"\r\n\r\n" +
					rtz::deflate_compress( response_body, i ) ) );
		}

		{
			const std::string request =
				fmt::format(
					"GET /gzip/{} HTTP/1.0\r\n"
					"From: unit-test\r\n"
					"User-Agent: unit-test\r\n"
					"Content-Type: application/x-www-form-urlencoded\r\n"
					"Connection: close\r\n"
					"\r\n",
					i );

			std::string response;

			REQUIRE_NOTHROW( response = do_request( request ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::Contains( "Content-Encoding: gzip\r\n" ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::EndsWith(
					"\r\n\r\n" +
					rtz::gzip_compress( response_body, i ) ) );
		}
	}

	other_thread.stop_and_join();
}

TEST_CASE( "user_controlled_output" , "[zlib][body_appender][user_controlled_output]" )
{
	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	const auto response_body = create_random_text( 128 * 1024, 16 );

	using router_t = restinio::router::express_router_t<>;

	auto router = std::make_unique< router_t >();

	namespace rtz = restinio::transforms::zlib;

	router->http_get(
		R"-(/deflate/:level(-?\d+))-",
		[ & ]( const restinio::request_handle_t& req, auto params ){
				auto resp = req->create_response< restinio::user_controlled_output_t >();

				resp
					.append_header( "Server", "RESTinio Benchmark" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" );

				const auto level = restinio::cast_to< int >( params["level" ] );

				auto ba = rtz::deflate_body_appender( resp, level );

				ba
					.append(
						restinio::string_view_t{
							response_body.data(),
							response_body.size()/2 } )
					.flush();

				ba
					.append(
						restinio::string_view_t{
							response_body.data() + response_body.size()/2,
							response_body.size() - response_body.size()/2 } )
					.complete();

				return resp.done();
		} );

	router->http_get(
		R"-(/gzip/:level(-?\d+))-",
		[ & ]( const restinio::request_handle_t& req, auto params ){
				auto resp = req->create_response< restinio::user_controlled_output_t >();

				resp
					.append_header( "Server", "RESTinio Benchmark" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" );

				const auto level = restinio::cast_to< int >( params["level" ] );

				auto ba = rtz::gzip_body_appender( resp, level );
				ba
					.append(
						restinio::string_view_t{
							response_body.data(),
							response_body.size()/2 } )
					.flush();

				ba
					.append(
						restinio::string_view_t{
							response_body.data() + response_body.size()/2,
							response_body.size() - response_body.size()/2 } )
					.complete();

				return resp.done();
		} );

	router->http_get(
		R"-(/identity)-",
		[ & ]( const restinio::request_handle_t& req, auto ){
				auto resp = req->create_response< restinio::user_controlled_output_t >();

				resp
					.append_header( "Server", "RESTinio Benchmark" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" );

				auto ba = rtz::identity_body_appender( resp );
				ba
					.append(
						restinio::string_view_t{
							response_body.data(),
							response_body.size()/2 } )
					.flush();

				ba
					.append(
						restinio::string_view_t{
							response_body.data() + response_body.size()/2,
							response_body.size() - response_body.size()/2 } )
					.complete();

				return resp.done();
		} );

	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t,
				router_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[&]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler( std::move( router ) );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread{ http_server };
	other_thread.run();

	{
		const std::string request{
				"GET /deflate/-1 HTTP/1.0\r\n"
				"From: unit-test\r\n"
				"User-Agent: unit-test\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n"
				"Connection: close\r\n"
				"\r\n"
		};
		std::string response;

		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "Content-Encoding: deflate\r\n" ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "\r\n\r\n" ) );

		const auto body_start = response.find( "\r\n\r\n" ) + 4;

		REQUIRE(
			response_body ==
			rtz::deflate_decompress(
				restinio::string_view_t{
					response.data() + body_start,
					response.size() - body_start } ) );
	}


	{
		const std::string request{
				"GET /gzip/-1 HTTP/1.0\r\n"
				"From: unit-test\r\n"
				"User-Agent: unit-test\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n"
				"Connection: close\r\n"
				"\r\n"
		};
		std::string response;

		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "Content-Encoding: gzip\r\n" ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "\r\n\r\n" ) );

		const auto body_start = response.find( "\r\n\r\n" ) + 4;

		REQUIRE(
			response_body ==
			rtz::gzip_decompress(
				restinio::string_view_t{
					response.data() + body_start,
					response.size() - body_start } ) );
	}


	{
		const std::string request{
				"GET /identity HTTP/1.0\r\n"
				"From: unit-test\r\n"
				"User-Agent: unit-test\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n"
				"Connection: close\r\n"
				"\r\n"
		};
		std::string response;

		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "Content-Encoding: identity\r\n" ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "\r\n\r\n" ) );

		const auto body_start = response.find( "\r\n\r\n" ) + 4;

		REQUIRE(
			response_body ==
			restinio::string_view_t{
				response.data() + body_start,
				response.size() - body_start } );
	}

	for( int i = 0; i <= 9; ++i )
	{
		{
			const std::string request =
				fmt::format(
					"GET /deflate/{} HTTP/1.0\r\n"
					"From: unit-test\r\n"
					"User-Agent: unit-test\r\n"
					"Content-Type: application/x-www-form-urlencoded\r\n"
					"Connection: close\r\n"
					"\r\n",
					i );

			std::string response;

			REQUIRE_NOTHROW( response = do_request( request ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::Contains( "Content-Encoding: deflate\r\n" ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::Contains( "\r\n\r\n" ) );

			const auto body_start = response.find( "\r\n\r\n" ) + 4;

			REQUIRE(
				response_body ==
				rtz::deflate_decompress(
					restinio::string_view_t{
						response.data() + body_start,
						response.size() - body_start } ) );
		}

		{
			const std::string request =
				fmt::format(
					"GET /gzip/{} HTTP/1.0\r\n"
					"From: unit-test\r\n"
					"User-Agent: unit-test\r\n"
					"Content-Type: application/x-www-form-urlencoded\r\n"
					"Connection: close\r\n"
					"\r\n",
					i );

			std::string response;

			REQUIRE_NOTHROW( response = do_request( request ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::Contains( "Content-Encoding: gzip\r\n" ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::Contains( "\r\n\r\n" ) );

			const auto body_start = response.find( "\r\n\r\n" ) + 4;

			REQUIRE(
				response_body ==
				rtz::gzip_decompress(
					restinio::string_view_t{
						response.data() + body_start,
						response.size() - body_start } ) );
		}
	}

	other_thread.stop_and_join();
}

TEST_CASE( "chunked_output" , "[zlib][body_appender][chunked_output]" )
{
	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	const auto response_body = create_random_text( 128 * 1024, 16 );

	using router_t = restinio::router::express_router_t<>;

	auto router = std::make_unique< router_t >();

	namespace rtz = restinio::transforms::zlib;

	router->http_get(
		R"-(/deflate/:level(-?\d+))-",
		[ & ]( const restinio::request_handle_t& req, auto params ){
				auto resp = req->create_response< restinio::chunked_output_t >();

				resp
					.append_header( "Server", "RESTinio Benchmark" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" );

				const auto level = restinio::cast_to< int >( params["level" ] );

				auto ba = rtz::deflate_body_appender( resp, level );

				ba
					.append(
						restinio::string_view_t{
							response_body.data(),
							response_body.size()/2 } )
					.make_chunk()
					.flush();

				ba
					.append(
						restinio::string_view_t{
							response_body.data() + response_body.size()/2,
							response_body.size() - response_body.size()/2 } )
					.make_chunk()
					.complete();

				return resp.done();
		} );

	router->http_get(
		R"-(/gzip/:level(-?\d+))-",
		[ & ]( const restinio::request_handle_t& req, auto params ){
				auto resp = req->create_response< restinio::chunked_output_t >();

				resp
					.append_header( "Server", "RESTinio Benchmark" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" );

				const auto level = restinio::cast_to< int >( params["level" ] );

				auto ba = rtz::gzip_body_appender( resp, level );

				ba
					.append(
						restinio::string_view_t{
							response_body.data(),
							response_body.size()/2 } )
					.make_chunk()
					.flush();

				ba
					.append(
						restinio::string_view_t{
							response_body.data() + response_body.size()/2,
							response_body.size() - response_body.size()/2 } )
					.make_chunk()
					.complete();

				return resp.done();
		} );

	router->http_get(
		R"-(/identity)-",
		[ & ]( const restinio::request_handle_t& req, auto ){
				auto resp = req->create_response< restinio::chunked_output_t >();

				resp
					.append_header( "Server", "RESTinio Benchmark" )
					.append_header_date_field()
					.append_header( "Content-Type", "text/plain; charset=utf-8" );

				auto ba = rtz::identity_body_appender( resp );

				ba
					.append(
						restinio::string_view_t{
							response_body.data(),
							response_body.size()/2 } )
					.make_chunk()
					.flush();

				ba
					.append(
						restinio::string_view_t{
							response_body.data() + response_body.size()/2,
							response_body.size() - response_body.size()/2 } )
					.make_chunk()
					.complete();

				return resp.done();
		} );


	using http_server_t =
		restinio::http_server_t<
			restinio::traits_t<
				restinio::asio_timer_manager_t,
				utest_logger_t,
				router_t > >;

	http_server_t http_server{
		restinio::own_io_context(),
		[&]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.request_handler( std::move( router ) );
		}
	};

	other_work_thread_for_server_t<http_server_t> other_thread{ http_server };
	other_thread.run();

	{
		const std::string request{
				"GET /deflate/-1 HTTP/1.0\r\n"
				"From: unit-test\r\n"
				"User-Agent: unit-test\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n"
				"Connection: close\r\n"
				"\r\n"
		};
		std::string response;

		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "Content-Encoding: deflate\r\n" ) );
	}


	{
		const std::string request{
				"GET /gzip/-1 HTTP/1.0\r\n"
				"From: unit-test\r\n"
				"User-Agent: unit-test\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n"
				"Connection: close\r\n"
				"\r\n"
		};
		std::string response;

		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "Content-Encoding: gzip\r\n" ) );
	}


	{
		const std::string request{
				"GET /identity HTTP/1.0\r\n"
				"From: unit-test\r\n"
				"User-Agent: unit-test\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n"
				"Connection: close\r\n"
				"\r\n"
		};
		std::string response;

		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::Contains( "Content-Encoding: identity\r\n" ) );
	}

	other_thread.stop_and_join();
}
