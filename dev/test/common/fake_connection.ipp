struct fake_connection_t : public restinio::impl::connection_base_t
{
	fake_connection_t() : restinio::impl::connection_base_t{ 0 }
	{}

	virtual void
	check_timeout(
		std::shared_ptr< restinio::tcp_connection_ctx_base_t > & ) override
	{}

	virtual void
	write_response_parts(
		restinio::request_id_t ,
		restinio::response_output_flags_t ,
		restinio::write_group_t ) override
	{}
};

