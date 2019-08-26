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

TEST_CASE( "body_handler" , "[zlib][body_handler]" )
{
	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	const auto response_body = create_random_text( 128 * 1024, 16 );

	using router_t = restinio::router::express_router_t<>;

	auto router = std::make_unique< router_t >();

	namespace rtz = restinio::transforms::zlib;

	router->http_post(
		"/",
		[ & ]( auto req, auto ){
			return
				restinio::transforms::zlib::handle_body(
					*req,
					[&]( auto body ){
						return
							req->create_response()
								.append_header( restinio::http_field::server, "RESTinio" )
								.append_header_date_field()
								.set_body( std::move( body ) )
								.done();
					} );
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

	for( int i = 0; i <= 9; ++i )
	{
		{
			auto compressed_data = rtz::deflate_compress( response_body, i );

			const std::string request =
				fmt::format(
					"POST / HTTP/1.0\r\n"
					"From: unit-test\r\n"
					"User-Agent: unit-test\r\n"
					"Content-Type: text/plain\r\n"
					"Content-Encoding: DEFLATE\r\n"
					"Content-Length: {}\r\n"
					"Connection: close\r\n"
					"\r\n"
					"{}",
					compressed_data.size(),
					compressed_data );

			std::string response;

			REQUIRE_NOTHROW( response = do_request( request ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::EndsWith(
					"\r\n\r\n" +
					response_body ) );
		}

		{
			auto compressed_data = rtz::gzip_compress( response_body, i );

			const std::string request =
				fmt::format(
					"POST / HTTP/1.0\r\n"
					"From: unit-test\r\n"
					"User-Agent: unit-test\r\n"
					"Content-Type: text/plain\r\n"
					"Content-Encoding: GZIP\r\n"
					"Content-Length: {}\r\n"
					"Connection: close\r\n"
					"\r\n"
					"{}",
					compressed_data.size(),
					compressed_data );

			std::string response;

			REQUIRE_NOTHROW( response = do_request( request ) );

			REQUIRE_THAT(
				response,
				Catch::Matchers::EndsWith(
					"\r\n\r\n" +
					response_body ) );
		}
	}

	other_thread.stop_and_join();
}

TEST_CASE( "body_handler void return" , "[zlib][body_handler][void-return]" )
{
	std::srand( static_cast<unsigned int>(std::time( nullptr )) );

	const auto response_body = create_random_text( 1024, 16 );

	using router_t = restinio::router::express_router_t<>;

	auto router = std::make_unique< router_t >();

	namespace rtz = restinio::transforms::zlib;

	router->http_post(
		"/",
		[ & ]( const restinio::request_handle_t& req, auto ){
			auto resp = req->create_response();

			resp.append_header( restinio::http_field::server, "RESTinio" )
				.append_header_date_field();

			restinio::transforms::zlib::handle_body(
				*req,
				[&]( auto body ){
					resp.set_body( std::move( body ) );
				} );

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
		const std::string request =
			fmt::format(
				"POST / HTTP/1.0\r\n"
				"From: unit-test\r\n"
				"User-Agent: unit-test\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Encoding: IDENTITY\r\n"
				"Content-Length: {}\r\n"
				"Connection: close\r\n"
				"\r\n"
				"{}",
				response_body.size(),
				response_body );

		std::string response;

		REQUIRE_NOTHROW( response = do_request( request ) );

		REQUIRE_THAT(
			response,
			Catch::Matchers::EndsWith(
				"\r\n\r\n" +
				response_body ) );
	}

	other_thread.stop_and_join();
}
