/*
	restinio
*/

/*!
	Tests for express router.
*/

#include <catch2/catch.hpp>

#include <iterator>

#include <restinio/all.hpp>

using namespace restinio;

namespace test
{

struct ud_factory_t
{
	struct data_t
	{
		int m_a;
		int m_b;
		int m_c;
		void * m_d;
	};

	void make_within( restinio::user_data_buffer_t<data_t> buf ) noexcept
	{
		new(buf.get()) data_t{ 0, 1, 2, buf.get() };
	}
};

} /* namespace test */

using express_router_t = restinio::router::express_router_t<
		restinio::router::std_regex_engine_t,
		test::ud_factory_t >;

using restinio::router::route_params_t;

#include "../express_router/tests.ipp"

