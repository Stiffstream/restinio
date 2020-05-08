#pragma once

#include <restinio/all.hpp>

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

	RESTINIO_NODISCARD
	static auto
	make( restinio::connection_id_t id )
	{
		return std::make_shared< dummy_connection_t >( id );
	}

};

