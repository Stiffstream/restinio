/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/all.hpp>

#include <test/common/utest_logger.hpp>
#include <test/common/pub.hpp>

namespace test
{

class test_extra_data_factory_t
{
	struct ctor_dtor_monitors_t
	{
		std::atomic<int> m_constructors{0};
		std::atomic<int> m_destructors{0};
	};

	std::atomic<int> m_index_counter{0};

	ctor_dtor_monitors_t m_ctor_dtor_monitors;

public:
	class data_t
	{
		ctor_dtor_monitors_t & m_monitors;

	public:
		data_t(
			ctor_dtor_monitors_t & monitors,
			int index )
			:	m_monitors{ monitors }
			,	m_index{ index }
		{
			++m_monitors.m_constructors;
		}
		~data_t()
		{
			++m_monitors.m_destructors;
		}

		int m_index;
	};

	void
	make_within( restinio::extra_data_buffer_t<data_t> buffer ) noexcept
	{
		new(buffer.get()) data_t{ m_ctor_dtor_monitors, ++m_index_counter };
	}

	RESTINIO_NODISCARD
	int
	current_value() noexcept
	{
		return m_index_counter.load( std::memory_order_acquire );
	}

	RESTINIO_NODISCARD
	int
	constructors_called() noexcept
	{
		return m_ctor_dtor_monitors.m_constructors.load( std::memory_order_acquire );
	}

	RESTINIO_NODISCARD
	int
	destructors_called() noexcept
	{
		return m_ctor_dtor_monitors.m_destructors.load( std::memory_order_acquire );
	}
};

struct test_traits_t : public restinio::traits_t<
	restinio::asio_timer_manager_t, utest_logger_t >
{
	using extra_data_factory_t = test_extra_data_factory_t;
};

} /* namespace test */

TEST_CASE( "remote_endpoint extraction" , "[remote_endpoint]" )
{
	using namespace test;

	using http_server_t = restinio::http_server_t< test_traits_t >;

	std::string endpoint_value;
	int index_value;

	auto extra_data_factory = std::make_shared< test_extra_data_factory_t >();

	http_server_t http_server{
		restinio::own_io_context(),
		[&endpoint_value, &index_value, extra_data_factory]( auto & settings ){
			settings
				.port( utest_default_port() )
				.address( "127.0.0.1" )
				.extra_data_factory( extra_data_factory )
				.request_handler(
					[&endpoint_value, &index_value]( auto req ){
						endpoint_value = fmt::format(
								RESTINIO_FMT_FORMAT_STRING( "{}" ),
								restinio::fmtlib_tools::streamed(
										req->remote_endpoint() ) );
						index_value = req->extra_data().m_index;

						req->create_response()
							.append_header( "Server", "RESTinio utest server" )
							.append_header_date_field()
							.append_header( "Content-Type", "text/plain; charset=utf-8" )
							.set_body(
								restinio::const_buffer( req->header().method().c_str() ) )
							.done();

						return restinio::request_accepted();
					} );
		} };

	other_work_thread_for_server_t<http_server_t> other_thread(http_server);
	other_thread.run();

	std::string response;
	const char * request_str =
		"GET / HTTP/1.1\r\n"
		"Host: 127.0.0.1\r\n"
		"User-Agent: unit-test\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n"
		"\r\n";

	REQUIRE_NOTHROW( response = do_request( request_str ) );

	REQUIRE_THAT( response, Catch::Matchers::EndsWith( "GET" ) );

	other_thread.stop_and_join();

	REQUIRE( !endpoint_value.empty() );
	REQUIRE( 0 != extra_data_factory->current_value() );
	REQUIRE( 1 == extra_data_factory->constructors_called() );
	REQUIRE( 1 == extra_data_factory->destructors_called() );
	REQUIRE( 1 == index_value );
}

