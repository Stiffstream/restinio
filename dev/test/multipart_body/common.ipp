/*
	restinio
*/

#include <catch2/catch_all.hpp>

#include <restinio/helpers/multipart_body.hpp>

using namespace std::string_literals;

class dummy_connection_t : public restinio::impl::connection_base_t
{
public:
	using restinio::impl::connection_base_t::connection_base_t;

	void
	write_response_parts(
		restinio::request_id_t /*request_id*/,
		restinio::response_output_flags_t /*response_output_flags*/,
		restinio::write_group_t /*wg*/ ) override
	{ /* Nothing to do! */ }

	void
	check_timeout(
		std::shared_ptr< restinio::tcp_connection_ctx_base_t > & /*self*/ ) override
	{ /* Nothing to do! */ }

	[[nodiscard]]
	static auto
	make( restinio::connection_id_t id )
	{
		return std::make_shared< dummy_connection_t >( id );
	}

};

[[nodiscard]]
inline auto
make_dummy_endpoint()
{
	return restinio::endpoint_t{
			restinio::asio_ns::ip::make_address("127.0.0.1"),
			12345u
	};
}

