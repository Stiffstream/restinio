struct fake_connection_t : public restinio::impl::connection_base_t
{
	fake_connection_t() : restinio::impl::connection_base_t{ 0 }
	{}

	virtual void
	check_timeout( std::shared_ptr< tcp_connection_ctx_base_t > & ) override
	{}

	virtual void
	write_response_parts(
		request_id_t ,
		response_output_flags_t ,
		write_group_t ) override
	{}
};


request_handle_t
create_fake_request( std::string target, http_method_id_t method = http_method_get() )
{
	restinio::no_user_data_factory_t user_data_factory;
	return std::make_shared< request_t >(
			0,
			http_request_header_t{ method, std::move( target ) },
			"",
			std::make_shared< fake_connection_t >(),
			restinio::endpoint_t{
				restinio::asio_ns::ip::make_address_v4("127.0.0.1"),
				3000 },
			user_data_factory );
}

